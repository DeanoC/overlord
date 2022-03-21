#pragma once

#include "core/core.h"
#include "memory/memory.h"

typedef struct CADT_Vector *CADT_VectorHandle;

EXTERN_C CADT_VectorHandle CADT_VectorCreate(size_t elementSize, Memory_Allocator* allocator);
EXTERN_C void CADT_VectorDestroy(CADT_VectorHandle handle);

EXTERN_C Memory_Allocator* CADT_VectorGetAllocator(CADT_VectorHandle handle);
EXTERN_C CADT_VectorHandle CADT_VectorClone(CADT_VectorHandle handle);

EXTERN_C size_t CADT_VectorElementSize(CADT_VectorHandle handle);

EXTERN_C size_t CADT_VectorSize(CADT_VectorHandle handle);
EXTERN_C void CADT_VectorResize(CADT_VectorHandle handle, size_t size);
EXTERN_C void CADT_VectorResizeWithZero(CADT_VectorHandle handle, size_t size);
EXTERN_C void CADT_VectorResizeWithDefault(CADT_VectorHandle handle, size_t size, void const* defaultData);

EXTERN_C bool CADT_VectorIsEmpty(CADT_VectorHandle handle);

EXTERN_C size_t CADT_VectorCapacity(CADT_VectorHandle handle);
EXTERN_C void CADT_VectorReserve(CADT_VectorHandle handle, size_t size);

EXTERN_C void* CADT_VectorAt(CADT_VectorHandle handle, size_t index);

EXTERN_C size_t CADT_VectorPushElement(CADT_VectorHandle handle, void const* element);
EXTERN_C size_t CADT_VectorPushElementCloneToEnd(CADT_VectorHandle handle, size_t srcIndex);
EXTERN_C void CADT_VectorReplace(CADT_VectorHandle handle, size_t srcIndex,	size_t dstIndex);
EXTERN_C void CADT_VectorSwap(CADT_VectorHandle handle, size_t index0, size_t index1);

EXTERN_C void CADT_VectorPopElement(CADT_VectorHandle handle, void* out);
EXTERN_C void CADT_VectorPopAndDiscardElement(CADT_VectorHandle handle);
EXTERN_C void* CADT_VectorPeekElement(CADT_VectorHandle handle);

EXTERN_C void* CADT_VectorData(CADT_VectorHandle handle);
EXTERN_C void CADT_VectorRemove(CADT_VectorHandle handle, size_t index);
// swaps the last element into index and resizes, faster than remove
EXTERN_C void CADT_VectorSwapRemove(CADT_VectorHandle handle, size_t index);

// returns the index of a peice data or -1 if not found. Linear find so slow for large vectors
EXTERN_C size_t CADT_VectorFind(CADT_VectorHandle handle, void const	* data);

