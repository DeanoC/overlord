//
// Created by deano on 9/27/22.
//

#pragma once
#include "core/core.h"

namespace Gfx2d {

// TODO math vector core classes
struct Point2d {
	int16_t x, y;
};
struct Size2d {
	uint16_t w, h;
};

struct Rect2d {
	Point2d origin;
	Size2d size;
};

typedef Rect2d Viewport;

} // end namespace