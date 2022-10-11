#include "core/core.h"
#include "dbg/print.h"
#include "dbg/assert.h"
#include "gfx2d_cpu/framebuffer.hpp"
#include "gfx2d_cpu/sprite_cpu.hpp"
#include "tiny_image_format/tiny_image_format_query.h"

namespace Gfx2dCpu {

typedef void (*Draw)(SpriteCpu * sprite, FrameBufferCPU * frameBuffer, Gfx2d::Point2d dest);
typedef void (*DrawClipped)(SpriteCpu * sprite, FrameBufferCPU * frameBuffer, Gfx2d::Point2d dest, Gfx2d::Rect2d clippedRect);

struct DrawFuncTable {
	Draw draw;
	DrawClipped drawClipped;
};

static void CopyDraw(SpriteCpu * sprite, FrameBufferCPU * frameBuffer, Gfx2d::Point2d dest) {
//	debug_printf("CopyDraw (%i, %i)\n", dest.x, dest.y);
	uint32_t const pixelSize = TinyImageFormat_BitSizeOfBlock(frameBuffer->format) / 8;
	uint32_t const srcStride = sprite->stride;
	uint32_t const destStride = frameBuffer->dims.w * pixelSize;
	uint8_t const * curSrc = sprite->spriteData;
	uint8_t * curDest = frameBuffer->frameBuffer + (dest.y * destStride) + (dest.x * pixelSize);
	for(int y = 0;y < sprite->dims.h;++y) {
		memcpy( curDest, curSrc, sprite->dims.w * pixelSize );
		curSrc += srcStride;
		curDest += destStride;
	}
}
void CopyClipped(SpriteCpu * sprite, FrameBufferCPU * frameBuffer, Gfx2d::Point2d dest, Gfx2d::Rect2d clippedRect) {
//	debug_printf("CopyClipped clipped (%i, %i) (%i, %i) to  (%i, %i)\n",
//	             clippedRect.origin.x, clippedRect.origin.y, clippedRect.size.w, clippedRect.size.h, dest.x, dest.y);

	uint32_t const pixelSize = TinyImageFormat_BitSizeOfBlock(frameBuffer->format) / 8;
	uint32_t const srcStride = sprite->stride;
	uint32_t const destStride = frameBuffer->dims.w * pixelSize;

	uint8_t const * curSrc = sprite->spriteData + clippedRect.origin.y * srcStride;
	uint8_t * curDest = frameBuffer->frameBuffer + dest.y * destStride + (dest.x * pixelSize);
	for(int y = 0;y < clippedRect.size.h;++y) {
		memcpy( curDest, curSrc + (clippedRect.origin.x * pixelSize), (clippedRect.size.w * pixelSize) );
		curSrc += srcStride;
		curDest += destStride;
	}

}

static void PunchThroughDraw(SpriteCpu * sprite, FrameBufferCPU * frameBuffer, Gfx2d::Point2d dest) {
	debug_printf("PunchThroughDraw\n");
}
void PunchThroughClipped(SpriteCpu * sprite, FrameBufferCPU * frameBuffer, Gfx2d::Point2d dest, Gfx2d::Rect2d clippedRect) {
	debug_printf("PunchThroughClipped clipped (%i, %i) (%i, %i)\n",
							 clippedRect.origin.x, clippedRect.origin.y, clippedRect.size.w, clippedRect.size.h);
}

static DrawFuncTable DrawTypeFuncTable[(uint8_t)SpriteCpuDrawType::MAX_DRAW_TYPES+1] = {
	{
		CopyDraw,
		CopyClipped,
	},
	{
		PunchThroughDraw,
		PunchThroughClipped,
	}
};


void SpriteCpu::Draw(FrameBufferCPU * frameBuffer, Gfx2d::Point2d dest) {
	assert( (uint8_t)this->drawType + !this->solid <= (uint8_t)SpriteCpuDrawType::MAX_DRAW_TYPES );
	DrawTypeFuncTable[(uint8_t)this->drawType + !this->solid].draw(this, frameBuffer, dest);
}

void SpriteCpu::DrawClipped(FrameBufferCPU * frameBuffer, Gfx2d::Point2d dest, Gfx2d::Rect2d clippedRect) {
	assert( (uint8_t)this->drawType + !this->solid < (uint8_t)SpriteCpuDrawType::MAX_DRAW_TYPES );
	DrawTypeFuncTable[(uint8_t)this->drawType + !this->solid].drawClipped(this, frameBuffer, dest, clippedRect);
}

} // end namesapce