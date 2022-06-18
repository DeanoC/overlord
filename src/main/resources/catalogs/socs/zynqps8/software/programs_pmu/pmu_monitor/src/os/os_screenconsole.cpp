#include "core/core.h"
#include "core/snprintf.h"
#include "platform/memory_map.h"
#include "platform/reg_access.h"
#include "platform/registers/uart.h"
#include "osservices/ipi3_transport.h"
#include "ipi3_os_server.hpp"
#include "../os_heap.hpp"
#include "osservices/osservices.h"

namespace IPI3_OsServer {

void ScreenConsoleInlinePrint(IPI3_Msg const *msgBuffer) {
	uint8_t size = msgBuffer->Payload.InlinePrint.size;
	const auto *text = (const char *) msgBuffer->Payload.InlinePrint.text;
	osHeap->console.console.PrintWithSize(size, text);
}


void ScreenConsoleEnable(IPI3_Msg const *msgBuffer) {
	osHeap->screenConsoleEnabled  = msgBuffer->Payload.ScreenConsoleEnable.enabled;
}

void ScreenConsolePtrPrint(IPI_Channel senderChannel, IPI3_Msg const *msgBuffer) {
	uint32_t size = msgBuffer->Payload.DdrPacket.packetSize - IPI3_HEADER_SIZE - sizeof(IPI3_DdrPacket);
	const auto *text = (const char *) (msgBuffer->Payload.PtrPrint.text);
	osHeap->console.console.PrintWithSize(size, text);
}


} // end namespace