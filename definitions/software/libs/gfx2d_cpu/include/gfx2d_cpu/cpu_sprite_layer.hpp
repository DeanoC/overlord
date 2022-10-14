//
// Created by deano on 9/28/22.
//

#pragma once

#include "gfx2d_cpu/sprite_cpu.hpp"
#include "gfx2d/layer.hpp"
#include "dbg/assert.h"
#include "dbg/print.h"

namespace Gfx2dCpu {

struct FrameBufferCPU;

struct CpuSpriteLayer : public Gfx2d::Layer {
	FrameBufferCPU *frameBuffer;
	uint16_t maxSprites;
	uint16_t numSprites;

	[[nodiscard]] SpriteCpu * GetSprite( uint16_t index ) const {

		assert(index < numSprites);
		return ((SpriteCpu*)(this + 1)) + index;
	}
	[[nodiscard]] uint16_t NewSprite() {
		assert(numSprites < maxSprites);
		return numSprites++;
	}
}; // followed by maxSprites SpriteCpu structures

CpuSpriteLayer *CreateCpuSpriteLayer( uint16_t maxSprites, FrameBufferCPU *frameBufferCpu, Memory_Allocator *allocator );

} // end namespace