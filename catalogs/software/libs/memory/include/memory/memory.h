// License Summary: MIT see LICENSE file
#pragma once
#include "core/core.h"

#undef STACK_ALLOC
#define STACK_ALLOC(x) __builtin_alloca(x)

// by default we enable memory tracking setup, which adds as small cost to every
// alloc (cost of a function and a few copies) however it also causes the exe
// to be bloated by file/line info at every alloc call site.
// so for final master set MEMORY_TRACKING_SETUP to 0 to remove this overhead
// as well
// Whether memory tracking is actually done (not just the setup) is decided
// inside memory.c
#undef MEMORY_TRACKING_SETUP
#define MEMORY_TRACKING_SETUP 0


#ifndef MEMORY_TRACKING_SETUP
#define MEMORY_TRACKING_SETUP 1
#endif

#undef memory
struct Memory_Allocator;

typedef void* (*Memory_MallocFunc)(struct Memory_Allocator* allocator, size_t size);
typedef void* (*Memory_AallocFunc)(struct Memory_Allocator* allocator, size_t size, size_t align);
typedef void* (*Memory_CallocFunc)(struct Memory_Allocator* allocator, size_t count, size_t size);
typedef void* (*Memory_ReallocFunc)(struct Memory_Allocator* allocator, void* memory, size_t size);
typedef void (*Memory_Free)(struct Memory_Allocator* allocator, void* memory);

typedef struct Memory_Allocator {
	Memory_MallocFunc malloc;
	Memory_AallocFunc aalloc;
	Memory_CallocFunc calloc;
	Memory_ReallocFunc realloc;
	Memory_Free free;
} Memory_Allocator;

typedef struct Memory_LinearAllocator {
		Memory_Allocator allocatorFuncs;
		void * bufferStart;
		void * bufferEnd;
		uint8_t * current;
		void * last; // when free is called after alloc with no other allocs in between, we can undo it
} Memory_LinearAllocator;

#if CPU_host
#define MEMORY_HEAP_ALLOCATOR_SIZE (sizeof(Memory_Allocator))
#else
#define MEMORY_HEAP_ALLOCATOR_SIZE (sizeof(Memory_Allocator) + CPU_CORE_COUNT * sizeof(void*))
#endif

// for global heap allocators, you must call Memory_HeapAllocatorInit(name) before any use
#define GLOBAL_HEAP_ALLOCATOR(name_)                                                     \
	uint8_t name_##_BLOCK[MEMORY_HEAP_ALLOCATOR_SIZE];               \
	Memory_Allocator* name_ = (Memory_Allocator * )name_##_BLOCK;

#define HEAP_ALLOCATOR(name_)                                                            \
	uint8_t name_##_BLOCK[MEMORY_HEAP_ALLOCATOR_SIZE];                                     \
	Memory_Allocator* name_ = (Memory_Allocator * )name_##_BLOCK;                         \
	Memory_HeapAllocatorInit(name_);

#define PERSISTANT_MEMORY_BUFFER_ALLOCATOR(name_, buffer_, sizeInBytes_)  \
	memcpy(&(name_), &Memory_LinearAllocatorTEMPLATE, sizeof(Memory_LinearAllocator)); \
	(name_).bufferStart = (void *) (buffer_);                                       \
	(name_).current = (uint8_t *) (name_).bufferStart;                        \
	(name_).bufferEnd = (void *)((name_).current + (sizeInBytes_));


