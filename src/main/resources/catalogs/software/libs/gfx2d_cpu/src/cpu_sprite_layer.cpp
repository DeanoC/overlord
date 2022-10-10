//
// Created by deano on 9/28/22.
//
#include "core/core.h"
#include "core/math.h"
#include "gfx2d_cpu/cpu_sprite_layer.hpp"
#include "gfx2d_cpu/framebuffer.hpp"
#include "memory/memory.h"
#include "tiny_image_format/tiny_image_format_query.h"
#include "dbg/print.h"

namespace Gfx2dCpu {

enum class ClipStatus : uint8_t {
	NoClipping,
	NeedsClipping,
	Culled
};

// output the status and if needs drawing the position and size of the source data to draw *from*
static ClipStatus Clip(Gfx2d::SpriteBase const * sprite, Gfx2d::Viewport const * view, Gfx2d::Point2d * outPoint, Gfx2d::Rect2d * outSrc) {
	assert(sprite);
	assert(view);
	assert(outSrc);

	// apply pivot transform
	outSrc->origin.x = sprite->position.x - sprite->pivot.x;
	outSrc->origin.y = sprite->position.y - sprite->pivot.y;
	outSrc->size.w = sprite->dims.w;
	outSrc->size.h = sprite->dims.h;

	// viewport coordinates of sprite
	int16_t vx = outSrc->origin.x - view->origin.x;
	int16_t vy = outSrc->origin.y - view->origin.y;
	outPoint->x  = Math_Clamp_I16(vx, 0, view->size.w);
	outPoint->y  = Math_Clamp_I16(vy, 0, view->size.h);

	// check if the sprite is fully outSrc of the view port
	bool const offLeft = (vx + outSrc->size.w) <= 0;
	bool const offTop = (vy + outSrc->size.h) <= 0;
	bool const offRight = vx >= view->size.w;
	bool const offBottom = vy >= view->size.h;
	if( offLeft || offTop || offRight || offBottom) return ClipStatus::Culled;

	// check if the sprite is fully in the viewport
	bool const onLeft = vx >= 0;
	bool const onTop = vy >= 0;
	bool const onRight = vx + outSrc->size.w < view->size.w;
	bool const onBottom = vy + outSrc->size.h < view->size.h;
	if( (onLeft && onTop) && (onRight && onBottom)) return ClipStatus::NoClipping;

	outSrc->origin.x = Math_Max_I16( 0 - vx, 0);
	outSrc->origin.y = Math_Max_I16( 0 - vy, 0);
	outSrc->size.w = Math_Min_I16( view->size.w - vx, outSrc->size.w) - outSrc->origin.x;
	outSrc->size.h = Math_Min_I16( view->size.h - vy, outSrc->size.h) - outSrc->origin.y;

	return ClipStatus::NeedsClipping;
}


static void DestroyFunc(Gfx2d::Layer * layer) {
	MFREE(layer->allocator, layer);
}

static void DrawFunc(Gfx2d::Layer * layer, Gfx2d::Viewport const * view) {
	auto const l = (CpuSpriteLayer *)layer;
	auto fb = l->frameBuffer;
	for(int i = 0; i < l->numSprites;++i) {
		auto spr = l->GetSprite(i);
		if(spr->enabled) {
			assert(spr->format == fb->format)
			Gfx2d::Point2d screenCoords {};
			Gfx2d::Rect2d srcCoords {};
			switch(Clip(spr, view, &screenCoords, &srcCoords)) {
				case ClipStatus::NoClipping:
					spr->Draw(fb, screenCoords);
					break;
				case ClipStatus::NeedsClipping:
					spr->DrawClipped(fb, screenCoords, srcCoords);
					break;
				case ClipStatus::Culled:
					break;
			}
		}
	}
}

static Gfx2d::LayerFuncTable FuncTable = {
	.destroyFunc = DestroyFunc,
	.drawFunc = DrawFunc,
};

CpuSpriteLayer * CreateCpuSpriteLayer( uint16_t maxSprites, FrameBufferCPU *frameBufferCpu, Memory_Allocator *allocator ) {
	auto size = sizeof( CpuSpriteLayer ) + (sizeof(SpriteCpu) * maxSprites);
	auto spriteLayer = (CpuSpriteLayer *) MCALLOC( allocator, 1, size );
	spriteLayer->allocator = allocator;
	spriteLayer->funcTable = &FuncTable;
	spriteLayer->frameBuffer = frameBufferCpu;
	spriteLayer->maxSprites = maxSprites;
	spriteLayer->numSprites = 0;

	return spriteLayer;
}

} // end namespace