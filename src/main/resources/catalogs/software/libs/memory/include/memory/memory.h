// License Summary: MIT see LICENSE file
#pragma once
#include "core/core.h"

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

typedef void* (*Memory_MallocFunc)(size_t size);
typedef void* (*Memory_AallocFunc)(size_t size, size_t align);
typedef void* (*Memory_CallocFunc)(size_t count, size_t size);
typedef void* (*Memory_ReallocFunc)(void* memory, size_t size);
typedef void (*Memory_Free)(void* memory);

typedef struct Memory_Allocator {
	Memory_MallocFunc malloc;
	Memory_AallocFunc aalloc;
	Memory_CallocFunc calloc;
	Memory_ReallocFunc realloc;
	Memory_Free free;
} Memory_Allocator;

// always returns true
EXTERN_C bool Memory_TrackerPushNextSrcLoc(const char *sourceFile, const unsigned int sourceLine, const char *sourceFunc);

// call this at exit, when tracking is on will log all non freed items, if no tracking does nothing
EXTERN_C void Memory_TrackerDestroyAndLogLeaks();
EXTERN_C uint64_t Memory_TrackerBreakOnAllocNumber; // set before the allocation occurs to break in memory tracking (0 disables)

EXTERN_C Memory_Allocator Memory_GlobalAllocator;

#if MEMORY_TRACKING_SETUP == 1

#define Memory_TrackingPaddingSize 4
#define MALLOC(allocator, size) ((Memory_TrackerPushNextSrcLoc(__FILE__, __LINE__, __func__)) ? (allocator)->malloc(size) : nullptr)
#define MAALLOC(allocator, size, align) ((Memory_TrackerPushNextSrcLoc(__FILE__, __LINE__, __func__)) ? (allocator)->aalloc(size, align) : nullptr)
#define MCALLOC(allocator, count, size) ((Memory_TrackerPushNextSrcLoc(__FILE__, __LINE__, __func__)) ? (allocator)->calloc(count, size) : nullptr)
#define MREALLOC(allocator, orig, size) ((Memory_TrackerPushNextSrcLoc(__FILE__, __LINE__, __func__)) ? (allocator)->realloc(orig, size) : nullptr)
#define MFREE(allocator, ptr) (allocator)->free(ptr)

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

#define MALLOC(allocator, size) (allocator)->malloc(size)
#define MAALLOC(allocator, size, align) (allocator)->aalloc(size, align)
#define MCALLOC(allocator, count, size) (allocator)->calloc(count, size)
#define MREALLOC(allocator, orig, size) (allocator)->realloc(orig, size)
#define MFREE(allocator, ptr) (allocator)->free(ptr)

#define Memory_TrackerCalculateActualSize(reportedSize) (reportedSize)
EXTERN_C void *Memory_TrackedAlloc(const char * a,const unsigned int b, const char * c, const size_t d, void * e);
EXTERN_C void *Memory_TrackedAAlloc(const char * a, const unsigned int b, const char * c, const size_t d, void * e);
EXTERN_C void *Memory_TrackedRealloc(const char *a ,const unsigned int b, const char * c,const size_t d,void * e,void *f);

#endif

#undef STACK_ALLOC
#define STACK_ALLOC(x) __builtin_alloca(x)

#if __cplusplus
#include <new>
#define ALLOC_CLASS(clas, ...) new( (clas*) MALLOC(sizeof(clas))) clas(__VA_ARGS__)
#define FREE_CLASS(clas, ptr) ptr->~clas(); FREE(ptr);
#endif