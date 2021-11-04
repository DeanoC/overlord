
#include <dbg/raw_print.h>
#include "core/core.h"
#include "osservices/osservices.h"
#include "osservices/ipi3_transport.h"
#include "core/snprintf.h"
#include "utils/string_utils.h"
#include "hw/cache.h"

// prints upto 30 characters via PMU.
// gives you 28 text chars with a newline at the end
void OsService_InlinePrint(uint8_t size, const char *const text) {
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

WEAK_LINKAGE void OsService_Print(const char *const text) {
	OsService_PrintWithSize(Utils_StringLength(text), text);
}

WEAK_LINKAGE void OsService_PrintWithSize(unsigned int count, const char *const text) {
	if(count <= 29) {
		OsService_InlinePrint(count, text);
	} else {
		char *buffer = ALLOCA(count+32); // string to send to OS (in DDR stack)
		IPI3_Msg* msg = (IPI3_Msg*) buffer;
		msg->function = OSF_PTR_PRINT;
		msg->ddrPtrFlag = true;
		msg->Payload.DdrPacket.packetDdrAddress = (uintptr_all_t)(uintptr_t)buffer;
		msg->Payload.DdrPacket.packetSize = count + 32;
		memcpy(buffer+32, text, count);
		Cache_DCacheCleanRange((uintptr_t) buffer, count+32);

		// using stack memory directly so must be valid until response is fetched
		IPI3_Response response;
		IPI3_OnService_SubmitAndFetchResponse(msg, &response);
		if(response.result == IRR_SUCCESS) {
		}
	}
}

void OsService_Printf(const char *format, ...) {
	char buffer[256]; // 256 byte max string (on stack)
	va_list va;
	va_start(va, format);
	vsnprintf(buffer, 256, format, va);
	va_end(va);
	buffer[255] = 0;

	OsService_Print(buffer);
}

