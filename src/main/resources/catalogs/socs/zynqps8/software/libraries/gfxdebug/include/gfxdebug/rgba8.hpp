#pragma once

namespace GfxDebug {

struct RGBA8 {
	RGBA8();

	explicit RGBA8(uint16_t width, uint16_t height, uint8_t *framebuffer);

	uint32_t backgroundColour;
	uint32_t penColour;
	uint16_t width;
	uint16_t height;
	int fontZoom;
	uint8_t *frameBuffer;

	void setBackgroundColour(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
		backgroundColour = a << 24 | b << 16 | g << 8 | r << 0;
	}

	void setPenColour(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
		penColour = a << 24 | b << 16 | g << 8 | r << 0;
	}

	void Clear() const;
	void PutString(int col, int row, char const *str) const;
	void PutChar(int col, int row, char c) const;

private:
	void PutChar8(int col, int row, char c) const;
	void PutChar8x16(int col, int row, char c) const;

};

}