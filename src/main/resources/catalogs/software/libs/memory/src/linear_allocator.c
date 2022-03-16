
#include "core/core.h"
#include "dbg/print.h"
#include "memory/memory.h"


static void * linearMalloc(Memory_Allocator* allocator, size_t size) {
	Memory_LinearAllocator* linearAllocator = (Memory_LinearAllocator*) allocator;
	uint8_t * const ret = linearAllocator->current;
	if(ret + size > (uint8_t *)linearAllocator->bufferEnd ) {
		debug_printf( "Linear Allocator run out of space\n");
		return nullptr;
	}
	linearAllocator->current = ret + size;
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
	if(!ret) {
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
		memcpy(ret, ptr, size);
		return ret;
	} else {
		return nullptr;
	}
}

static void linearFree(Memory_Allocator* allocator, void *ptr) {
	// do nothing
}

Memory_LinearAllocator const Memory_LinearAllocatorTEMPLATE = {
		.allocatorFuncs.malloc = &linearMalloc,
		.allocatorFuncs.aalloc = &linearAalloc,
		.allocatorFuncs.calloc = &linearCalloc,
		.allocatorFuncs.realloc = &linearRealloc,
		.allocatorFuncs.free = &linearFree,
};

