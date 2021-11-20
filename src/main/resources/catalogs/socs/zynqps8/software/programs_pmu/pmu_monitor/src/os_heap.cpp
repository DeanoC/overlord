#include "core/core.h"
#include "hw/memory_map.h"
#include "os_heap.hpp"

void OsHeap::Init() {
	// allocate the 1MB DDR heap for the OS
	osHeap = (OsHeap *) DDR_DDR4_0_BASE_ADDR;

	// gcc warns osHeap is null, but really its just DDR start is 0x0 address
	// so turn off the null checker just here.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnonnull"
	memset(osHeap, 0, OsHeap::TotalSize);
#pragma GCC diagnostic pop

	osHeap->ddrLoAllocator.Init(OsHeap::TotalSize);
	osHeap->ddrHiAllocator.Init(0);

	// fill in a poison 'null' page
	memset((void *) (osHeap->nullBlock), 0xDC, sizeof(osHeap->nullBlock));

	osHeap->tmpOsBufferAllocator.Init((uintptr_lo_t) (uintptr_t)osHeap->tmpBuffer);
}

[[maybe_unused]] void OsHeap::Fini() {

}
