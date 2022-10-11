// License Summary: MIT see LICENSE file
#include "core/core.h"
//#include "dbg/print.h"
//#include "dbg/assert.h"
//#include "memory/memory.h"
#include "core/math.h"
#include "multi_core/freelist.h"
#if 0
#define END_OF_LIST_SENTINEL (~0UL)
static bool AllocNewBlock(MultiCore_FreeList *fl) {
	// first thing we need to do is claim our new index range
	uint64_t baseIndex = Atomic_Add_U64(&fl->totalElementsAllocated, (fl->elementsPerBlockMask + 1));

	if (baseIndex >= (fl->elementsPerBlockMask + 1) * fl->maxBlocks) {
		debug_printf("Trying to allocate more than %lu blocks! Increase block size or max blocks", fl->maxBlocks);
		Atomic_Sub_U64(&fl->totalElementsAllocated, (fl->elementsPerBlockMask + 1));
		return false;
	}

	assert((baseIndex >> fl->elementsPerBlockShift) < fl->maxBlocks);

	size_t const blockSize = (fl->elementsPerBlockMask + 1) * fl->elementSize;

	uint8_t *base = (uint8_t *) MEMORY_CALLOC(1, blockSize);
	if (!base) {
		debug_print("Out of memory!");
		return false;
	}

	Atomic_Store_Ptr(fl->blocks + (baseIndex >> fl->elementsPerBlockShift), base);

	// init free list for new block
	uint8_t *workPtr = base;
	for (uint32_t i = 0u; i < fl->elementsPerBlockMask; ++i) {
		*((uintptr_t *) workPtr) = (uintptr_t) (workPtr + fl->elementSize);
		workPtr += fl->elementSize;
	}

	// link the new block into the free list and attach existing free list to the
	// end of this block
	Redo:;
	uint64_t head = Atomic_Load_U64(&fl->freeListHead);
	*((uintptr_t *) workPtr) = head;

	if (Atomic_CompareExchange_U64(&fl->freeListHead, &head, (uint64_t) base) != head) {
		goto Redo; // something changed reverse the transaction
	}
	return true;
}

MultiCore_FreeListHandle MultiCore_FreeListCreate(size_t elementSize, uint32_t blockCount, uint32_t maxBlocks) {
	assert(elementSize >= sizeof(uintptr_t));

	if (!Math_IsPowerOfTwo_U32(blockCount)) {
		debug_printf("blockCount (%u) should be a power of 2, using %u", blockCount, Math_NextPowerOfTwo_U32(blockCount));
		blockCount = Math_NextPowerOfTwo_U32(blockCount);
	}

	// each block has space for the data
	size_t const blockSize = (blockCount * elementSize);

	// first block is attached directly to the header
	size_t const allocSize = sizeof(MultiCore_FreeList)
                         + blockSize +
                         8 + // padding to ensure atomics are at least 8 byte aligned
                         (maxBlocks * sizeof(void*));

	MultiCore_FreeList *fl = (MultiCore_FreeList *) MEMORY_CALLOC(1, allocSize);
	if (!fl) {
		return nullptr;
	}
	fl->elementSize = elementSize;
	fl->elementsPerBlockMask = blockCount - 1;
	fl->elementsPerBlockShift = Math_LogTwo_U32(blockCount);
	fl->maxBlocks = maxBlocks;

	uint8_t *base = (uint8_t *) (fl + 1);

	// get to blocks space with 8 byte alignment guarenteed
	fl->blocks = (void* *) (((uintptr_t) base + blockSize + 0x8ull) & ~0x7ull);
	Atomic_Store_Ptr(fl->blocks + 0, base);
	Atomic_Store_U64(&fl->totalElementsAllocated, blockCount);

	// init free list for new block
	uint8_t *workPtr = base;
	for (uint32_t i = 0u; i < (blockCount - 1); ++i) {
		*((uintptr_t *) workPtr) = (uintptr_t) (workPtr + fl->elementSize);
		workPtr += fl->elementSize;
	}

	// fix last index to point to the invalid marker
	*((uintptr_t *) workPtr) = END_OF_LIST_SENTINEL;

	// point head to start of the free list
	Atomic_Store_U64(&fl->freeListHead, (uintptr_t) base);

	return fl;
}

