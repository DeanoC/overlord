#include "core/core.h"
#include "osservices/osservices.h"
#include "osservices/ipi3_transport.h"
#include "hw/memory_map.h"

void* OsService_DdrLoBlockAlloc(uint16_t blocks1MB) {
	IPI3_Msg msg = {
			.function = OSF_DDR_LO_BLOCK_ALLOC,
			.OSF_DdrLoBlockAlloc.blocks1MB = blocks1MB,
			};
	IPI3_OsService_Submit(&msg);

	IPI3_Response response;
	IPI3_OnService_FetchResponse(&response);
	if(response.result == IRR_SUCCESS) {
		return (void *) (uintptr_t) (DDR_DDR4_0_BASE_ADDR + response.OSF_DDRloBlockAlloc.block_1MB_Offset * (1024 * 1024));
	} else {
		OsService_InlinePrint(OSS_INLINE_TEXT("DdrLoBlocKAlloc failed"));
		switch(response.result) {
			case IRR_OUT_OF_MEMORY:
				OsService_InlinePrint(OSS_INLINE_TEXT(" : IRR_OUT_OF_MEMORY\n"));
				break;
			default:
				OsService_InlinePrint(OSS_INLINE_TEXT("\n"));
				break;
		}
		return nullptr;
	}
}

void OsService_DdrLoBlockFree(void* ptr) {
	IPI3_Msg msg = {
			.function = OSF_DDR_LO_BLOCK_FREE,
			.OSF_DDRloBlockFree.free_blocks_starting_at = ((uintptr_t)ptr - DDR_DDR4_0_BASE_ADDR) / (1024*1024),
			};

	IPI3_OsService_Submit(&msg);
}
