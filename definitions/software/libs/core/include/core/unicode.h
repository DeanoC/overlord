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
#pragma once

#include "core/core.h"
#include "core/utf8.h"

// length of a null terminated UCS2 string as a UTF8 string
EXTERN_C size_t Core_UCS2toUTF8NullLen(uint16_t const * text);

// length of a UCS2 string of len as a UTF8String
EXTERN_C size_t Core_UCS2toUTF8Len(uint16_t const * text, int len);

// Convert a UCS-2 string to a UTF-8 string
EXTERN_C void Core_UCS2toUTF8(const uint16_t *src, int len, utf8_int8_t *dst);
