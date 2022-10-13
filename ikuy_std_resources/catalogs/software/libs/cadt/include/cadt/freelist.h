#pragma once

#include "core/core.h"
#include "memory/memory.h"

typedef struct CADT_FreeList *CADT_FreeListHandle;

EXTERN_C CADT_FreeListHandle CADT_FreeListCreate(size_t elementSize, size_t capacity, Memory_Allocator* allocator);
EXTERN_C void CADT_FreeListDestroy(CADT_FreeListHandle handle);
EXTERN_C CADT_FreeListHandle CADT_FreeListClone(CADT_FreeListHandle handle);

EXTERN_C size_t CADT_FreeListElementSize(CADT_FreeListHandle handle);

EXTERN_C void* CADT_FreeListAlloc(CADT_FreeListHandle handle);
EXTERN_C void CADT_FreeListRelease(CADT_FreeListHandle handle, void* ptr);