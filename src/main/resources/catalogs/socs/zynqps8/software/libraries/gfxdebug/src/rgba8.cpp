#include "core/core.h"
#include "gfxdebug/rgba8.hpp"
#include "gfxdebug/fonts.hpp"

namespace GfxDebug {

RGBA8::RGBA8() :
		backgroundColour(0x0),
		penColour(0xFFFFFFFF),
		width(0),
		height(0),
		fontZoom(1),
		frameBuffer(nullptr) {

}

RGBA8::RGBA8(uint16_t	width, uint16_t height, uint8_t *framebuffer) :
	backgroundColour(0x0),
	penColour(0xFFFFFFFF),
	width (width),
	height(height),
	fontZoom(2),
	frameBuffer(framebuffer) {
}

void RGBA8::Clear() const {
	auto ptr = (uint32_t *) this->frameBuffer;

	auto lineBuf = (uint32_t*) ALLOCA(width * 4);
	for(int i = 0;i < width;++i){
		lineBuf[i] = backgroundColour;
	}
	for (int i = 0; i < height; ++i) {
		memcpy(ptr, lineBuf, width * 4);
		ptr += width;
	}
}
void RGBA8::PutChar(int col, int row, char c) const {
#if GFXDEBUG_FONTS_MINIMAL_MEMORY != 0
	PutChar8(col, row, c);
#else
	if(this->fontZoom & 0x1) {
		PutChar8(col, row, c);
	} else {
		PutChar8x16(col, row, c);
	}
#endif
}

void RGBA8::PutChar8(int col, int row, char c) const {
	if (c < GFXDEBUG_FONTS_MIN_CHAR || c >= GFXDEBUG_FONTS_MAX_CHAR)
		return;

	const int rowInLines = (row * 8 * this->fontZoom);
	const int colInPixels = (col * 8 * this->fontZoom);
	auto *tl = (uint32_t *) this->frameBuffer;
	tl += (rowInLines * this->width) + colInPixels;
	if(rowInLines >= this->height) return;
	if(colInPixels >= this->width) return;

	auto charLineBuf = (uint32_t*) ALLOCA(8 * 4 * this->fontZoom);
	for (int y = 0; y < 8; ++y) {
		int ac = ((int)c) - GFXDEBUG_FONTS_MIN_CHAR;
		uint8_t const fd = font8x8[(ac * 8) + y];
		for (int x = 0; x < 8; ++x) {
#if GFXDEBUG_FONTS_MINIMAL_MEMORY == 2
			bool const bit = !!(fd & (1 << x));
#else
			bool const bit = !!(fd & (1 << (7-x)));
#endif
			uint32_t const colour = bit ? penColour : backgroundColour;
			switch(this->fontZoom) {
				case 2: charLineBuf[(x*2)+1] = charLineBuf[(x*2)] = colour; break;
				case 1: charLineBuf[x] = colour; break;
				default:
					for (int i = 0; i < this->fontZoom; ++i) {
						charLineBuf[x*this->fontZoom] = colour;
					}
			}
		}

		for (int j = 0; j < this->fontZoom; ++j) {
			memcpy(tl, charLineBuf, 8*this->fontZoom*4);
			tl += this->width;
		}
	}
}
#if GFXDEBUG_FONTS_MINIMAL_MEMORY == 0

void RGBA8::PutChar8x16(int col, int row, char c) const {
	if (c < GFXDEBUG_FONTS_MIN_CHAR || c >= GFXDEBUG_FONTS_MAX_CHAR)
		return;

	int const fz = this->fontZoom/2;
	const int rowInLines = (row * 16 * fz);
	const int colInPixels = (col * 8 * this->fontZoom);
	auto *tl = (uint32_t *) this->frameBuffer;

	if(rowInLines >= this->height) return;
	if(colInPixels >= this->width) return;

	tl += (rowInLines * this->width) + colInPixels;

	auto charLineBuf = (uint32_t*) ALLOCA(8 * 4 * this->fontZoom);
	for (int y = 0; y < 16; ++y) {
		int ac = ((int)c) - GFXDEBUG_FONTS_MIN_CHAR;
		uint8_t const fd = font8x16[(ac * 16) + y];

		for (int x = 0; x < 8; ++x) {
			bool const bit = !!(fd & (1 << (7-x)));
			uint32_t const colour = bit ? penColour : backgroundColour;
			switch(this->fontZoom) {
				case 2: charLineBuf[(x*2)+1] = charLineBuf[(x*2)] = colour; break;
				case 1: charLineBuf[x] = colour; break;
				default:
					for (int i = 0; i < this->fontZoom; ++i) {
						charLineBuf[x*this->fontZoom] = colour;
					}
			}
		}

		for (int j = 0; j < fz; ++j) {
			memcpy(tl, charLineBuf, 8*4*this->fontZoom);
			tl += this->width;
		}
	}
}
#endif

void RGBA8::PutString(int col, int row, char const *str) const {
	int startCol = col;
	while (*str != 0) {
		char const c = *str++;
		if (c == 10) {
			row++;
			col = startCol;
		} else
			PutChar(col++, row, c);
	}
}

} // end namespace