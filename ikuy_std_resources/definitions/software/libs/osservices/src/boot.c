#include "core/core.h"
#include "osservices/osservices.h"
#include "osservices/ipi3_transport.h"
// TODO we copy into the IPI3_Msg and then copy that to the HW
// buffer. could write directly to HW buffer.
void OsService_BootComplete(BootData const* bootData) {
	IPI3_Msg msg = {
			.ddrPtrFlag = false,
			.function = OSF_BOOT_COMPLETE,
	};
	memcpy(&msg.Payload.BootData.bootData, bootData, sizeof(BootData));
	IPI3_OsService_Submit(&msg);
}

void OsService_FetchBootData(BootData* bootData) {
	IPI3_Msg ALIGN(64) msg = {
			.ddrPtrFlag = false,
			.function = OSF_FETCH_BOOT_DATA,
	};
	memcpy(&msg.Payload.BootData.bootData, bootData, sizeof(BootData));
	IPI3_Response ALIGN(64) response;
	IPI3_OnService_SubmitAndFetchResponse(&msg, &response);
	if(response.result == IRR_SUCCESS) {
		memcpy(bootData, &response.BootData, sizeof(BootData));
	} else {
		memset(bootData, 0, sizeof(BootData));
		OsService_InlinePrint(OSS_INLINE_TEXT("Fetch Boot Data failed\n"));
	}
}
