#pragma once

#include "core/cpp/bitmap_allocator_single_threaded.hpp"
#include "text_console.hpp"
#include "timers.hpp"

// the os heap is a 2MB chunk of DDR (at ddr base) that the pmu reserves
// for itself.
// if possible other CPUs should trap access to this to help catch NULLs
// it also ensure the data is safe if other CPU go mad

// its available once IPI3_OSServiceInit is finished
struct OsHeap {
	static void Init();
	const uint32_t nullBlock[1024]; // 4K poisoned to 0xDCDCDCDC for null page

	static const unsigned int TotalSize = 2 * 1024*1024;
	static const unsigned int UartBufferSize = 32 * 1024;
	static const unsigned int BounceBufferSize = 64 * 1024;

	uint8_t uart0TransmitBuffer[UartBufferSize]; // filled in an interrupt!
	uint8_t uart0ReceiveBuffer[UartBufferSize]; // filled in an interrupt!

	BitmapAllocator_SingleThreaded<1024*1024, 2046> ddrLoAllocator;
	BitmapAllocator_SingleThreaded<1024*1024, 2048> ddrHiAllocator;

	BitmapAllocator_SingleThreaded<64, 4096> tmpOsBufferAllocator;
	uint8_t tmpBuffer[64 * 4096];

	uint8_t bounceBuffer[BounceBufferSize];

	TextConsole console;

	Timers::Callback ThirtyHzCallbacks[Timers::MaxThirtyHzCallbacks];


};

static_assert(sizeof(OsHeap) < (OsHeap::TotalSize));

extern OsHeap *osHeap;

