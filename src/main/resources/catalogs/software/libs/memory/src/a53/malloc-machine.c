#include "core/core.h"
#include "dbg/assert.h"
#include "malloc-machine.h"
#include "osservices/osservices.h"

void * FAKE_mmap(void *addr, size_t length, int prot, int flags,int fd, int64_t offset) {
	assert(addr == 0);
	assert(prot ==  (PROT_READ|PROT_WRITE))
	assert(flags == (MAP_PRIVATE|MAP_ANONYMOUS));
	assert(fd == -1);
	assert(offset == 0);

	assert((length & ((1 << 20)-1)) == 0);

	return (void*) (uintptr_t) OsService_DdrLoBlockAlloc(length >> 20);
}

// only supports entire chunk that was mapped being unmapped
int FAKE_munmap(void *addr, size_t length) {
	assert((length & ((1 << 20)-1)) == 0);
	OsService_DdrLoBlockFree((uintptr_lo_t)(uintptr_t)addr);
	return 0;
}
