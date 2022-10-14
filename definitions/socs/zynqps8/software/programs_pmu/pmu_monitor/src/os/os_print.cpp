#include "core/core.h"
#include "core/snprintf.h"
#include "platform/memory_map.h"
#include "platform/reg_access.h"
#include "platform/registers/uart.h"
#include "osservices/ipi3_transport.h"
#include "ipi3_os_server.hpp"
#include "../os_heap.hpp"
#include "osservices/osservices.h"
#include "utils/string_utils.hpp"

extern uint32_t uartDebugTransmitLast;
extern uint32_t uartDebugTransmitHead;

#define UART_DEBUG_BASE_ADDR UART1_BASE_ADDR

namespace IPI3_OsServer {

void PutSizedData(uint32_t size, const uint8_t *text) {
	if (size == 0)
		return;

	//put text into to transmit buffer, interrupts will send it to host

	// split at buffer end
	if (uartDebugTransmitHead + size >= OsHeap::UartBufferSize) {
		const uint32_t firstBlockSize = OsHeap::UartBufferSize - uartDebugTransmitHead;
		if (firstBlockSize > 0) {
			memcpy(&osHeap->uartDEBUGTransmitBuffer[uartDebugTransmitHead], text, firstBlockSize);
			text += firstBlockSize;
			size -= firstBlockSize;
		}
		uartDebugTransmitHead = 0;
	}

	if (size > 0) {
		memcpy(&osHeap->uartDEBUGTransmitBuffer[uartDebugTransmitHead], text, size);
		uartDebugTransmitHead += (uint32_t) size;
	}

	HW_REG_WRITE(HW_REG_GET_ADDRESS(UART_DEBUG), UART, INTRPT_EN, UART_INTRPT_EN_TEMPTY);
}

#define IsTransmitFull() (HW_REG_GET_BIT(UART_DEBUG, CHANNEL_STS, TNFUL))

void DebugInlinePrint(IPI3_Msg const *msgBuffer) {
	uint8_t size = msgBuffer->Payload.InlinePrint.size;
	const auto *text = (const uint8_t *) msgBuffer->Payload.InlinePrint.text;
	PutSizedData(size, text);
}

void DebugPtrPrint(IPI_Channel senderChannel, IPI3_Msg const *msgBuffer) {
	uint32_t size = msgBuffer->Payload.DdrPacket.packetSize - IPI3_HEADER_SIZE - sizeof(IPI3_DdrPacket);
	const auto *text = (const uint8_t *) (msgBuffer->Payload.PtrPrint.text);
	PutSizedData(size, text);
}

} // end namespace



// override the weak prints, to write directly into the buffer
EXTERN_C WEAK_LINKAGE void OsService_InlinePrint(uint8_t size, const char *const text) {
	IPI3_OsServer::PutSizedData(size, (const uint8_t *)text);
}

EXTERN_C WEAK_LINKAGE void OsService_Print(const char *const text) {
	OsService_PrintWithSize(Utils_StringLength(text), text);
}

EXTERN_C WEAK_LINKAGE void OsService_PrintWithSize(unsigned int count, const char *const text) {
	IPI3_OsServer::PutSizedData(count, (const uint8_t *) text);
}

EXTERN_C WEAK_LINKAGE void OsService_Printf(const char *format, ...) {
	char buffer[256]; // 256 byte max string (on stack)
	va_list va;
	va_start(va, format);
	int const len = vsnprintf(buffer, 256, format, va);
	va_end(va);
	buffer[255] = 0;

	OsService_PrintWithSize(len,buffer);
}

EXTERN_C WEAK_LINKAGE void debug_print(char const * text){
	OsService_Print(text);
}
EXTERN_C WEAK_LINKAGE void debug_printf(const char *format, ...) {
	char buffer[256]; // 256 byte max string (on stack)
	va_list va;
	va_start(va, format);
	int const len = vsnprintf(buffer, 256, format, va);
	va_end(va);
	buffer[255] = 0;

	OsService_PrintWithSize(len,buffer);
}
EXTERN_C WEAK_LINKAGE void debug_sized_print(uint32_t size, char const * text) {
	OsService_PrintWithSize(size, text);
}
