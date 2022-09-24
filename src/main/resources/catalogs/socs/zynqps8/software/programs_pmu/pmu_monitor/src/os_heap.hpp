#pragma once

#include "core/bitmap_allocator_single_threaded.hpp"
#include "host_interface.hpp"
#include "timers.hpp"
#include "osservices/osservices.h"

// the os heap is a 1MB chunk of DDR (at ddr base) that the pmu reserves
// for itself.
// if possible other CPUs should trap access to this to help catch NULLs
// it also ensure the data is safe if other CPU go mad
// The default MMU code for A53 only has 2MB pages so even though only 1MB
// so app should allocate a dummy 1MB or install a L3 MMU section

// its available once IPI3_OSServiceInit is finished
struct OsHeap {
	static void Init();
	[[maybe_unused]] static void Fini();
	const uint32_t nullBlock[1024]; // 4K poisoned to 0xDCDCDCDC for null page

	static const unsigned int TotalSize = 1 * 1024*1024;
	static const unsigned int UartBufferSize = 32 * 1024;
	static const unsigned int BounceBufferSize = 64 * 1024;

	uint8_t uartDEBUGTransmitBuffer[UartBufferSize]; // filled in an interrupt!
	uint8_t uartDEBUGReceiveBuffer[UartBufferSize]; // filled in an interrupt!

	BitmapAllocator_SingleThreaded<64*1024, 2046*16> ddrLoAllocator;
	BitmapAllocator_SingleThreaded<64*1024, 2048*16> ddrHiAllocator;

	BitmapAllocator_SingleThreaded<64, 4096> tmpOsBufferAllocator;
	uint8_t tmpBuffer[64 * 4096];

	uint8_t bounceBuffer[BounceBufferSize];

	Timers::Callback hundredHzCallbacks[Timers::MaxHundredHzCallbacks];
	Timers::Callback thirtyHzCallbacks[Timers::MaxThirtyHzCallbacks];

	uint8_t bootOCMStore[256*1024];
	HostInterface hostInterface;
	BootData bootData;

	static const int MaxMainCalls = 50;
	typedef void (*MainCallCallback)();
	MainCallCallback mainCallCallbacks[MaxMainCalls];
	uint32_t mainCallCallbacksIndex;

};

enum class HundredHzTasks {
	HOST_MAIN_CALLS = 0,
	HOST_INPUT,
	HOST_COMMANDS_PROCESSING
};

enum class ThirtyHzTasks {
};

static_assert(sizeof(OsHeap) < (OsHeap::TotalSize));

extern OsHeap *osHeap;

