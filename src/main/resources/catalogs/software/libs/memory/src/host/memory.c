//
// Created by deano on 3/14/22.
//

#include "core/core.h"
#include "dbg/print.h"
#include "dbg/assert.h"
#include "platform/host/platform.h"

#if IKUY_HOST_PLATFORM == IKUY_HOST_PLATFORM_WINDOWS
#include "malloc.h"
// on win32 we only have 8-byte alignment guaranteed, but the CRT provides special aligned allocation fns
void * platformMalloc(size_t size) {
	return _aligned_malloc(size, 16);
}

void * platformAalloc(size_t size, size_t align) {
	return _aligned_malloc(size, align);
}

void * platformCalloc(size_t count, size_t size) {
	if (count == 0) {
		return NULL;
	}
	void * mem = _aligned_malloc(count * size, 16);
	if (mem) {
		memset(mem, 0, count * size);
	}
	return mem;
}

void * platformRealloc(void *ptr, size_t size) {
	return _aligned_realloc(ptr, size, 16);
}

void platformFree(void *ptr) {
	_aligned_free(ptr);
}

#elif IKUY_HOST_PLATFORM == IKUY_HOST_PLATFORM_UNIX || IKUY_HOST_PLATFORM_OS == IKUY_HOST_OS_OSX
#include <stdlib.h>

void * platformMalloc(size_t size)
{
	void * mem;
	posix_memalign(&mem, 16, size);
	if(!mem) {
		debug_printf("platformMalloc(%zu) failed\n", size);
		return nullptr;
	}
	return mem;
}

void * platformAalloc(size_t size, size_t align)
{
	void* mem;
	posix_memalign(&mem, align, size);
	if(!mem) {
		debug_printf("platformAalloc(%zu) failed\n", size);
		return nullptr;
	}
	return mem;
}

void * platformCalloc(size_t count, size_t size)
{
	if(count == 0) {
		return nullptr;
	}

	void* mem;
	posix_memalign(&mem, 16, count * size);
	if(!mem) {
		debug_printf("platformCalloc(%zu) failed\n", count * size);
		return nullptr;
	}

	if(mem) {
		memset(mem, 0, count * size);
	}
	return mem;
}

void * platformRealloc(void* ptr, size_t size) {
	// technically this appears to be a bit dodgy but given
	// chromium and ffmpeg do this according to
	// https://trac.ffmpeg.org/ticket/6403
	// i'll live with it and assert it
	ptr = realloc(ptr, size);
	assert(((uintptr_t) ptr & 0xFUL) == 0);
	if(!ptr) {
		debug_printf("platformRealloc(%zu) failed\n", size);
		return nullptr;
	}
	return ptr;
}

void platformFree(void* ptr)
{
	if(ptr) free(ptr);
}

#else

void * platformAalloc(size_t size, size_t align)
{
		int const offset = align - 1 + sizeof(void*);
		void* p1 = malloc(size + offset);

		if (p1 == NULL) return NULL;

		void** p2 = (void**)(((size_t)(p1) + offset) & ~(align - 1));
		p2[-1] = p1;

		return p2;
}

void * platformMalloc(size_t size)
{
	return platformAalloc(size, 16);
}

void * platformCalloc(size_t count, size_t size)
{
	if(count == 0) {
		return NULL;
	}

	void * mem = platformMalloc(count * size);
	if(mem) {
		memset(mem, 0, count * size);
	}
	return mem;
}

void * platformRealloc(void* ptr, size_t size) {

	// TODO store align? or at least warn if realloc on unknown with align != 16
	int const align = 16;
	int const offset = align - 1 + sizeof(void*);
	void* p1 = realloc(ptr, size + offset);
	if(!p1) return NULL;

	void** p2 = (void**)(((size_t)(p1) + offset) & ~(align - 1));
	p2[-1] = p1;

	return p2;

}

void platformFree(void* ptr)
{
	free(((void**)ptr)[-1]);
}

#endif
