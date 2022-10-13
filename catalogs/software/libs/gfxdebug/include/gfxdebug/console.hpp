#pragma once

#include "core/snprintf.h"
#include "utils/string_utils.hpp"
#include "gfxdebug/rgba8.hpp"
#include "gfxdebug/clut8.hpp"
#include "dbg/ansi_escapes.h"
#include "dbg/assert.h"
#include "dbg/print.h"

namespace GfxDebug {

struct ConsoleBase {
	struct Attribute {
		uint8_t bright: 1;
		uint8_t flash: 1;
		uint8_t pen: 3;
		uint8_t back: 3;
	};

	void Init(uint16_t w, uint16_t h, uint8_t* tb, Attribute* ab);
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

	void Init() {
		ConsoleBase::Init(WIDTH, HEIGHT, textArray, attributeArray);

#if GFXDEBUG_FONTS_MINIMAL_MEMORY == 0
		debug_print( ANSI_CYAN_PEN "8x16 Font + 8x8 Font (6144 bytes in data seg)\n");
		int fontBytes = 4096 + 2048;
#elif GFXDEBUG_FONTS_MINIMAL_MEMORY == 1
		debug_print( ANSI_CYAN_PEN "8x8 Font (2048 bytes in data seg)\n");
		int fontBytes = 2048;
#elif GFXDEBUG_FONTS_MINIMAL_MEMORY == 2
		debug_print( ANSI_CYAN_PEN "8x8 Font Printable Only (736 bytes in data seg)\n");
		int fontBytes = 736;
#endif
		int textBytes = WIDTH * HEIGHT * 2;

		debug_printf( "%i x %i text buffer (%i bytes) TOTAL: %i bytes" ANSI_WHITE_PEN "\n", WIDTH, HEIGHT, textBytes, textBytes + fontBytes);
		debug_print(ANSI_CYAN_PEN "Screen Console " ANSI_GREEN_PEN "OK \n" ANSI_WHITE_PEN);
		ThirtyHzCounter = 0;
	}

	void Display(DrawerBase * drawer, uint16_t offsetX, uint16_t offsetY) {
		bool flashUpdate = false;
		ThirtyHzCounter++;
		if(ThirtyHzCounter > ThirtyHzFlashFlipCount) {
			flashState ^= 1;
			flashUpdate = true;
			ThirtyHzCounter = 0;
		}

		if(!dirty && !flashUpdate) {
			return;
		}

		Attribute attrib = DefaultAttribute;
		for(int y = 0; y < HEIGHT; ++y) {
			for(int x = 0; x < WIDTH; ++x) {
				Attribute const a = attributeBuffer[x + (y * WIDTH)];

				bool const charNeedsUpdate = dirty || a.flash || attrib.flash;
				if(charNeedsUpdate) {
					if(a.back != attrib.back ||
					   a.pen != attrib.pen ||
					   a.bright != attrib.bright ||
					   a.flash != attrib.flash) {
						if(a.flash && this->flashState) {
							drawer->setPenColour(a.back);
							drawer->setBackgroundColour((a.bright * 8) + a.pen);
						} else {
							drawer->setPenColour((a.bright * 8) + a.pen);
							drawer->setBackgroundColour(a.back);
						}
						attrib = a;
					}
					char const c = textBuffer[x + (y * WIDTH)];
					drawer->PutChar( offsetX + x, offsetY + y, c );
				}
			}
		}
		dirty = false;
	}

	void NewLine() {
		if (curRow == HEIGHT - 1) {
			memcpy(&textBuffer[0], &textBuffer[1 * WIDTH], WIDTH * (HEIGHT - 1));
			memset(&textBuffer[WIDTH * (HEIGHT - 1)], 0, WIDTH);
			memcpy(&attributeBuffer[0], &attributeBuffer[1 * WIDTH], WIDTH * (HEIGHT - 1));
			memset(&attributeBuffer[WIDTH * (HEIGHT - 1)], *(uint8_t *) &DefaultAttribute, WIDTH);
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
					if(curCol >= WIDTH) {
						NewLine();
					}
					break;
			}
			i++;
		}
		dirty = true;
	}

	void PrintWithSizeLn(uint32_t size, char const *str)  NON_NULL(3){
		PrintWithSize(size, str);
		NewLine();
	}
	void Print(char const *str)  NON_NULL(2) {
		PrintWithSize(Utils::StringLength(str), str);
	}

	void PrintLn(char const *str)  NON_NULL(2) {
		PrintWithSizeLn(Utils::StringLength(str), str);
	}
	void Printf(const char *format, ...) NON_NULL(2) __attribute__((format(printf, 2, 3))) {
		char buffer[256]; // 256 byte max string (on stack)
		va_list va;
		va_start(va, format);
		int len = vsnprintf(buffer, 256, format, va);
		va_end(va);
		buffer[255] = 0;
		PrintWithSize(len, buffer);
	}
	void PutChar(char c) {
		PrintWithSize(1, &c);
	}

	uint8_t ThirtyHzCounter;
	uint8_t textArray[HEIGHT * WIDTH];
	Attribute attributeArray[HEIGHT * WIDTH];

};

}