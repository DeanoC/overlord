//
// Created by deano on 7/8/22.
//

#include "gfxdebug/bitmap.hpp"

namespace GfxDebug {

BitmapBase::BitmapBase(uint16_t width_, uint16_t height_, uint8_t * buffer_) :
	width(width_),
	height(height_),
	widthInBytes(width_/8),
	bitmapBuffer(buffer_)
{
}

void BitmapBase::setPixel(uint16_t x_, uint16_t y_, bool pixel_) {
	uint8_t const pixels = get8AlignedPixels(x_, y_);
	uint8_t const bit = ((uint8_t)pixel_) << (x_ & 7);
	set8AlignedPixels(x_, y_, (pixels & ~bit) | bit);
}

bool BitmapBase::getPixel(uint16_t x_, uint16_t y_) {
	uint8_t pixels = get8AlignedPixels(x_, y_);
	return (pixels >> (x_ & 7)) & 0x1;
}

void BitmapBase::set8AlignedPixels(uint16_t x_, uint16_t y_, uint8_t pixels) {
	bitmapBuffer[ (y_ * widthInBytes) + (x_ >> 3) ] = pixels;
}

uint8_t BitmapBase::get8AlignedPixels(uint16_t x_, uint16_t y_) {
	return bitmapBuffer[ (y_ * widthInBytes) + (x_ >> 3) ];
}


} // GfxDebug