#include "core/core.h"
#include "osservices/osservices.h"
#include "osservices/ipi3_transport.h"
#include "core/snprintf.h"
#include "utils/string_utils.h"
#include "hw/cache.h"
#include "dbg/print.h"
#include "dbg/raw_print.h"

// prints upto 29 characters to uart via PMU.
WEAK_LINKAGE void OsService_InlinePrint(uint8_t size, const char *const text) {
	if(size > 29) {
		OsService_Printf("OSS InlinePrint size %d > 29 chars\n", size);
		size = 29;
	}
	// TODO we copy into the IPI3_Msg and then copy that to the HW
	// buffer. could write directly to HW buffer.
	IPI3_Msg msg = {
			.ddrPtrFlag = false,
			.function = OSF_INLINE_PRINT,
			.Payload.InlinePrint.size = size,
	};

	memcpy(msg.Payload.InlinePrint.text, text, size);
	IPI3_OsService_Submit(&msg);
}

WEAK_LINKAGE void OsService_PrintWithSize(unsigned int count, const char *const text) {
	if(count <= 29) {
		OsService_InlinePrint(count, text);
	} else {
		uint32_t const totalSize = count + IPI3_HEADER_SIZE + sizeof(IPI3_DdrPacket);
		char *buffer = ALLOCA(totalSize); // string to send to OS (in DDR stack)
		IPI3_Msg* msg = (IPI3_Msg*) buffer;
		msg->function = OSF_SCREEN_CONSOLE_PTR_PRINT;
		msg->ddrPtrFlag = true;
		msg->Payload.DdrPacket.packetDdrAddress = (uintptr_all_t)(uintptr_t)buffer;
		msg->Payload.DdrPacket.packetSize = totalSize;
		memcpy(msg->Payload.PtrPrint.text, text, count);
		Cache_DCacheCleanRange((uintptr_t) buffer, totalSize);

		// using stack memory directly so must be valid until response is fetched
		IPI3_Response response;
		IPI3_OnService_SubmitAndFetchResponse(msg, &response);
	}
}

WEAK_LINKAGE void OsService_Print(const char *const text) {
	OsService_PrintWithSize(Utils_StringLength(text), text);
}

WEAK_LINKAGE void OsService_Printf(const char *format, ...) {
	char buffer[256]; // 256 byte max string (on stack)
	va_list va;
	va_start(va, format);
	int const len = vsnprintf(buffer, 256, format, va);
	va_end(va);
	buffer[255] = 0;

	OsService_PrintWithSize(len,buffer);
}

// prints upto 29 characters to uart via PMU.
WEAK_LINKAGE void OsService_ScreenConsoleInlinePrint(uint8_t size, const char *const text) {
	if(size > 29) {
		OsService_Printf("OSS ScreenConsoleInlinePrint size %d > 29 chars\n", size);
		size = 29;
	}
	// TODO we copy into the IPI3_Msg and then copy that to the HW
	// buffer. could write directly to HW buffer.
	IPI3_Msg msg = {
			.ddrPtrFlag = false,
			.function = OSF_SCREEN_CONSOLE_INLINE_PRINT,
			.Payload.InlinePrint.size = size,
	};

	memcpy(msg.Payload.InlinePrint.text, text, size);
	IPI3_OsService_Submit(&msg);
}

WEAK_LINKAGE void OsService_ScreenConsolePrintWithSize(unsigned int count, const char *const text) {
	if(count <= 29) {
		OsService_ScreenConsoleInlinePrint(count, text);
	} else
	{
		uint32_t const totalSize = count + IPI3_HEADER_SIZE + sizeof(IPI3_DdrPacket);
		char *buffer = ALLOCA(totalSize); // string to send to OS (in DDR stack)
		IPI3_Msg* msg = (IPI3_Msg*) buffer;
		msg->function = OSF_SCREEN_CONSOLE_PTR_PRINT;
		msg->ddrPtrFlag = true;
		msg->Payload.DdrPacket.packetDdrAddress = (uintptr_all_t)(uintptr_t)buffer;
		msg->Payload.DdrPacket.packetSize = totalSize;
		memcpy(msg->Payload.PtrPrint.text, text, count);
		Cache_DCacheCleanRange((uintptr_t) buffer, totalSize);

		// using stack memory directly so must be valid until response is fetched
		IPI3_Response response;
		IPI3_OnService_SubmitAndFetchResponse(msg, &response);
	}
}

WEAK_LINKAGE void OsService_ScreenConsolePrint(const char *const text) {
	OsService_ScreenConsolePrintWithSize(Utils_StringLength(text), text);
}

WEAK_LINKAGE void OsService_ScreenConsolePrintf(const char *format, ...) {
	char buffer[256]; // 256 byte max string (on stack)
	va_list va;
	va_start(va, format);
	int const len = vsnprintf(buffer, 256, format, va);
	va_end(va);
	buffer[255] = 0;

	OsService_ScreenConsolePrintWithSize(len, buffer);
}

void OsServer_EnableScreenConsole(uintptr_lo_t framebuffer, uint16_t fbWidth, uint16_t fbHeight) {
	// TODO we copy into the IPI3_Msg and then copy that to the HW
	// buffer. could write directly to HW buffer.
	IPI3_Msg msg = {
			.ddrPtrFlag = false,
			.function = OSF_SCREEN_CONSOLE_CONFIG,
			.Payload.ScreenConsoleConfig.address = framebuffer,
			.Payload.ScreenConsoleConfig.width = fbWidth,
			.Payload.ScreenConsoleConfig.height = fbHeight
	};
	IPI3_OsService_Submit(&msg);
}

void OsServer_DisableScreenConsole() {
	// TODO we copy into the IPI3_Msg and then copy that to the HW
	// buffer. could write directly to HW buffer.
	IPI3_Msg msg = {
			.ddrPtrFlag = false,
			.function = OSF_SCREEN_CONSOLE_CONFIG,
			.Payload.ScreenConsoleConfig.address = 0,
	};
	IPI3_OsService_Submit(&msg);
}

