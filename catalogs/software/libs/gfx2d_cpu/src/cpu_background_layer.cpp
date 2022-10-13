//
// Created by deano on 9/28/22.
//
#include "core/core.h"
#include "gfx2d_cpu/cpu_background_layer.hpp"
#include "gfx2d_cpu/framebuffer.hpp"
#include "memory/memory.h"
#include "tiny_image_format/tiny_image_format_query.h"
#include "image/image.h"
#include "core/math.h"

namespace Gfx2dCpu {

static void DestroyFunc(Gfx2d::Layer * layer) {
	MFREE(layer->allocator, layer);
}

static void CopyClipped(CpuBackgroundLayer * l, Gfx2d::Point2d dest, Gfx2d::Rect2d clippedRect) {
	uint32_t const pixelSize = TinyImageFormat_BitSizeOfBlock(l->frameBuffer->format) / 8;
	uint32_t const srcStride = Image_ByteCountPerRowOf(l->image);
	uint32_t const destStride = l->frameBuffer->dims.w * pixelSize;

	uint8_t const * curSrc = ((uint8_t const *)Image_RawDataPtr(l->image)) + clippedRect.origin.y * srcStride;
	uint8_t * curDest = l->frameBuffer->frameBuffer + dest.y * destStride + (dest.x * pixelSize);
	for(int y = 0;y < clippedRect.size.h;++y) {
		memcpy( curDest, curSrc + (clippedRect.origin.x * pixelSize), (clippedRect.size.w * pixelSize) );
		curSrc += srcStride;
		curDest += destStride;
	}

}
static void DrawFunc(Gfx2d::Layer * layer, Gfx2d::Viewport const * view) {
	auto l = (CpuBackgroundLayer *) layer;
	if(l->dirty || l->drawWhenClean) {
		uint16_t const mw = Math_Min_U16(view->size.w, l->image->width);
		uint16_t const mh = Math_Min_U16(view->size.h, l->image->height);
		CopyClipped(l, Gfx2d::Point2d{ 0, 0}, Gfx2d::Rect2d{ {0,0}, { mw, mh}});
		l->dirty = false;
	}
}

static Gfx2d::LayerFuncTable FuncTable = {
	.destroyFunc = DestroyFunc,
	.drawFunc = DrawFunc,
};

CpuBackgroundLayer * CreateCpuBackgroundLayer(FrameBufferCPU * frameBufferCpu,
																							Image_ImageHeader const * image,
																							Memory_Allocator * allocator) {
	auto backgroundLayer = (CpuBackgroundLayer *) MALLOC(allocator, sizeof(CpuBackgroundLayer));
	backgroundLayer->allocator = allocator;
	backgroundLayer->funcTable = &FuncTable;
	backgroundLayer->frameBuffer = frameBufferCpu;
	backgroundLayer->image = image;
	backgroundLayer->dirty = true;
	backgroundLayer->drawWhenClean = true;

	return backgroundLayer;
}

} // end namespace