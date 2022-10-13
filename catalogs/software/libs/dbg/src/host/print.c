
#include <stdarg.h>
#include <stdio.h>

#define VPRINTF_STACK_SIZE 32 * 1024

void raw_debug_printf(const char *format, ...) {
	char buffer[VPRINTF_STACK_SIZE]; // VPRINTF_STACK_SIZE byte max string (on stack)
	va_list va;
	va_start(va, format);
	int len = vsnprintf(buffer, VPRINTF_STACK_SIZE, format, va);
	va_end(va);
	buffer[VPRINTF_STACK_SIZE-1] = 0;
	printf("%s", buffer);
}


void debug_printf(const char *format, ...) {
	char buffer[VPRINTF_STACK_SIZE]; // VPRINTF_STACK_SIZE byte max string (on stack)
	va_list va;
	va_start(va, format);
	int len = vsnprintf(buffer, VPRINTF_STACK_SIZE, format, va);
	va_end(va);
	buffer[VPRINTF_STACK_SIZE-1] = 0;
	printf("%s", buffer);
}
void debug_print(const char *txt) {
	printf("%s", txt);
}