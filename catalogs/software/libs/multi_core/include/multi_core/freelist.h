// License Summary: MIT see LICENSE file
#pragma once

#include "core/core.h"
#include "multi_core/atomics.h"

typedef struct MultiCore_FreeList {
		uint64_t elementSize;
		uint64_t maxBlocks;
		uint32_t elementsPerBlockMask;
		uint32_t elementsPerBlockShift;

		uint64_t freeListHead;
		void* * blocks;
		uint64_t totalElementsAllocated;
		uint64_t currentAllocating;
} MultiCore_FreeList;

typedef struct MultiCore_FreeList *MultiCore_FreeListHandle;

// free threaded (any thread can call at any time)
EXTERN_C size_t MultiCore_FreeListElementSize(MultiCore_FreeListHandle handle);
EXTERN_C void* MultiCore_FreeListAlloc(MultiCore_FreeListHandle fl);
EXTERN_C void MultiCore_FreeListRelease(MultiCore_FreeListHandle handle, void* ptr);

// not thread safe!
EXTERN_C MultiCore_FreeListHandle MultiCore_FreeListCreate(size_t elementSize, uint32_t blockCount, uint32_t maxBlocks);
EXTERN_C void MultiCore_FreeListDestroy(MultiCore_FreeListHandle handle);
EXTERN_C void MultiCore_FreeListReset(MultiCore_FreeListHandle handle, bool freeAllocatedMemory);
