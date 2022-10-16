#pragma once

#include "drawerbase.hpp"

namespace GfxDebug {

struct RGBA8 : public DrawerBase {
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
		backgroundColour = PackABGR( a, r, g, b);
	}

	void setBackgroundColour(uint8_t index) override {
		backgroundColour = palette[index];
	}

	void setPenColour(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
		penColour = PackABGR( a, r, g, b);
	}
	void setPenColour(uint8_t index) override {
		penColour = palette[index];
	}

	void Clear() const override;
	void PutString(int col, int row, char const *str) const override;
	void PutChar(int col, int row, char c) const override;

	void SetPixel(int x, int y, uint8_t index) override;

	void SetPixel(int x, int y, uint8_t r, uint8_t g, uint8_t b, uint8_t a);

private:
	void PutChar8(int col, int row, char c) const;
	void PutChar8x16(int col, int row, char c) const;

	constexpr static uint32_t PackABGR(uint8_t alpha, uint8_t red, uint8_t green, uint8_t blue) {
		return (alpha << 24) | (blue << 16) | (green << 8) | (red << 0);
	}
	const static uint32_t palette[16];
};

}