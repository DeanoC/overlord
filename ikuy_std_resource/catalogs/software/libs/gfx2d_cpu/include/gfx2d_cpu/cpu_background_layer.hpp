#pragma once
#include "gfx2d/layer.hpp"
#include "memory/memory.h"

struct Image_ImageHeader;

namespace Gfx2dCpu {

struct FrameBufferCPU;

struct CpuBackgroundLayer : public Gfx2d::Layer {
	FrameBufferCPU * frameBuffer;
	Image_ImageHeader const * image;
	bool dirty : 1;
	bool drawWhenClean : 1;
};

CpuBackgroundLayer * CreateCpuBackgroundLayer(FrameBufferCPU * frameBufferCpu,
                                              Image_ImageHeader const * image,
																							Memory_Allocator * allocator);

} // end namespace