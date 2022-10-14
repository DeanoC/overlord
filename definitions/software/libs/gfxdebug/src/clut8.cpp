#include "core/core.h"
#include "gfxdebug/clut8.hpp"
#include "gfxdebug/fonts.hpp"
#include "dbg/print.h"

namespace GfxDebug {

CLUT8::CLUT8() :
	backgroundColour(0x0),
	penColour(0xF),
	width(0),
	height(0),
	fontZoom(1),
	frameBuffer(nullptr) {
}

CLUT8::CLUT8(uint16_t	width, uint16_t height, uint8_t *framebuffer) :
	backgroundColour(0x0),
	penColour(0xF),
	width (width),
	height(height),
	fontZoom(2),
	frameBuffer(framebuffer) {
}

void CLUT8::Clear() const {
	auto ptr = (uint8_t *) this->frameBuffer;
	memset(ptr, 0, width * height);
}

void CLUT8::SetPixel(int x, int y, uint8_t index) {
	auto *tl = (uint8_t *) this->frameBuffer;
	if(y >= this->height) return;
	if(x >= this->width) return;
	tl += (y * this->width) + x;
	*tl = index;
}

void CLUT8::PutChar(int col, int row, char c) const {
	// GFXDEBUG_FONTS_MINIMAL_MEMORY == 2 has both 8 and 16 height font and can swap at runtime
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
void CLUT8::PutChar8(int col, int row, char c) const {
	if (c < GFXDEBUG_FONTS_MIN_CHAR || c > GFXDEBUG_FONTS_MAX_CHAR)
#if GFXDEBUG_FONTS_MINIMAL_MEMORY == 2
		c = GFXDEBUG_FONTS_MAX_CHAR - GFXDEBUG_FONTS_MIN_CHAR;
#else
		c = ' ' - GFXDEBUG_FONTS_MIN_CHAR;
#endif
	else {
		c = (char)(((int)c) - GFXDEBUG_FONTS_MIN_CHAR);
	}

	if(this->fontZoom > MaxFontZoom)
		return;

	const int rowInLines = (row * 8 * this->fontZoom);
	const int colInPixels = (col * 8 * this->fontZoom);
	auto *tl = (uint8_t *) this->frameBuffer;
	tl += (rowInLines * this->width) + colInPixels;
	if(rowInLines >= this->height) return;
	if(colInPixels >= this->width) return;

	auto charLineBuf = (uint8_t*) fontTmpBuffer;
	for (int y = 0; y < 8; ++y) {
		uint8_t const fd = font8x8[(c * 8) + y];
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
			memcpy(tl, charLineBuf, 8*this->fontZoom);
			tl += this->width;
		}
	}
}

#if GFXDEBUG_FONTS_MINIMAL_MEMORY == 0
void CLUT8::PutChar8x16(int col, int row, char c) const {
	if (c < GFXDEBUG_FONTS_MIN_CHAR || c > GFXDEBUG_FONTS_MAX_CHAR)
#if GFXDEBUG_FONTS_MINIMAL_MEMORY == 2
		c = GFXDEBUG_FONTS_MAX_CHAR - GFXDEBUG_FONTS_MIN_CHAR;
#else
		c = ' ' - GFXDEBUG_FONTS_MIN_CHAR;
#endif
	else {
		c = (char)(((int)c) - GFXDEBUG_FONTS_MIN_CHAR);
	}
	if(this->fontZoom > MaxFontZoom)
		return;


	int const fz = this->fontZoom/2;
	const int rowInLines = (row * 16 * fz);
	const int colInPixels = (col * 8 * this->fontZoom);
	auto *tl = (uint32_t *) this->frameBuffer;

	if(rowInLines >= this->height) return;
	if(colInPixels >= this->width) return;

	tl += (rowInLines * this->width) + colInPixels;

	auto charLineBuf = (uint8_t*) fontTmpBuffer;
	memset(charLineBuf, 0xFF, 8*4*this->fontZoom);

	for (int y = 0; y < 16; ++y) {
		uint8_t const fd = font8x16[(c * 16) + y];

		for (int x = 0; x < 8; ++x) {
			bool const bit = !!(fd & (1 << (7-x)));
			uint8_t const colour = bit ? penColour : backgroundColour;

			switch(this->fontZoom) {
				case 2:
					charLineBuf[(x*2)+0] = colour;
					charLineBuf[(x*2)+1] = colour;
					break;
				case 1: charLineBuf[x] = colour; break;
				default:
					for (int i = 0; i < this->fontZoom; ++i) {
						charLineBuf[(x*this->fontZoom)+i] = colour;
					}
					break;
			}
		}

		for (int j = 0; j < fz; ++j) {
			memcpy(tl, charLineBuf, 8*this->fontZoom);
			tl += this->width;
		}
	}
}
#endif

void CLUT8::PutString(int col, int row, char const *str) const {
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