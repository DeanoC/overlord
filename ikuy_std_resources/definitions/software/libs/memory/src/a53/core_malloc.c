#include "core/core.h"
#include "memory/memory.h"
#include "multi_core/core_local.h"
#include "../malloc/dlmalloc.h"

typedef struct Memory_HeapAllocator {
		Memory_Allocator allocatorFuncs;
		CORE_LOCAL(void*, threadMSpaces);
} Memory_HeapAllocator;

#define INITIAL_PER_CORE_ALLOC (64*1024)
#define ALWAYS_LOCK 0

#if ALWAYS_LOCK == 1
#define NOLOCK false
#else
#define NOLOCK true
#endif

ALWAYS_INLINE void coreMemoryInit(Memory_HeapAllocator* heap) {
	if(READ_CORE_LOCAL(heap->threadMSpaces) == nullptr) {
		WRITE_CORE_LOCAL(heap->threadMSpaces, create_mspace(INITIAL_PER_CORE_ALLOC, true));
	}
}

static void* coreMallocMalloc(Memory_Allocator* allocator, size_t size) {
	Memory_HeapAllocator * heap = (Memory_HeapAllocator*)allocator;
	coreMemoryInit(heap);
	// TODO if we have run out of ram from our mspace, would could try and steal it from another cores mspace
	return mspace_malloc( READ_CORE_LOCAL(heap->threadMSpaces), size, NOLOCK);
}

static void* coreMallocAalloc(Memory_Allocator* allocator, size_t alignment, size_t size) {
	Memory_HeapAllocator * heap = (Memory_HeapAllocator*)allocator;
	coreMemoryInit(heap);
	return mspace_memalign(READ_CORE_LOCAL(heap->threadMSpaces), alignment, size, NOLOCK);
}

static void *coreMallocCalloc(Memory_Allocator* allocator, size_t count, size_t size) {
	Memory_HeapAllocator * heap = (Memory_HeapAllocator*)allocator;
	coreMemoryInit(heap);
	return mspace_calloc(READ_CORE_LOCAL(heap->threadMSpaces), count, size, NOLOCK);
}

static void* coreMallocRealloc(Memory_Allocator* allocator, void * mem, size_t size) {
	Memory_HeapAllocator * heap = (Memory_HeapAllocator*)allocator;
	coreMemoryInit(heap);
	mspace m = mspace_from_pointer(mem);
	if(m != READ_CORE_LOCAL(heap->threadMSpaces)) {
		return mspace_realloc(READ_CORE_LOCAL(heap->threadMSpaces), mem, size, false);
	}
	else {
		return mspace_realloc(READ_CORE_LOCAL(heap->threadMSpaces), mem, size, NOLOCK);
	}

}

static void coreMallocFree(Memory_Allocator* allocator, void* ptr) {
	Memory_HeapAllocator * heap = (Memory_HeapAllocator*)allocator;
	mspace m = mspace_from_pointer(ptr);
	if(m != READ_CORE_LOCAL(heap->threadMSpaces)) {
		// this will use a spin lock that could be contended as it is in another core's mspace
		mspace_free(m, ptr, false);
	} else {
		// allocated from our cores mspace so we can free with no contention
		mspace_free(m,ptr, NOLOCK);
	}
}

void Memory_HeapAllocatorInit(void* heapAllocator) {
	Memory_HeapAllocator* heap = (Memory_HeapAllocator*) heapAllocator;
	memset(heap, 0, sizeof(Memory_HeapAllocator));
	heap->allocatorFuncs.malloc = &coreMallocMalloc;
	heap->allocatorFuncs.aalloc = &coreMallocAalloc;
	heap->allocatorFuncs.calloc = &coreMallocCalloc;
	heap->allocatorFuncs.realloc = &coreMallocRealloc;
	heap->allocatorFuncs.free = &coreMallocFree;
}

void Memory_HeapAllocatorFinish(void* heapAllocator) {
	Memory_HeapAllocator* heap = (Memory_HeapAllocator*) heapAllocator;
	for(int i = 0; i < CPU_CORE_COUNT; i++) {
		destroy_mspace(heap->threadMSpaces[i]);
		heap->threadMSpaces[i] = nullptr;
	}
}
