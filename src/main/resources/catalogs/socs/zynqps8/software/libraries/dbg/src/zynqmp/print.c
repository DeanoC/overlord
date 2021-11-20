#include "core/core.h"
#include "core/snprintf.h"
#include "hw/reg_access.h"
#include "hw/memory_map.h"
#include "hw_regs/uart.h"
#include "osservices/osservices.h"

#define IsTransmitFull() (HW_REG_GET_BIT(UART0, CHANNEL_STS, TNFUL))

static bool force_raw_print;

void debug_force_raw_print(bool enabled) {
	force_raw_print = enabled;
}
void raw_debug_print(char const *const text) {
	char const *cur = text;
	while (*cur != 0)
	{
		if(*cur == '\n') {
			while (IsTransmitFull()){}
			HW_REG_SET(UART0, TX_RX_FIFO, '\r');
		}

		// stall
		while (IsTransmitFull()){}
		HW_REG_SET(UART0, TX_RX_FIFO, *cur);
		cur++;
	}
}
void raw_debug_sized_print(uint32_t size, char const * text) {
	char const *cur = text;
	for(int i=0;i < size;++i)
	{
		if(*cur == '\n') {
			while (IsTransmitFull()){}
			HW_REG_SET(UART0, TX_RX_FIFO, '\r');
		}

		// stall
		while (IsTransmitFull()){}
		HW_REG_SET(UART0, TX_RX_FIFO, *cur);
		cur++;
	}
}

void raw_debug_printf(const char *format, ...) {
	char buffer[256]; // 256 byte max string (on stack)
	va_list va;
	va_start(va, format);
	int len = vsnprintf(buffer, 256, format, va);
	va_end(va);
	buffer[255] = 0;
	raw_debug_sized_print(len, buffer);
}

void debug_print(char const *const text){
	if(force_raw_print) {
		raw_debug_print(text);
	} else {
		OsService_Print(text);
	}
}

void debug_printf(const char *format, ...) {
	char buffer[256]; // 256 byte max string (on stack)
	va_list va;
	va_start(va, format);
	vsnprintf(buffer, 256, format, va);
	va_end(va);
	buffer[255] = 0;
	if (force_raw_print) {
		raw_debug_print(buffer);
	} else {
		OsService_Print(buffer);
	}
}

void debug_sized_print(uint32_t size, char const * text) {
	if (force_raw_print) {
		raw_debug_sized_print(size, text);
	} else {
		OsService_PrintWithSize(size, text);
	}
}

