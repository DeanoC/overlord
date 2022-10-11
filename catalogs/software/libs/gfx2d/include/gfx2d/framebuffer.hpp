#pragma once

#include "gfx2d/base.hpp"
#include "gfx2d/layer.hpp"
#include "tiny_image_format/tiny_image_format_base.h"

namespace Gfx2d {

// a framebuffer is  a 2D array of pixels of format
struct FrameBuffer  {
	Gfx2d::Size2d dims;
	TinyImageFormat format;
};

} // end namespace