#include "core/core.h"
#include "gfxdebug/console.hpp"

static uint32_t GetNumber(uint32_t size, char const *str, int* outIndex = nullptr, char const delim = ';') {
	uint32_t result = 0;
	int i = 0;
	while (*(str + i) != delim) {
		char c = *(str + i);
		int32_t n = c - '0';
		if (n < 0 || n > 9) {
			if(outIndex) *outIndex = i;
			return result;
		}
		result = (result * 10) + n;
		i++;
	}
	if(outIndex) *outIndex = i;
	return result;
}

namespace GfxDebug {
void ConsoleBase::Init(uint16_t w, uint16_t h, uint8_t* tb, Attribute* ab) {
	dirty = true;
	flashState = false;
	width = w;
	height = h;
	curCol = 0;
	curRow = 0;
	currentAttribute = DefaultAttribute;
	textBuffer = tb;
	attributeBuffer = ab;
#if GFXDEBUG_FONTS_MINIMAL_MEMORY == 2
	// 0x7f = GFXDEBUG_FONTS_MAX_CHAR
	memset(textBuffer, 0x7f, width * height);
#else
	memset(textBuffer, 0, width * height);
#endif
	memset(attributeBuffer, *(uint8_t *) &DefaultAttribute, width * height);
}

uint32_t ConsoleBase::ProcessANSI(uint32_t size, char const *str, uint32_t i) {
	enum class ANSIState {
		AWAIT_BRACKET,
		PARAMETERS,
		CURSOR_UP,
		CURSOR_DOWN,
		CURSOR_FORWARD,
		CURSOR_BACK,
		CURSOR_POSITION,
		ERASE_IN_DISPLAY,
		ERASE_IN_LINE,

		SELECT_GRAPHIC_RENDITION,
	} ansiState = ANSIState::AWAIT_BRACKET;
	char parameters[16]{};
	int currentParameter = 0;

	while (true) {
		switch (ansiState) {
			case ANSIState::AWAIT_BRACKET: {
				char const c = str[i++];
				if (c != '[') {
					return i;
				} else {
					ansiState = ANSIState::PARAMETERS;
				}
				break;
			}
			case ANSIState::PARAMETERS: {
				char const c = str[i++];
				// collect parameters until end;
				switch (c) {
					case 'A': ansiState = ANSIState::CURSOR_UP;
						break;
					case 'B': ansiState = ANSIState::CURSOR_DOWN;
						break;
					case 'C': ansiState = ANSIState::CURSOR_FORWARD;
						break;
					case 'D': ansiState = ANSIState::CURSOR_BACK;
						break;
					case 'H': ansiState = ANSIState::CURSOR_POSITION;
						break;
					case 'J': ansiState = ANSIState::ERASE_IN_DISPLAY;
						break;
					case 'm': ansiState = ANSIState::SELECT_GRAPHIC_RENDITION;
						break;
					case ' ': break; // skip spaces
					default: parameters[currentParameter++] = c;
						break;
				}
				break;
			}
			case ANSIState::CURSOR_UP: {
				uint32_t n = GetNumber(16, &parameters[0]);
				if (n == 0)
					n = 1;
				int r = ((int) curRow) - n;
				if (r < 0) {
					curRow = 0;
				} else {
					curRow = (uint16_t) r;
				}
				return i;
			}
			case ANSIState::CURSOR_DOWN: {
				uint32_t n = GetNumber(16, &parameters[0]);
				if (n == 0)
					n = 1;
				int r = ((int) curRow) + n;
				if (r > height - 1) {
					curRow = height - 1;
				} else {
					curRow = (uint16_t) r;
				}
				return i;
			}
			case ANSIState::CURSOR_BACK: {
				uint32_t n = GetNumber(16, &parameters[0]);
				if (n == 0)
					n = 1;
				int r = ((int) curCol) - n;
				if (r < 0) {
					curCol = 0;
				} else {
					curCol = (uint16_t) r;
				}
				return i;
			}
			case ANSIState::CURSOR_FORWARD: {
				uint32_t n = GetNumber(16, &parameters[0]);
				if (n == 0)
					n = 1;
				int r = ((int) curCol) + n;
				if (r > width - 1) {
					curCol = width - 1;
				} else {
					curCol = (uint16_t) r;
				}
				return i;
			}
			case ANSIState::CURSOR_POSITION: {
				int secondIndex = 0;
				uint32_t n = GetNumber(16, &parameters[0], &secondIndex);
				uint32_t m = GetNumber(16 - secondIndex, &parameters[secondIndex]);
				if (n > 0)
					n = n - 1;
				if (m > 0)
					m = m - 1;
				if (n > height - 1U)
					n = height - 1U;
				if (m > width - 1U)
					m = width - 1U;
				curRow = n;
				curCol = m;
				return i;
			}
			case ANSIState::ERASE_IN_DISPLAY: {
				uint32_t n = GetNumber(16, &parameters[0]);
				switch (n) {
					case 0: {
						// clear from cursor to end of screen
						auto const start = (curRow * width) + curCol;
						auto const end = width * height;
						memset(textBuffer + start, 0, end - start);
						memset(attributeBuffer + start, *(uint8_t *) &currentAttribute, end - start);
						break;
					}
					case 1: { // clear from cursor to beginning of screen
						auto const start = 0;
						auto const end = (curRow * width) + curCol;
						memset(textBuffer + start, 0, end - start);
						memset(attributeBuffer + start, *(uint8_t *) &currentAttribute, end - start);
						break;
					}
					case 3:
						// clear whole screen + clear scroll back buffer
					case 2: {
						// clear whole screen;
						auto const start = 0;
						auto const end = width * height;
						memset(textBuffer + start, 0, end - start);
						memset(attributeBuffer + start, *(uint8_t *) &currentAttribute, end - start);
						break;
					}
				}
				return i;
			}
			case ANSIState::SELECT_GRAPHIC_RENDITION: {
				uint32_t n = GetNumber(16, &parameters[0]);
				switch (n) {
					case 0: currentAttribute = DefaultAttribute;
						break;
					case 1: currentAttribute.bright = 1;
						break;
					case 5:
					case 6: currentAttribute.flash = 1;
						break;
					case 22: currentAttribute.bright = 0;
						break;
					case 25: currentAttribute.flash = 0;
						break;
					case 30:
					case 31:
					case 32:
					case 33:
					case 34:
					case 35:
					case 36:
					case 37: currentAttribute.pen = n - 30;
						break;
					case 40:
					case 41:
					case 42:
					case 43:
					case 44:
					case 45:
					case 46:
					case 47: currentAttribute.back = n - 40;
						break;
					default: break;
				}
				return i;
			}
			default: return i;
		}
	}
}

}