#pragma once

#include "utils/string_utils.hpp"
#include "gfxdebug/rgba8.hpp"
#include "dbg/assert.h"

namespace GfxDebug {

struct ConsoleBase {
	struct Attribute {
		uint8_t bright: 1;
		uint8_t flash: 1;
		uint8_t pen: 3;
		uint8_t back: 3;
	};

	void Init(uint16_t w, uint16_t h, uint8_t* tb, Attribute* ab) {
		dirty = true;
		flashState = false;
		width = w;
		height = h;
		curCol = 0;
		curRow = 0;
		currentAttribute = DefaultAttribute;
		textBuffer = tb;
		attributeBuffer = ab;
		memset(textBuffer, 0, width * height);
		memset(attributeBuffer, *(uint8_t *) &DefaultAttribute, width * height);
	}
	uint32_t ProcessANSI(uint32_t size, char const *str, uint32_t i);

	constexpr static Attribute const DefaultAttribute = {0, 0, 0x7, 0};

	bool dirty;
	bool flashState;
	uint16_t width;
	uint16_t height;
	uint16_t curCol;
	uint16_t curRow;
	Attribute currentAttribute;
	uint8_t * textBuffer;
	Attribute* attributeBuffer;

};

template<int WIDTH, int HEIGHT>
struct Console : ConsoleBase {
	static const uint8_t ThirtyHzFlashFlipCount = 3;

	constexpr static uint32_t PackRGB(uint8_t red, uint8_t green, uint8_t blue) {
		return (0xFF << 24) | (blue << 16) | (green << 8) | (red << 0);
	}

	// VGA Palette (except Yellow)
	constexpr static const uint32_t palette[] = {
			PackRGB(0, 0, 0), // Black
			PackRGB(170, 0, 0), // Red
			PackRGB(0, 170, 0), // Green
			PackRGB(187, 187, 0), // Yellow
			PackRGB(0, 0, 170), // Blue
			PackRGB(170, 0, 170), // Magenta
			PackRGB(0, 170, 170), // Cyan
			PackRGB(170, 170, 170), // White

			PackRGB(85, 85, 85), // Bright Black (Gray)
			PackRGB(255, 85, 85), // Bright Red
			PackRGB(85, 255, 85), // Bright Green
			PackRGB(255, 255, 85), // Bright Yellow
			PackRGB(85, 85, 255), // Bright Blue
			PackRGB(255, 85, 255), // Bright Magenta
			PackRGB(85, 255, 255), // Bright Cyan
			PackRGB(255, 255, 255), // Bright White
	};

	void Init() {
		ConsoleBase::Init(WIDTH, HEIGHT, textArray, attributeArray);
		ThirtyHzCounter = 0;
	}

	void Display(RGBA8 *drawer, uint16_t offsetX, uint16_t offsetY) {
		bool flashUpdate = false;
		ThirtyHzCounter++;
		if(ThirtyHzCounter > ThirtyHzFlashFlipCount) {
			flashState ^= 1;
			flashUpdate = true;
			ThirtyHzCounter = 0;
		}

		if (!dirty && !flashUpdate) {
			return;
		}
		Attribute attrib = DefaultAttribute;
		for (int y = 0; y < HEIGHT; ++y) {
			for (int x = 0; x < WIDTH; ++x) {
				Attribute const a = attributeBuffer[x + (y * WIDTH)];

				bool const charNeedsUpdate = dirty || a.flash || attrib.flash;
				if(charNeedsUpdate) {
					if (a.back != attrib.back ||
							a.pen != attrib.pen ||
							a.bright != attrib.bright ||
							a.flash != attrib.flash) {
						if (a.flash && this->flashState) {
							drawer->penColour = palette[a.back];
							drawer->backgroundColour = palette[(a.bright * 8) + a.pen];
						} else {
							drawer->penColour = palette[(a.bright * 8) + a.pen];
							drawer->backgroundColour = palette[a.back];
						}
						attrib = a;
					}

					char const c = textBuffer[x + (y * WIDTH)];
					drawer->PutChar(offsetX + x, offsetY + y, c);
				}
			}
		}
		dirty = false;
	}

	void NewLine() {
		if (curRow == HEIGHT - 1) {
			memcpy(&textBuffer[0], &textBuffer[1 * WIDTH], WIDTH * (HEIGHT - 1));
			memset(&textBuffer[WIDTH * (HEIGHT - 1)], 0, WIDTH);
			dirty = true;
		} else {
			curRow++;
		}
		curCol = 0;
	}

	void PrintWithSize(uint32_t size, char const *str) {
		if (str == nullptr) {
			return;
		}

		uint32_t i = 0U;
		while (i < size) {
			char const c = str[i];
			switch (c) {
				// ANSI escape code
				case 0x1B: i = ProcessANSI(size, str, ++i);
					continue;
				case 0x0A: // 0x0A aka 10 - line feed
					NewLine();
					break;
				case 0x0D: //  0x0D aka 13 - carriage return
					curCol = 0;
					break;
				default:
					textBuffer[(curRow * WIDTH) + curCol] = c;
					attributeBuffer[(curRow * WIDTH) + curCol] = currentAttribute;
					curCol++;
					break;
			}
			i++;
		}
		dirty = true;
	}

	void PrintWithSizeLn(uint32_t size, char const *str) {
		PrintWithSize(size, str);
		NewLine();
	}
	void Print(char const *str) {
		PrintWithSize(Utils::StringLength(str));
	}

	void PrintLn(char const *str) {
		PrintWithSizeLn(Utils::StringLength(str), str);
	}

	uint8_t ThirtyHzCounter;
	uint8_t textArray[HEIGHT * WIDTH];
	Attribute attributeArray[HEIGHT * WIDTH];

};

}