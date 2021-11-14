#pragma once
struct GfxDebugRGBA8 {
	GfxDebugRGBA8() : width(0), height(0), fontZoom(1), frameBuffer(nullptr) {}

	explicit GfxDebugRGBA8(uint16_t width, uint16_t height, uint8_t *framebuffer) :
			width(width),
			height(height),
			fontZoom(2),
			frameBuffer(framebuffer) {}

	uint16_t width;
	uint16_t height;
	int fontZoom;
	uint8_t *frameBuffer;

	void Clear(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
	void PutChar(int col, int row, char c);
	void PutString(int col, int row, char const *str);

};
