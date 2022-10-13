//
// Created by deano on 9/28/22.
//

#pragma once
#include "gfx2d/layer.hpp"
#include "memory/memory.h"

namespace Gfx2dCpu {

struct FrameBufferCPU;

struct CpuClearLayer : public Gfx2d::Layer {
	FrameBufferCPU * frameBuffer;
};

CpuClearLayer * CreateCpuClearLayer(FrameBufferCPU * frameBufferCpu, Memory_Allocator * allocator);

} // end namespace