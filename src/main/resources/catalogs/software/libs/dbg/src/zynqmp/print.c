#include "core/core.h"
#include "core/snprintf.h"
#include "platform/reg_access.h"
#include "platform/memory_map.h"
#include "platform/registers/uart.h"
#include "osservices/osservices.h"

// TODO fix this

#define UART_DEBUG_BASE_ADDR UART1_BASE_ADDR
#define UART_DEBUG_REGISTER(reg) UART_##reg##_OFFSET
#define UART_DEBUG_FIELD(reg, field) UART_##reg##_##field
#define UART_DEBUG_FIELD_MASK(reg, field) UART_##reg##_##field##_MASK
#define UART_DEBUG_FIELD_LSHIFT(reg, field) UART_##reg##_##field##_LSHIFT
#define UART_DEBUG_FIELD_ENUM(reg, field, enm) UART_##reg##_##field##_##enm

#define IsTransmitFull() (HW_REG_GET_BIT(UART_DEBUG, CHANNEL_STS, TNFUL))

#define VPRINTF_STACK_SIZE 1024
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
			HW_REG_SET(UART_DEBUG, TX_RX_FIFO, '\r');
		}

		// stall
		while (IsTransmitFull()){}
		HW_REG_SET(UART_DEBUG, TX_RX_FIFO, *cur);
		cur++;
	}
}
void raw_debug_sized_print(uint32_t size, char const * text) {
	char const *cur = text;
	for(int i=0;i < size;++i)
	{
		if(*cur == '\n') {
			while (IsTransmitFull()){}
			HW_REG_SET(UART_DEBUG, TX_RX_FIFO, '\r');
		}

		// stall
		while (IsTransmitFull()){}
		HW_REG_SET(UART_DEBUG, TX_RX_FIFO, *cur);
		cur++;
	}
}

void raw_debug_printf(const char *format, ...) {
	char buffer[VPRINTF_STACK_SIZE]; // max string (on stack)
	va_list va;
	va_start(va, format);
	int len = vsnprintf(buffer, VPRINTF_STACK_SIZE, format, va);
	va_end(va);
	buffer[VPRINTF_STACK_SIZE-1] = 0;
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

	char buffer[VPRINTF_STACK_SIZE]; // max string (on stack)
	va_list va;
	va_start(va, format);
	vsnprintf(buffer, VPRINTF_STACK_SIZE, format, va);
	va_end(va);
	buffer[VPRINTF_STACK_SIZE-1] = 0;
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