#define MEMORY_BUFFER_ALLOCATOR(name_, buffer_, sizeInBytes_)                           \
	Memory_LinearAllocator name_##_BLOCK;                                                 \
	Memory_Allocator* name_ = (Memory_Allocator * )&name_##_BLOCK;                         \
	PERSISTANT_MEMORY_BUFFER_ALLOCATOR( name_##_BLOCK, buffer_, sizeInBytes_)

	// add some space when stack allocing and tracking for some overhead so exact stack allocation works
#if MEMORY_TRACKING_SETUP == 1
#define MEMORY_STACK_ALLOCATOR(name, sizeInBytes) MEMORY_BUFFER_ALLOCATOR(name, STACK_ALLOC((sizeInBytes)+64), (sizeInBytes)+64)
#else
#define MEMORY_STACK_ALLOCATOR(name, sizeInBytes) MEMORY_BUFFER_ALLOCATOR(name, STACK_ALLOC(sizeInBytes), sizeInBytes)
#endif
// this should be called before any malloc or heap is setup. In tracking build its sets up the tracking heap
EXTERN_C void Memory_MallocInit();
EXTERN_C void Memory_MallocFinish();

// call this at exit, when tracking is on will log all non freed items, if no tracking does nothing
EXTERN_C void Memory_TrackerDestroyAndLogLeaks();
EXTERN_C uint64_t Memory_TrackerBreakOnAllocNumber; // set before the allocation occurs to break in memory tracking (0 disables)

EXTERN_C Memory_LinearAllocator const Memory_LinearAllocatorTEMPLATE; // do not use directly use, use macro
EXTERN_C void Memory_HeapAllocatorInit(void* heapAllocator);          // do not use directly except with GLOBAL_HEAP_ALLOCATOR
EXTERN_C void Memory_HeapAllocatorFinish(void* heapAllocator);        // Free the heap and all the memory owned by it

#if MEMORY_TRACKING_SETUP == 1

EXTERN_C void * Memory_TrackedMalloc(Memory_Allocator * allocator_,
                                   char const * sourceFile_,
                                   unsigned int sourceLine_,
                                   char const * sourceFunc_,
                                   size_t size_);
EXTERN_C void * Memory_TrackedAalloc(Memory_Allocator * allocator_,
                                    char const * sourceFile_,
                                    unsigned int sourceLine_,
                                    char const * sourceFunc_,
                                    size_t size_,
																		size_t align_);
EXTERN_C void * Memory_TrackedCalloc(Memory_Allocator * allocator_,
                                     char const * sourceFile_,
                                     unsigned int sourceLine_,
                                     char const * sourceFunc_,
                                     size_t count_,
                                     size_t size_);
EXTERN_C void * Memory_TrackedRealloc(Memory_Allocator * allocator_,
                                     const char * sourceFile_,
                                     unsigned int sourceLine_,
                                     char const * sourceFunc_,
																		 void * address_,
                                     size_t size_);
EXTERN_C bool Memory_TrackedFree(Memory_Allocator * allocator_, void * address_);


#define Memory_TrackingPaddingSize 4
#define MALLOC(allocator, size) Memory_TrackedMalloc(allocator, __FILE__, __LINE__, __func__, (size))
#define MAALLOC(allocator, size, align) Memory_TrackedAalloc(allocator, __FILE__, __LINE__, __func__, (size), (align))
#define MCALLOC(allocator, count, size) Memory_TrackedCalloc(allocator, __FILE__, __LINE__, __func__, (count), (size))
#define MREALLOC(allocator, orig, size) Memory_TrackedRealloc(allocator, __FILE__, __LINE__, __func__, (orig), (size))
#define MFREE(allocator, ptr) Memory_TrackedFree(allocator, ptr)

// to use tracking on custom allocated, add these in the same way trackedMalloc etc in memory.c does for the
// default platform allocator (e.g. adjust size of alloc except aligned alloc and call tracked after ur custom alloc)
ALWAYS_INLINE size_t Memory_TrackerCalculateActualSize(const size_t reportedSize) {
	return reportedSize + Memory_TrackingPaddingSize * sizeof(uint32_t) * 2;
}

#else

#define MALLOC(allocator, size) allocator->malloc(allocator, size)
#define MAALLOC(allocator, size, align) allocator->aalloc(allocator, size, align)
#define MCALLOC(allocator, count, size) allocator->calloc(allocator, count, size)
#define MREALLOC(allocator, orig, size) allocator->realloc(allocator, orig, size)
#define MFREE(allocator, ptr) allocator->free(allocator, ptr)

#endif
