#include "core/core.h"
#include "dbg/print.h"
#include "dbg/assert.h"
#include "memory/memory.h"
#include "multi_core/core_local.h"
#include "../malloc/dlmalloc.h"

void *platformMalloc(size_t size);
void *platformAalloc(size_t size, size_t align);
void *platformCalloc(size_t count, size_t size);
void *platformRealloc(void *ptr, size_t size);
void platformFree(void *ptr);

static void *platformMallocAdaptor(Memory_Allocator*heap, size_t size) {
	return platformMalloc(size);
}
static void *platformAallocAdaptor(Memory_Allocator* heap, size_t size, size_t align) {
	platformAalloc(size, align);
}
static void *platformCallocAdaptor(Memory_Allocator* heap, size_t count, size_t size) {
	platformCalloc(count, size);
}
static void *platformReallocAdaptor(Memory_Allocator* heap, void *ptr, size_t size) {
	platformRealloc(ptr, size);
}
static void platformFreeAdaptor(Memory_Allocator* heap, void *ptr) {
	platformFree(ptr);
}


void Memory_HeapAllocatorInit(void* heapAllocator) {
	Memory_Allocator* heap = (Memory_Allocator*) heapAllocator;
	memset(heap, 0, sizeof(Memory_Allocator));
	heap->malloc = &platformMallocAdaptor;
	heap->aalloc = &platformAallocAdaptor;
	heap->calloc = &platformCallocAdaptor;
	heap->realloc = &platformReallocAdaptor;
	heap->free = &platformFreeAdaptor;
}

void Memory_HeapAllocatorFinish(void* heapAllocator) {
}