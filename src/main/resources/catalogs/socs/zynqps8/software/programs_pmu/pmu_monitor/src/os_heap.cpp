#include "core/core.h"
#include "platform/memory_map.h"
#include "os_heap.hpp"

void OsHeap::Init() {
	raw_debug_print("OsHeap::Init\n");
	// allocate the OsHeap::TotalSize DDR heap for the OS
	osHeap = (OsHeap *) DDR_0_BASE_ADDR;

	// gcc warns osHeap is null, but really its just DDR start is 0x0 address
	// so turn off the null checker just here.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnonnull"
	raw_debug_print("  osHeap clear\n");
	memset(osHeap, 0, OsHeap::TotalSize);
#pragma GCC diagnostic pop
	raw_debug_printf("  osHeap = start %08lx sizeof(OsHeap) = %iKB\n", (uint32_t)osHeap, sizeof(OsHeap)/1024);

	raw_debug_print("  osHeap ddrLoAllocator Init\n");
	osHeap->ddrLoAllocator.Init(OsHeap::TotalSize);
	raw_debug_print("  osHeap ddrHiAllocator Init\n");
	osHeap->ddrHiAllocator.Init(0);

	raw_debug_print("  Poison Null Page\n");
	// fill in a poison 'null' page
	memset((void *) (osHeap->nullBlock), 0xDC, sizeof(osHeap->nullBlock));

	raw_debug_print("  osHeap tmpOsBufferAllocator Init\n");
	osHeap->tmpOsBufferAllocator.Init((uintptr_lo_t) (uintptr_t)osHeap->tmpBuffer);

	raw_debug_print("OsHeap::Init Finished\n");

}

[[maybe_unused]] void OsHeap::Fini() {

}
