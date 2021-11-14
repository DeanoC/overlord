#include "core/core.h"
#include "../include/gfxdebug_rgba8.hpp"

extern char font8x8_basic[][8];

void GfxDebugRGBA8::Clear(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
	uint32_t const colour = a << 24 | b << 16 | g << 8 | r << 0;
	auto *ptr = (uint32_t *) this->frameBuffer;
	for(int i =0; i < 1280*720;++i) {
		*ptr++ = colour;
	}
}
void GfxDebugRGBA8::PutChar(int col, int row, char c) {
	if(c <= 32 || c >= 128)
		return;

	const int rowInLines = (row * 8 * this->fontZoom);
	const int colInPixels = (col * 8 * this->fontZoom);
	auto *tl = (uint32_t*)this->frameBuffer;
	tl += (rowInLines * this->width) + colInPixels;

	for(int y = 0;y < 8;++y) {
		if(rowInLines+y > this->height) return;

		uint8_t const line = font8x8_basic[(int) c-33][y];
		for(int j=0;j < this->fontZoom;++j) {
			for (int x = 0; x < 8; ++x) {
				bool const bit = !!(line & (1 << x));
				uint32_t const colour = bit ? 0xFFFFFFFF : 0x0;
				for(int i=0;i < this->fontZoom;++i) {
					if((colInPixels + (x * this->fontZoom) + i) > this->width)
						continue;

					*(tl + ((x * this->fontZoom) + i)) = colour;
				}
			}
			tl += this->width;
		}
	}
}

void GfxDebugRGBA8::PutString(int col, int row, char const * str) {
	int startCol = col;
	while(*str != 0) {
		char const c = *str++;
		if(c == 10) { row++; col = startCol; }
		else PutChar(col++, row, c);
	}
}

