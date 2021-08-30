#include "core/core.h"
#include "core/snprintf.h"
#include "hw/reg_access.h"
#include "hw/memory_map.h"
#include "hw_regs/uart.h"
#include "osservices/osservices.h"

#define IsTransmitFull() (HW_REG_GET_BIT(UART0, CHANNEL_STS, TNFUL))

void raw_debug_print(char const *const text) {
	char const *cur = text;
	while (*cur != 0)
	{
		while (IsTransmitFull())
		{
			// stall
		}

		HW_REG_SET(UART0, TX_RX_FIFO, *cur);
		cur++;
	}
}

void raw_debug_printf(const char *format, ...) {
	char buffer[256]; // 256 byte max string (on stack)
	va_list va;
	va_start(va, format);
	vsnprintf(buffer, 256, format, va);
	va_end(va);
	buffer[255] = 0;
	raw_debug_print(buffer);
}

void debug_print(char const *const text) {
	OsService_Print(text);
}

void debug_printf(const char *format, ...) {
	char buffer[256]; // 256 byte max string (on stack)
	va_list va;
	va_start(va, format);
	vsnprintf(buffer, 256, format, va);
	va_end(va);
	buffer[255] = 0;
	OsService_Print(buffer);
}