void MultiCore_FreeListDestroy(MultiCore_FreeListHandle fl) {
	if (!fl) {
		return;
	}

	// 0th block is embedded
	for (uint32_t i = 1u; i < fl->maxBlocks; ++i) {
		if (fl->blocks[i]) {
			MEMORY_FREE(fl->blocks[i]);
		}
	}

	MEMORY_FREE(fl);
}

size_t MultiCore_FreeListElementSize(MultiCore_FreeListHandle freelist) {
	assert(freelist != NULL);
	return freelist->elementSize;
}

void *MultiCore_FreeListAlloc(MultiCore_FreeListHandle fl) {
	assert(fl != NULL);

	Redo:;
	uint64_t head = Atomic_Load_U64(&fl->freeListHead);
	// check to see if the free list is empty
	if (head == END_OF_LIST_SENTINEL) {
		// we only allow N thread to allocate at once, the other spin wait
		// this stops the potentially fatal incorrect failure if N > maxBlocks
		uint64_t isAllocating = Atomic_Add_U64(&fl->currentAllocating, 1);
		if (isAllocating <= 2) {
			// we now allocate a new block in a lock free way
			bool retry = AllocNewBlock(fl);
			if (retry == false) {
				debug_printf("freelist has run out of space");
				return nullptr;
			}
		}
		Atomic_Add_U64(&fl->currentAllocating, -1);
		goto Redo;
	}

	uintptr_t next = *((uintptr_t *) head);

	if (Atomic_CompareExchange_U64(&fl->freeListHead, &head, next) != head) {
		goto Redo; // something changed reverse the transaction
	}

	// the item is now ours to abuse
	// clear it out ready for its new life
	memset((void *) head, 0x0, fl->elementSize);

	return (void *) head;

}

void MultiCore_FreeListRelease(MultiCore_FreeListHandle fl, void *vptr) {
	assert(fl != nullptr);
	assert(vptr != nullptr);

	Redo:;
	uint64_t head = Atomic_Load_U64(&fl->freeListHead);
	uint64_t ptr = (uint64_t) vptr;
	*((uintptr_t *) ptr) = head;

	if (Atomic_CompareExchange_U64(&fl->freeListHead, &head, ptr) != head) {
		goto Redo; // something changed reverse the transaction
	}
}

void MultiCore_FreeListReset(MultiCore_FreeListHandle fl, bool freeAllocatedMemory) {
	assert(fl != nullptr);

	uintptr_t endLink = END_OF_LIST_SENTINEL;
	for (uint32_t i = fl->maxBlocks-1; i >= 1; --i) {
		if (fl->blocks[i]) {
			if (!freeAllocatedMemory) {
				uint8_t *workPtr = (uint8_t *) fl->blocks[i];
				for (uint32_t j = 0u; j < fl->elementsPerBlockMask; ++j) {
					*((uintptr_t *) workPtr) = (uintptr_t) (workPtr + fl->elementSize);
					workPtr += fl->elementSize;
				}
				*((uintptr_t *) workPtr) = endLink;
				endLink = (uintptr_t) fl->blocks[i];
			} else {
				MEMORY_FREE(fl->blocks[i]);
				fl->blocks[i] = NULL;
			}
		}
	}

	uint8_t *workPtr = (uint8_t *) fl->blocks[0];
	for (uint32_t i = 0u; i < fl->elementsPerBlockMask; ++i) {
		*((uintptr_t *) workPtr) = (uintptr_t) (workPtr + fl->elementSize);
		workPtr += fl->elementSize;
	}
	*((uintptr_t *) workPtr) = endLink;
	fl->freeListHead = (uint64_t) fl->blocks[0];
	if(freeAllocatedMemory){
		fl->totalElementsAllocated = fl->elementsPerBlockMask+1;
	}
}

#endif