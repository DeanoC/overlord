//
// Created by deano on 8/21/22.
//
#include "core/core.h"
#include "dbg/print.h"
#include "dbg/ansi_escapes.h"
#include "platform/cache.h"
#include "osservices/osservices.h"
#include "platform/memory_map.h"
#include "platform/reg_access.h"
#include "zynqps8/display_port/display.hpp"
#include "dp.hpp"

DisplayPort::Display::Connection link;
DisplayPort::Display::Display display;
DisplayPort::Display::Mixer mixer;
uintptr_lo_t videoBlock;
uintptr_lo_t FrameBuffer;

constexpr uint16_t MakeRGB565( uint8_t r_, uint8_t g_, uint8_t b_) {
	return (((uint16_t)r_)& 0x1f) << 11 | (((uint16_t)g_) & 0x3f) << 6 | (((uint16_t)b_) & 0x1f) << 0;
}

void BringUpDisplayPort() {
	debug_printf( ANSI_YELLOW_PEN "BringUpDisplayPort\n" ANSI_RESET_ATTRIBUTES );
	using namespace DisplayPort::Display;

	// 4K DMA descriptor space + 640 * 480 * 2B (RGB565)
	static constexpr int Blocks = 10; // 10 * 64KB
	static constexpr int TotalMem = Blocks * 64 * 1024;

	Init( &display );
	Init( &mixer );
	Init( &link );
	CopyStandardVideoMode( DisplayPort::Display::StandardVideoMode::VM_640_480_60, &display.videoTiming );
	if(videoBlock == 0) videoBlock = OsService_DdrLoBlockAlloc( Blocks, OS_SERVICE_TAG('V', 'I', 'D', 'B') );

	Cache_DCacheZeroRange( videoBlock, TotalMem );

	auto dmaDesc = (DMADescriptor *) (uintptr_t) videoBlock;
	FrameBuffer = (uintptr_lo_t) (uintptr_t) (videoBlock + 4096);

	Init( dmaDesc );
	dmaDesc->enableDescriptorUpdate = false;
	dmaDesc->transferSize = 640 * 480 * 2;
	dmaDesc->width = 640 * 2;
	dmaDesc->stride = (640 * 2) >> 4;
	dmaDesc->nextDescriptorAddress = (uint32_t) (uintptr_t) dmaDesc;
	dmaDesc->nextDescriptorAddressExt = (uint32_t) (((uintptr_t) dmaDesc) >> 32ULL);
	dmaDesc->sourceAddress = (uint32_t) FrameBuffer;
	dmaDesc->sourceAddressExt = (uint32_t) (((uintptr_t) FrameBuffer) >> 32ULL);

	mixer.function = DisplayPort::Display::MixerFunction::GFX;
	mixer.globalAlpha = 0x80;

	mixer.videoPlane.source = DisplayPort::Display::DisplayVideoPlane::Source::DISABLED;
	mixer.videoPlane.format = DisplayPort::Display::DisplayVideoPlane::Format::RGB8;

	mixer.gfxPlane.source = DisplayPort::Display::DisplayGfxPlane::Source::BUFFER;
	mixer.gfxPlane.format = DisplayPort::Display::DisplayGfxPlane::Format::RGB565;
	mixer.gfxPlane.simpleDescBufferAddress = (uintptr_t) dmaDesc;
	Cache_DCacheCleanAndInvalidateRange( videoBlock, TotalMem );

	if(videoBlock == 0) {
		if(!IsDisplayConnected( &link )) {
			debug_print("DP: No Display is connected\n");
			return;
		}
	}

	SetDisplay( &link, &display, &mixer );

	/*
#define DP_AV_BUF_PALETTE_MEMORY_OFFSET 0x0000b400U
	for(int i = 0; i < 256;i++) {
		hw_RegWrite(DP_BASE_ADDR, DP_AV_BUF_PALETTE_MEMORY_OFFSET + (256 * 0) + (i * 4), (i*2) << 4); // blue
		hw_RegWrite(DP_BASE_ADDR, DP_AV_BUF_PALETTE_MEMORY_OFFSET + (256 * 4) + (i * 4), i << 4); // green
		hw_RegWrite(DP_BASE_ADDR, DP_AV_BUF_PALETTE_MEMORY_OFFSET + (256 * 8) + (i * 4), (i*3) << 4); // red
		hw_RegWrite(DP_BASE_ADDR, DP_AV_BUF_PALETTE_MEMORY_OFFSET + (256 * 12) + (i * 4), 0x0); // alpha? doesn't appear to work
	}
	*/

	auto fb = (uint16_t *)(uintptr_all_t)FrameBuffer;
	for(int i=0;i < 480;++i) {
		for(int j = 0;j < 640;++j) {
			fb[j] = MakeRGB565(j, i, i/2);
		}
		fb += 640;
	}
	Cache_DCacheCleanAndInvalidateRange( videoBlock, TotalMem );

}
