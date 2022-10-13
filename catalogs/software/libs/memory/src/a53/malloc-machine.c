#include "core/core.h"
#include "dbg/print.h"
#include "dbg/assert.h"
#include "malloc-machine.h"
#include "osservices/osservices.h"

void * FAKE_mmap(void *addr_, size_t length_, int prot_, int flags_,int fd_, int64_t offset_) {
	assert(addr_ == 0);
	assert(prot_ ==  (PROT_READ|PROT_WRITE))
	assert(flags_ == (MAP_PRIVATE|MAP_ANONYMOUS));
	assert(fd_ == -1);
	assert(offset_ == 0);
	assert((length_ & ((1 << 16)-1)) == 0);

	return (void*) (uintptr_t) OsService_DdrLoBlockAlloc(length_ >> 16, OS_SERVICE_TAG('M', 'M', 'a', 'p'));
}

// only supports entire chunk that was mapped being unmapped
int FAKE_munmap(void *addr_, size_t length_) {
	assert((length_ & ((1 << 16)-1)) == 0);
	OsService_DdrLoBlockFree((uintptr_lo_t)(uintptr_t)addr_, length_ >> 16, OS_SERVICE_TAG('M', 'M', 'a', 'p'));
	return 0;
}
