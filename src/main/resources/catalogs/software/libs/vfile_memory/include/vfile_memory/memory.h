#pragma once

#include "core/core.h"
#include "memory/memory.h"

typedef struct VFileMemory_Data_t {
  void const *memory;
  Memory_Allocator* allocator;
  size_t size;
  size_t offset;
  bool autoFreeMemoryOnClose; // handle is always freed on close, if this is true free memory as well
} VFileMemory_Data_t;

EXTERN_C VFile_Handle VFileMemory_FromBuffer(void const *memory, size_t size, bool takeOwnership, Memory_Allocator* memoryAllocator);
EXTERN_C VFile_Handle VFileMemory_FromSize(size_t size, Memory_Allocator* memoryAllocator);
EXTERN_C VFile_Handle VFileMemory_CreateEmpty(Memory_Allocator* memoryAllocator);

#if MEMORY_TRACKING_SETUP == 1
#define VFileMemory_FromBuffer(memory, size, takeOwnership, memoryAllocator) ((Memory_TrackerPushNextSrcLoc(__FILE__, __LINE__, __FUNCTION__)) ? VFileMemory_FromBuffer(memory, size, takeOwnership, memoryAllocator) : NULL)
#define VFileMemory_FromSize(size, memoryAllocator) ((Memory_TrackerPushNextSrcLoc(__FILE__, __LINE__, __FUNCTION__)) ? VFileMemory_FromSize(size, memoryAllocator) : NULL)
#define VFileMemory_CreateEmpty(memoryAllocator) ((Memory_TrackerPushNextSrcLoc(__FILE__, __LINE__, __FUNCTION__)) ? VFileMemory_CreateEmpty(memoryAllocator) : NULL)

#endif