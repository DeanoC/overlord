#pragma once

namespace GfxDebug {

struct RGBA8 {
	static const int MaxFontZoom = 4;
	RGBA8();

	// framebuffer is assumed to be far away, fastTmpBuffer is near and should be 8 * 4 * max font zoom
	explicit RGBA8(uint16_t width, uint16_t height, uint8_t *framebuffer);

	uint32_t backgroundColour;
	uint32_t penColour;
	uint16_t width;
	uint16_t height;
	int fontZoom;
	uint8_t *frameBuffer;
	uint8_t *fontTmpBuffer[MaxFontZoom * 8 * 4];

	void setBackgroundColour(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
		backgroundColour = a << 24 | b << 16 | g << 8 | r << 0;
	}

	void setPenColour(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
		penColour = a << 24 | b << 16 | g << 8 | r << 0;
	}

	void Clear() const;
	void PutString(int col, int row, char const *str) const;
	void PutChar(int col, int row, char c) const;

	void SetPixel(int x, int y, uint32_t val);
	void SetPixel(int x, int y, uint8_t r, uint8_t g, uint8_t b, uint8_t a);


private:
	void PutChar8(int col, int row, char c) const;
	void PutChar8x16(int col, int row, char c) const;

};

}