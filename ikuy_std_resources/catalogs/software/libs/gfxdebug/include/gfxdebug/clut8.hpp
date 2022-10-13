#pragma once

#include "core/core.h"
#include "drawerbase.hpp"
namespace GfxDebug {

// CLUT8 expects a palette setup with the first 16 colour set like so
// PackRGB(0, 0, 0), // Black
// PackRGB(170, 0, 0), // Red
// PackRGB(0, 170, 0), // Green
// PackRGB(187, 187, 0), // Yellow
// PackRGB(0, 0, 170), // Blue
// PackRGB(170, 0, 170), // Magenta
// PackRGB(0, 170, 170), // Cyan
// PackRGB(170, 170, 170), // White
//
// PackRGB(85, 85, 85), // Bright Black (Gray)
// PackRGB(255, 85, 85), // Bright Red
// PackRGB(85, 255, 85), // Bright Green
// PackRGB(255, 255, 85), // Bright Yellow
// PackRGB(85, 85, 255), // Bright Blue
// PackRGB(255, 85, 255), // Bright Magenta
// PackRGB(85, 255, 255), // Bright Cyan
// PackRGB(255, 255, 255), // Bright White
//
struct CLUT8 : public DrawerBase {
	static const int MaxFontZoom = 4;
	CLUT8();

	// framebuffer is assumed to be far away, fastTmpBuffer is near and should be 8 * 4 * max font zoom
	explicit CLUT8(uint16_t width, uint16_t height, uint8_t *framebuffer);

	uint8_t backgroundColour;
	uint8_t penColour;
	uint16_t width;
	uint16_t height;
	int fontZoom;
	uint8_t *frameBuffer;
	uint8_t *fontTmpBuffer[MaxFontZoom * 8];

	void setBackgroundColour(uint8_t index) override {
		backgroundColour = index;
	}

	void setPenColour(uint8_t index) override {
		penColour = index;
	}

	void Clear() const override;
	void PutString(int col, int row, char const *str) const override;
	void PutChar(int col, int row, char c) const override;

	void SetPixel(int x, int y, uint8_t index) override;

private:
	void PutChar8(int col, int row, char c) const;
	void PutChar8x16(int col, int row, char c) const;
};
} // end namespace