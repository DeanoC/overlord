#include "core/core.h"
#include "dbg/assert.h"
#include "core/math.h"
#include "memory/memory.h"
#include "cadt/vector.h"

typedef struct CADT_Vector {
		size_t elementSize;
		size_t capacity;
		size_t size;
		Memory_Allocator* allocator;

		uint8_t* data;
} CADT_Vector;

void CADT_VectorDestroy(CADT_VectorHandle vector) {
	if(!vector) {
		return;
	}
	if(vector->data != NULL) {
		MFREE(vector->allocator, vector->data);
	}
	MFREE(vector->allocator, vector);
}

CADT_VectorHandle CADT_VectorClone(CADT_VectorHandle handle) {
	assert(handle != NULL);
	CADT_Vector* ovector = (CADT_Vector*)handle;
	CADT_Vector* nvector = (CADT_Vector*)CADT_VectorCreate(ovector->elementSize, ovector->allocator);
	if (nvector == NULL) return NULL;
	CADT_VectorResize(nvector, ovector->size);
	memcpy(nvector->data, ovector->data, ovector->size * ovector->elementSize);
	return nvector;
}

Memory_Allocator* CADT_VectorGetAllocator(CADT_VectorHandle vector) {
	assert(vector != NULL);
	return vector->allocator;
}

size_t CADT_VectorElementSize(CADT_VectorHandle handle) {
	assert(handle != NULL);
	CADT_Vector const* vector = (CADT_Vector const*)handle;
	return vector->elementSize;
}

size_t CADT_VectorSize(CADT_VectorHandle handle) {
	assert(handle != NULL);
	CADT_Vector const* vector = (CADT_Vector const*)handle;
	return vector->size;
}

void CADT_VectorResize(CADT_VectorHandle vector, size_t size) {
	assert(vector != NULL);

	CADT_VectorReserve(vector, size);
	vector->size = size;
}
void CADT_VectorResizeWithZero(CADT_VectorHandle vector, size_t size) {
	assert(vector != NULL);

	CADT_VectorReserve(vector, size);
	if (size > vector->size) {
		size_t const dif = size - vector->size;
		memset(vector->data + (vector->size * vector->elementSize), 0, dif * vector->elementSize);
	}
	vector->size = size;
}

void CADT_VectorResizeWithDefault(CADT_VectorHandle vector, size_t size, void const* defaultData) {
	if (!defaultData) {
		CADT_VectorResizeWithZero(vector, size);
		return;
	}

	assert(vector != NULL);

	CADT_VectorReserve(vector, size);
	if (size > vector->size) {
		size_t const dif = size - vector->size;
		for (size_t i = 0; i < dif; ++i) {
			memcpy(vector->data + ((vector->size + i) * vector->elementSize), defaultData, vector->elementSize);
		}
	}

	vector->size = size;
}

bool CADT_VectorIsEmpty(CADT_VectorHandle vector) {
	assert(vector != NULL);
	return (vector->size == 0);
}
size_t CADT_VectorCapacity(CADT_VectorHandle vector) {
	assert(vector != NULL);
	return vector->capacity;
}


void* CADT_VectorAt(CADT_VectorHandle vector, size_t index) {
	assert(vector != NULL);
	assert(index < vector->size);
	assert(vector->data);
	return vector->data + (index * vector->elementSize);
}

size_t CADT_VectorPushElement(CADT_VectorHandle vector, void const* element) {
	assert(vector != NULL);
	size_t const index = vector->size;
	CADT_VectorResize(vector, vector->size+1);
	assert(index < vector->size);

	memcpy(CADT_VectorAt(vector, index), element, vector->elementSize);
	return index;
}

void CADT_VectorPopElement(CADT_VectorHandle vector, void* out) {
	assert(vector != NULL);
	assert(vector->size > 0);
	size_t const index = vector->size-1;
	memcpy(out, CADT_VectorAt(vector, index), vector->elementSize);
	CADT_VectorResize(vector, vector->size-1);
}

void CADT_VectorPopAndDiscardElement(CADT_VectorHandle vector) {
	assert(vector != NULL);
	assert(vector->size > 0);
	CADT_VectorResize(vector, vector->size-1);
}

void* CADT_VectorPeekElement(CADT_VectorHandle vector) {
	assert(vector != NULL);
	assert(vector->size > 0);
	size_t const index = vector->size-1;
	return CADT_VectorAt(vector, index);
}

void CADT_VectorRemove(CADT_VectorHandle vector, size_t index) {
	assert(vector != NULL);
	assert(index < vector->size);
	if (vector->size == 1) {
		vector->size = 0;
		return;
	}
	if (vector->size-1 != index) {
		uint8_t* dst = vector->data + (index * vector->elementSize);
		uint8_t* src = dst + vector->elementSize;
		size_t copySize = (vector->size - index - 1) * vector->elementSize;

		memmove(dst, src, copySize);
	}

	vector->size = vector->size - 1;
}

void CADT_VectorReplace(CADT_VectorHandle vector, size_t srcIndex, size_t dstIndex) {
	assert(vector != NULL);
	memcpy(CADT_VectorAt(vector, dstIndex), CADT_VectorAt(vector, srcIndex), vector->elementSize);
}

void CADT_VectorSwap(CADT_VectorHandle vector, size_t index0, size_t index1) {
	assert(vector != NULL);
	void* tmp = STACK_ALLOC(vector->elementSize);
	memcpy(tmp, CADT_VectorAt(vector, index0), vector->elementSize);
	memcpy(CADT_VectorAt(vector, index0), CADT_VectorAt(vector, index1), vector->elementSize);
	memcpy(CADT_VectorAt(vector, index1), tmp, vector->elementSize);
}

void CADT_VectorSwapRemove(CADT_VectorHandle vector, size_t index) {
	assert(vector != NULL);
	CADT_VectorReplace(vector, vector->size - 1, index);
	CADT_VectorResize(vector, vector->size - 1);
}

void* CADT_VectorData(CADT_VectorHandle vector) {
	assert(vector != NULL);
	return vector->data;
}

size_t CADT_VectorFind(CADT_VectorHandle vector, void const* data) {
	assert(vector != NULL);
	for (size_t i = 0; i < vector->size; ++i) {
		if (memcmp(data, vector->data + (i * vector->elementSize), vector->elementSize) == 0) {
			return i;
		}
	}
	return (size_t)-1;
}

CADT_VectorHandle CADT_VectorCreate(size_t elementSize, Memory_Allocator* allocator) {
	CADT_Vector* vector = MCALLOC(allocator, 1, sizeof(CADT_Vector));
	if(vector == NULL) return nullptr;
	vector->elementSize = elementSize;
	vector->allocator = allocator;
	return vector;
}

void CADT_VectorReserve(CADT_VectorHandle vector, size_t size) {
	assert(vector != NULL);

	// reserve always grews unless ShrinkToFit
	if(size <= vector->capacity) return;

	void* oldData = vector->data;
	size_t const oldCapacity = (vector->capacity) ? vector->capacity : 1;
	size_t const newCapacity = Math_Max_U64(oldCapacity*2, size);
	vector->capacity = newCapacity;
	vector->data = (uint8_t*) MCALLOC(vector->allocator, newCapacity, vector->elementSize);
	assert(vector->data);
	if(oldData) {
		memcpy(vector->data, oldData, vector->size * vector->elementSize);
		MFREE(vector->allocator, oldData);
	}
}