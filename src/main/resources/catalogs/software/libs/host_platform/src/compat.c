//
// Created by deano on 3/4/22.
//
#include <stdarg.h>
#include <stdio.h>

void raw_debug_printf(const char *format, ...) {
	char buffer[256]; // 256 byte max string (on stack)
	va_list va;
	va_start(va, format);
	int len = vsnprintf(buffer, 256, format, va);
	va_end(va);
	buffer[255] = 0;
	printf("%s", buffer);
}


