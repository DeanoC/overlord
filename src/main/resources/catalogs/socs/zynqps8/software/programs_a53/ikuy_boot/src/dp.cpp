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

void BringUpDisplayPort() {
	debug_printf( ANSI_YELLOW_PEN "BringUpDisplayPort\n" ANSI_RESET_ATTRIBUTES );
	using namespace DisplayPort::Display;

	Init( &display );
	Init( &mixer );
	Init( &link );
	CopyStandardVideoMode( DisplayPort::Display::StandardVideoMode::VM_640_480_60, &display.videoTiming );
	if(videoBlock == 0) videoBlock = OsService_DdrLoBlockAlloc( 16 ); // 1MB
	Cache_DCacheZeroRange( videoBlock, 1 * 1024 * 1024 );

	auto dmaDesc = (DMADescriptor *) (uintptr_t) videoBlock;
	FrameBuffer = (uintptr_lo_t) (uintptr_t) (videoBlock + 4096);

	Init( dmaDesc );
	dmaDesc->enableDescriptorUpdate = true;
	dmaDesc->transferSize = 1280 * 480;
	dmaDesc->width = (1280);
	dmaDesc->stride = (1280) >> 4;
	dmaDesc->nextDescriptorAddress = (uint32_t) (uintptr_t) dmaDesc;
	dmaDesc->nextDescriptorAddressExt = (uint32_t) (((uintptr_t) dmaDesc) >> 32ULL);
	dmaDesc->sourceAddress = (uint32_t) FrameBuffer;
	dmaDesc->sourceAddressExt = (uint32_t) (((uintptr_t) FrameBuffer) >> 32ULL);

	mixer.function = DisplayPort::Display::MixerFunction::GFX;
	mixer.globalAlpha = 0x80;

	mixer.videoPlane.source = DisplayPort::Display::DisplayVideoPlane::Source::DISABLED;
	mixer.videoPlane.format = DisplayPort::Display::DisplayVideoPlane::Format::RGB8;
	mixer.videoPlane.simpleDescPlane0Address = (uintptr_t) dmaDesc;

	mixer.gfxPlane.source = DisplayPort::Display::DisplayGfxPlane::Source::BUFFER;
	mixer.gfxPlane.format = DisplayPort::Display::DisplayGfxPlane::Format::RGBA5551;
	mixer.gfxPlane.simpleDescBufferAddress = (uintptr_t) dmaDesc;
	Cache_DCacheCleanAndInvalidateRange( videoBlock, 1 * 1024 * 1024 );

	if(videoBlock == 0) {
		if(!IsDisplayConnected( &link ))
			return;
	}

	SetDisplay( &link, &display, &mixer );

#define DP_AV_BUF_PALETTE_MEMORY_OFFSET 0x0000b400U
	for(int i = 0; i < 256;i++) {
		hw_RegWrite(DP_BASE_ADDR, DP_AV_BUF_PALETTE_MEMORY_OFFSET + (256 * 0) + (i * 4), (i*2) << 4); // blue
		hw_RegWrite(DP_BASE_ADDR, DP_AV_BUF_PALETTE_MEMORY_OFFSET + (256 * 4) + (i * 4), i << 4); // green
		hw_RegWrite(DP_BASE_ADDR, DP_AV_BUF_PALETTE_MEMORY_OFFSET + (256 * 8) + (i * 4), (i*3) << 4); // red
		hw_RegWrite(DP_BASE_ADDR, DP_AV_BUF_PALETTE_MEMORY_OFFSET + (256 * 12) + (i * 4), 0x0); // alpha? doesn't appear to work
	}

	uint16_t * fb = (uint16_t *)FrameBuffer;
	for(int i=0;i < 480;++i) {
		for(int j = 0;j < 640;++j) {
			if(((j & 64) ^ (i & 64)) == 0) fb[j] = (255 - j) | (255 - j);
			else fb[j] = j | (j << 8);
		}
//		memset( fb, i & 0xFF, 640);
		fb += 640;
	}

}
