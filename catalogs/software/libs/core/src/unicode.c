// these unicode conversion functions come from SDL
// changes to types and names to match IKUY style
// Any bugs probably introduced by me
/*
  SDL_ttf:  A companion library to SDL for working with TrueType (tm) fonts
  Copyright (C) 2001-2022 Sam Lantinga <slouken@libsdl.org>
  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.
  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:
  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/
#include "core/core.h"
#include "core/utf8.h"
#include "core/unicode.h"
#include "dbg/print.h"

size_t Core_UCS2toUTF8NullLen(uint16_t const * text)
{
	size_t bytes = 1;
	while (*text) {
		uint16_t ch = *text++;
		if (ch <= 0x7F) {
			bytes += 1;
		} else if (ch <= 0x7FF) {
			bytes += 2;
		} else {
			bytes += 3;
		}
	}
	return bytes;
}
size_t Core_UCS2toUTF8Len(uint16_t const * text, int len)
{
	size_t bytes = 1;
	while (len > 0) {
		uint16_t ch = *text++;
		if (ch <= 0x7F) {
			bytes += 1;
		} else if (ch <= 0x7FF) {
			bytes += 2;
		} else {
			bytes += 3;
		}
		len--;
	}
	return bytes;
}
/* Convert a UCS-2 string to a UTF-8 string */
void Core_UCS2toUTF8(const uint16_t *src, int len, utf8_int8_t *dst)
{
	while (len > 0) {
		uint16_t ch = *src++;
		if (ch <= 0x7F) {
			*dst++ = (utf8_int8_t) ch;
		} else if (ch <= 0x7FF) {
			*dst++ = 0xC0 | (utf8_int8_t) ((ch >> 6) & 0x1F);
			*dst++ = 0x80 | (utf8_int8_t) (ch & 0x3F);
		} else {
			*dst++ = 0xE0 | (utf8_int8_t) ((ch >> 12) & 0x0F);
			*dst++ = 0x80 | (utf8_int8_t) ((ch >> 6) & 0x3F);
			*dst++ = 0x80 | (utf8_int8_t) (ch & 0x3F);
		}
		len--;
	}
	*dst = '\0';
}