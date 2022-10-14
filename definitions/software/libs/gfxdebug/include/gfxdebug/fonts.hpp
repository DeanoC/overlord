#pragma once

// 0 = 8x8 and 8x16 font used all 256 chars
// 1 = 8x8 font all 256 chars
// 2 = 8x8 font 32-128 chars only
#ifndef GFXDEBUG_FONTS_MINIMAL_MEMORY
#define GFXDEBUG_FONTS_MINIMAL_MEMORY 0
#endif

extern uint8_t const * font8x8;

#if GFXDEBUG_FONTS_MINIMAL_MEMORY == 2
#define GFXDEBUG_FONTS_MIN_CHAR 33
#define GFXDEBUG_FONTS_MAX_CHAR 127
#elif GFXDEBUG_FONTS_MINIMAL_MEMORY == 1
#define GFXDEBUG_FONTS_MIN_CHAR 0
#define GFXDEBUG_FONTS_MAX_CHAR 255
#elif GFXDEBUG_FONTS_MINIMAL_MEMORY == 0
#define GFXDEBUG_FONTS_MIN_CHAR 0
#define GFXDEBUG_FONTS_MAX_CHAR 255
extern uint8_t const * font8x16;
#else
#error GFXDEBUG_FONTS_MINIMAL_MEMORY must be 0 to 2
#endif