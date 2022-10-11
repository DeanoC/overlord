//
// Created by deano on 9/26/22.
//

#pragma once
#include "core/core.h"
#include "library_defines/library_defines.h"

#include "core/utf8.h"
#include "core/utils.hpp"
#include "gfx2d/scene.hpp"
#include "tiny_image_format/tiny_image_format_base.h"

struct Image_ImageHeader;

namespace Gfx2d {

struct SpriteBase {
	Point2d position;
	Size2d dims;
	Point2d pivot;

	TinyImageFormat format;
	bool enabled : 1;
	bool solid : 1;
	bool flipX : 1;
	bool flipY : 1;
	uint8_t padd[1];

};

static_assert(sizeof(SpriteBase) == 16);

struct PACKED SpriteData {
	Image_ImageHeader const * image;
	utf8_int8_t const * handlerName;

	Point2d spriteSheetCoord;
	Size2d spriteDimension;
	Point2d spritePivotPoint;
};

} // end namespace