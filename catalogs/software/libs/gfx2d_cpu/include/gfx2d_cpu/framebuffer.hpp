#pragma once

#include "gfx2d/framebuffer.hpp"

namespace Gfx2dCpu {

struct FrameBufferCPU : public Gfx2d::FrameBuffer {
	uint8_t* frameBuffer;
};

} // end namespace