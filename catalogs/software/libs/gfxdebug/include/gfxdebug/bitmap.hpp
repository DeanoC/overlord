#pragma once
#include "core/core.h"

namespace GfxDebug {

struct BitmapBase {

	BitmapBase(uint16_t width_, uint16_t height_, uint8_t * buffer_);

	uint16_t const width;
	uint16_t const height;
	uint16_t const widthInBytes;

	uint8_t * const bitmapBuffer;

	void clear() {
		memset(bitmapBuffer, 0, widthInBytes * height);
	}

	void setPixel(uint16_t x_, uint16_t y_, bool pixel_);
	bool getPixel(uint16_t x_, uint16_t y_);

	// x will be round down to nearest 8 pixels and replaced with pixels
	void set8AlignedPixels(uint16_t x_, uint16_t y_, uint8_t pixels);
	uint8_t get8AlignedPixels(uint16_t x_, uint16_t y_);
};

template<int WIDTH, int HEIGHT>
class Bitmap : public BitmapBase {

	Bitmap() : BitmapBase(WIDTH, HEIGHT, buffer) {
		clear();
	}

	uint8_t buffer[((WIDTH+7)>>3) * HEIGHT];
};

} // GfxDebug
