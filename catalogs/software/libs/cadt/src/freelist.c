#include "core/core.h"
#include "dbg/assert.h"
#include "dbg/print.h"
#include "memory/memory.h"
#include "cadt/freelist.h"
#include "dbg/ansi_escapes.h"

typedef struct CADT_FreeList {
		size_t elementSize;
		size_t capacity;

		uintptr_t headIndex;
		Memory_Allocator * allocator;
} CADT_FreeList;

#define END_OF_LIST_SENTINEL (~0UL)

CADT_FreeListHandle CADT_FreeListCreate(size_t elementSize, size_t capacity, Memory_Allocator* allocator) {
	assert(elementSize >= sizeof(uintptr_t));
	assert(capacity > 0);

	if(elementSize < sizeof(uintptr_t)) {
		debug_printf("Free List element size has minimum %lu\n", sizeof(uintptr_t));
		elementSize = sizeof(uintptr_t);
	}

	// we allocate 1 extra element for the header if header is smaller than an element,
	// whilst this may waste some space it ensure alignment of all elements
	size_t const headerSize = (sizeof(CADT_FreeList) > elementSize) ? sizeof(CADT_FreeList) : elementSize;
	size_t const totalSize = headerSize + (capacity * elementSize);
	CADT_FreeList *freelist = MAALLOC(allocator, totalSize, elementSize);
	if (freelist == nullptr) {
		debug_print("Free List Create out of alloc failed\n");
		return nullptr;
	}

	freelist->elementSize = elementSize;
	freelist->headIndex = 0;
	freelist->capacity = capacity;
	freelist->allocator = allocator;

	uint8_t *data = ((uint8_t *)freelist) + headerSize;
	for (size_t i = 0; i < capacity - 1; ++i) {
		*(uintptr_t *) data = i + 1;
		data += freelist->elementSize;
	}
	*(uintptr_t *) data = END_OF_LIST_SENTINEL;

	return freelist;
}

void CADT_FreeListDestroy(CADT_FreeListHandle freelist) {
	assert(freelist != nullptr);
	MFREE(freelist->allocator, freelist);
}

CADT_FreeListHandle CADT_FreeListClone(CADT_FreeListHandle ofreelist) {
	assert(ofreelist != nullptr);
	size_t const size = sizeof(CADT_FreeList) + sizeof(ofreelist->elementSize) * ofreelist->capacity;

	CADT_FreeList *freelist = MAALLOC(ofreelist->allocator, size, ofreelist->elementSize);
	if (freelist == nullptr) return nullptr;

	memcpy(freelist, ofreelist, size);

	return freelist;
}

size_t CADT_FreeListElementSize(CADT_FreeListHandle freelist) {
	assert(freelist != nullptr);
	return freelist->elementSize;
}

void *CADT_FreeListAlloc(CADT_FreeListHandle freelist) {
	assert(freelist != nullptr);

	size_t const headerSize = (sizeof(CADT_FreeList) > freelist->elementSize) ? sizeof(CADT_FreeList) : freelist->elementSize;

	if (freelist->headIndex == END_OF_LIST_SENTINEL) {
		debug_print( ANSI_RED_PAPER "FreeList out of free items\n" ANSI_BLACK_PAPER);
		return nullptr;
	}

	uint8_t *data = ((uint8_t *)freelist) + headerSize;
	data = data + (freelist->headIndex * freelist->elementSize);
	freelist->headIndex = *(uintptr_t *) data;

	return data;
}

void CADT_FreeListRelease(CADT_FreeListHandle freelist, void *ptr) {
	assert(freelist != nullptr);
	assert(ptr != nullptr);

	size_t const headerSize = (sizeof(CADT_FreeList) > freelist->elementSize) ? sizeof(CADT_FreeList) : freelist->elementSize;

	uint8_t *data = ((uint8_t *)freelist) + headerSize;
	assert((uint8_t *) ptr >= data);
	assert((uint8_t *) ptr < data + freelist->elementSize * freelist->capacity);

	*((uintptr_t *) ptr) = freelist->headIndex;
	freelist->headIndex = ((uint8_t *) ptr - data) / freelist->elementSize;
}
