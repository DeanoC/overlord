// License Summary: MIT see LICENSE file
#pragma once
#include "core/core.h"

#undef STACK_ALLOC
#define STACK_ALLOC(x) __builtin_alloca(x)

// loweset level of malloc etc. usually you don't want to call directly
EXTERN_C void * platformMalloc(size_t size);
EXTERN_C void * platformAalloc(size_t size, size_t align);
EXTERN_C void * platformCalloc(size_t count, size_t size);
EXTERN_C void * platformRealloc(void *ptr, size_t size);
EXTERN_C void platformFree(void *ptr);

// by default we enable memory tracking setup, which adds as small cost to every
// alloc (cost of a function and a few copies) however it also causes the exe
// to be bloated by file/line info at every alloc call site.
// so for final master set MEMORY_TRACKING_SETUP to 0 to remove this overhead
// as well
// Whether memory tracking is actually done (not just the setup) is decided
// inside memory.c
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
		uint8_t* current;
} Memory_LinearAllocator;

// always returns true
EXTERN_C bool Memory_TrackerPushNextSrcLoc(const char *sourceFile, const unsigned int sourceLine, const char *sourceFunc);

// call this at exit, when tracking is on will log all non freed items, if no tracking does nothing
EXTERN_C void Memory_TrackerDestroyAndLogLeaks();
EXTERN_C uint64_t Memory_TrackerBreakOnAllocNumber; // set before the allocation occurs to break in memory tracking (0 disables)

EXTERN_C Memory_Allocator Memory_GlobalAllocator;
EXTERN_C Memory_LinearAllocator const Memory_LinearAllocatorTEMPLATE; // do not use directly use, copy it

#define MEMORY_BUFFER_ALLOCATOR(name_, buffer_, sizeInBytes_)    \
Memory_LinearAllocator name_##_BLOCK;                          \
Memory_Allocator* name_ = (Memory_Allocator * )&name_##_BLOCK;                       \
memcpy(&name_##_BLOCK, &Memory_LinearAllocatorTEMPLATE, sizeof(Memory_LinearAllocator)); \
name_##_BLOCK.bufferStart = (void *) (buffer_);                                 \
name_##_BLOCK.current = (uint8_t *) name_##_BLOCK.bufferStart;                 \
name_##_BLOCK.bufferEnd = (void *)(name_##_BLOCK.current + (sizeInBytes_));

#define MEMORY_STACK_ALLOCATOR(name, sizeInBytes) MEMORY_BUFFER_ALLOCATOR(name, STACK_ALLOC(sizeInBytes), sizeInBytes)

#if MEMORY_TRACKING_SETUP == 1

#define Memory_TrackingPaddingSize 4
#define MALLOC(allocator, size) ((Memory_TrackerPushNextSrcLoc(__FILE__, __LINE__, __func__)) ? (allocator)->malloc(allocator, size) : nullptr)
#define MAALLOC(allocator, size, align) ((Memory_TrackerPushNextSrcLoc(__FILE__, __LINE__, __func__)) ? (allocator)->aalloc(allocator, size, align) : nullptr)
#define MCALLOC(allocator, count, size) ((Memory_TrackerPushNextSrcLoc(__FILE__, __LINE__, __func__)) ? (allocator)->calloc(allocator, count, size) : nullptr)
#define MREALLOC(allocator, orig, size) ((Memory_TrackerPushNextSrcLoc(__FILE__, __LINE__, __func__)) ? (allocator)->realloc(allocator, orig, size) : nullptr)
#define MFREE(allocator, ptr) (allocator)->free(allocator, ptr)

// to use tracking on custom allocated, add these in the same way trackedMalloc etc in memory.c does for the
// default platform allocator (e.g. adjust size of alloc except aligned alloc and call tracked after ur custom alloc)
ALWAYS_INLINE size_t Memory_TrackerCalculateActualSize(const size_t reportedSize) {
	return reportedSize + Memory_TrackingPaddingSize * sizeof(uint32_t) * 2;
}

EXTERN_C void *Memory_TrackedAlloc(const char *sourceFile,
																				 const unsigned int sourceLine,
																				 const char *sourceFunc,
																				 const size_t reportedSize,
																				 void *actualSizedAllocation);
EXTERN_C void *Memory_TrackedAAlloc(const char *sourceFile,
																					const unsigned int sourceLine,
																					const char *sourceFunc,
																					const size_t reportedSize,
																					void *actualSizedAllocation);
EXTERN_C void *Memory_TrackedRealloc(const char *sourceFile,
																					 const unsigned int sourceLine,
																					 const char *sourceFunc,
																					 const size_t reportedSize,
																					 void *reportedAddress,
																					 void *actualSizedAllocation);
EXTERN_C bool Memory_TrackedFree(const void *reportedAddress);

#else

#define MALLOC(allocator, size) (allocator)->malloc(allocator, size)
#define MAALLOC(allocator, size, align) (allocator)->aalloc(allocator, size, align)
#define MCALLOC(allocator, count, size) (allocator)->calloc(allocator, count, size)
#define MREALLOC(allocator, orig, size) (allocator)->realloc(allocator, orig, size)
#define MFREE(allocator, ptr) (allocator)->free(allocator, ptr)

#define Memory_TrackerCalculateActualSize(reportedSize) (reportedSize)
EXTERN_C void *Memory_TrackedAlloc(const char * a,const unsigned int b, const char * c, const size_t d, void * e);
EXTERN_C void *Memory_TrackedAAlloc(const char * a, const unsigned int b, const char * c, const size_t d, void * e);
EXTERN_C void *Memory_TrackedRealloc(const char *a ,const unsigned int b, const char * c,const size_t d,void * e,void *f);

#endif
