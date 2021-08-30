#include "core/core.h"
#include "osservices/ipi3_transport.h"
#include "dbg/raw_print.h"
#include "core/snprintf.h"

// prints upto 30 characters via PMU.
// gives you 28 text chars with a newline at the end
void OsService_InlinePrint(uint8_t size, const char *const text) {
	if(size > 30) {
		raw_debug_printf("OSS InlinePrint size %d > 30 chars\n", size);
		size = 30;
	}
	// TODO we copy into the IPI3_Msg and then copy that to the HW
	// buffer. could write directly to HW buffer.
	IPI3_Msg msg = {
			.function = OSF_INLINE_PRINT,
			.OSF_InlinePrint.size = size,
	};

	memcpy(msg.OSF_InlinePrint.text, text, size);
	IPI3_OsService_Submit(&msg);
}

void OsService_Print(const char *const text) {
	// break into 30 character chunks and send them by N inline prints
	unsigned int count = 0;
	const char * ptr = text;

	IPI3_Msg msg = {
			.function = OSF_INLINE_PRINT,
			.OSF_InlinePrint.size = 30,
			};

	while(*ptr != 0 ) {
		if(count == 30) {
			memcpy(msg.OSF_InlinePrint.text, ptr-30, 30);
			IPI3_OsService_Submit(&msg);
			count = 0;
		}
		count++;
		ptr++;
	}

	if(count > 0) {
		msg.OSF_InlinePrint.size = count;
		memcpy(msg.OSF_InlinePrint.text, ptr-count, count);
		IPI3_OsService_Submit(&msg);
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

