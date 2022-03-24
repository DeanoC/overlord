
#include "core/core.h"
#include "dbg/print.h"
#include "memory/memory.h"
#include "dbg/assert.h"

static void * linearMalloc(Memory_Allocator* allocator, size_t size) {
	Memory_LinearAllocator* linearAllocator = (Memory_LinearAllocator*) allocator;
	assert(linearAllocator->current < (uint8_t *)linearAllocator->bufferEnd);
	uint8_t * const ret = linearAllocator->current;
	size_t left = (uint8_t*)linearAllocator->bufferEnd - ret;

	if(size > left) {
		debug_printf( "Linear Allocator run out of space left %zi size %zu\n", left, size);
		return nullptr;
	}
	linearAllocator->current = ret + size;
	linearAllocator->last = ret;
	return ret;
}

static void * linearAalloc(Memory_Allocator* allocator, size_t size, size_t align) {
	Memory_LinearAllocator* linearAllocator = (Memory_LinearAllocator*) allocator;
	uintptr_t const addr = (uintptr_t) linearAllocator->current;
	linearAllocator->current = (uint8_t *)((addr + align - 1) & ~(align - 1));
	return linearMalloc(allocator, size);
}
static void * linearCalloc(Memory_Allocator* allocator, size_t count, size_t size) {
	void * ret = linearMalloc(allocator, size * count);
	if(ret) {
		memset(ret, 0, size * count);
		return ret;
	} else {
		return nullptr;
	}
}

static void * linearRealloc(Memory_Allocator* allocator, void *ptr, size_t size) {
	// we never extends always alloc and copy
	void * ret = linearMalloc(allocator, size);
	if(!ret) {
		return nullptr;
	} else {
		memcpy(ret, ptr, size);
		return ret;
	}
}

static void linearFree(Memory_Allocator* allocator, void *ptr) {
	Memory_LinearAllocator* linearAllocator = (Memory_LinearAllocator*) allocator;
	if(linearAllocator->last == ptr) {
		linearAllocator->current = linearAllocator->last;
	} else {
		// do nothing
	}
}

Memory_LinearAllocator const Memory_LinearAllocatorTEMPLATE = {
		.allocatorFuncs.malloc = &linearMalloc,
		.allocatorFuncs.aalloc = &linearAalloc,
		.allocatorFuncs.calloc = &linearCalloc,
		.allocatorFuncs.realloc = &linearRealloc,
		.allocatorFuncs.free = &linearFree,
};

