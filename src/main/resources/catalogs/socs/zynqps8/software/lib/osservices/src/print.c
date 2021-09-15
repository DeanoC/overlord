#include "core/core.h"
#include "osservices/osservices.h"
#include "osservices/ipi3_transport.h"
#include "dbg/assert.h"
#include "dbg/raw_print.h"
#include "core/snprintf.h"

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
#if CPU_pmu == 1 || CPU_r5f == 1
		// stack isn't in DDR so we instead
		// break into chunks and send them by N inline prints
		// The weak linkage allows it to be overridden with something more custom
		unsigned int inlineCount = 0;
		const char *ptr = text;
		const char *start = ptr;
		while(*ptr != 0 ) {
			if(inlineCount == 29) {
				OsService_InlinePrint(29, start);
				inlineCount = 0;
				start = ptr;
			}
			inlineCount++;
			ptr++;
		}
		if(inlineCount > 0) {
			OsService_InlinePrint(inlineCount, start);
		}
#else
		const char * ptr = text;
		const uint32_t BUFFER_SIZE = 1024;
		while(*ptr != 0) { ptr++; }
		unsigned int count = ptr - text;

		assert(count < BUFFER_SIZE);
		if(count <= 29) {
			OsService_InlinePrint(count, text);
		} else {

		char buffer[BUFFER_SIZE]; // BUFFER_SIZE byte max string (on stack)

		count = 32;
		ptr = text;
		while(*ptr != 0 && count < BUFFER_SIZE-1) {
			buffer[count] = *ptr;
			count++;
			ptr++;
		}

		IPI3_Msg* msg = (IPI3_Msg*) buffer;
		msg->function = OSF_PTR_PRINT;
		msg->ddrPtrFlag = true;
		msg->Payload.DdrPacket.packetDdrAddress = (uintptr_all_t)(uintptr_t)buffer;
		msg->Payload.DdrPacket.packetSize = count;

		// using stack memory directly so must be valid until response is fetched
		IPI3_OsService_Submit(msg);

		IPI3_Response response;
		IPI3_OnService_FetchResponse(&response);
		if(response.result == IRR_SUCCESS) {
		}
	}
#endif
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

