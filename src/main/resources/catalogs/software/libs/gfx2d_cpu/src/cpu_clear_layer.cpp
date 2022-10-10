//
// Created by deano on 9/28/22.
//
#include "core/core.h"
#include "gfx2d_cpu/cpu_clear_layer.hpp"
#include "gfx2d_cpu/framebuffer.hpp"
#include "memory/memory.h"
#include "tiny_image_format/tiny_image_format_query.h"

namespace Gfx2dCpu {

static void DestroyFunc(Gfx2d::Layer * layer) {
	MFREE(layer->allocator, layer);
}

static void DrawFunc(Gfx2d::Layer * layer, Gfx2d::Viewport const * view) {
	// cpu clean doesn't care about the window, it just set the framebuffer to 0
	auto l = (CpuClearLayer *) layer;
	auto fb = l->frameBuffer;
	auto size = fb->dims.w * fb->dims.h * TinyImageFormat_BitSizeOfBlock(fb->format) / 8;
	memset(fb->frameBuffer, 0, size);
}

static Gfx2d::LayerFuncTable FuncTable = {
	.destroyFunc = DestroyFunc,
	.drawFunc = DrawFunc,
};

CpuClearLayer * CreateCpuClearLayer(FrameBufferCPU * frameBufferCpu, Memory_Allocator * allocator) {
	auto clearLayer = (CpuClearLayer *) MALLOC(allocator, sizeof(CpuClearLayer));
	clearLayer->allocator = allocator;
	clearLayer->funcTable = &FuncTable;
	clearLayer->frameBuffer = frameBufferCpu;

	return clearLayer;
}

} // end namespace