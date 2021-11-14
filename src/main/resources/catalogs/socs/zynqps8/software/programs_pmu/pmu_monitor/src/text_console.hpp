#pragma once

#include "gfxdebug/console.hpp"
struct TextConsole {
	void Init() {
		console.Init();
		framebuffer = nullptr;
		frameBufferWidth = 0;
		frameBufferHeight = 0;
	}
	GfxDebug::Console<1280/16, 720/16> console;

	uint8_t * framebuffer;
	uint16_t frameBufferWidth;
	uint16_t frameBufferHeight;
};
