//
// Created by deano on 9/26/22.
//

#pragma once
#include "core/core.h"
#include "gfx2d/sprite.hpp"

namespace Gfx2dCpu {

struct FrameBufferCPU;

enum class SpriteCpuDrawType : uint8_t {
	Copy,

	MAX_DRAW_TYPES
};

struct SpriteCpu : public Gfx2d::SpriteBase {
	uint8_t const * spriteData;
	uint32_t stride;
	SpriteCpuDrawType drawType;
	uint8_t padd[3];

	void Draw(FrameBufferCPU * frameBuffer, Gfx2d::Point2d dest);
	void DrawClipped(FrameBufferCPU * frameBuffer, Gfx2d::Point2d dest, Gfx2d::Rect2d clippedRect);

};

static_assert(sizeof(SpriteCpu) == 32);


} // end namespace