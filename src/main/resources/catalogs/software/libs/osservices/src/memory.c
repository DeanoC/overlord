#include "core/core.h"
#include "osservices/osservices.h"
#include "osservices/ipi3_transport.h"
#include "platform/memory_map.h"

uintptr_lo_t OsService_DdrLoBlockAlloc(uint16_t blocks1MB) {
	IPI3_Msg msg = {
			.function = OSF_DDR_LO_BLOCK_ALLOC,
			.Payload.DdrLoBlockAlloc.blocks1MB = blocks1MB,
			};
	IPI3_Response response;
	IPI3_OnService_SubmitAndFetchResponse(&msg, &response);
	if(response.result == IRR_SUCCESS) {
		return (uintptr_lo_t)(MAINDDR4_0_BASE_ADDR + response.DdrLoBlockAlloc.block_1MB_Offset * (1024 * 1024));
	} else {
		OsService_InlinePrint(OSS_INLINE_TEXT("DdrLoBlocKAlloc failed"));
		switch(response.result) {
			case IRR_BAD_PARAMETERS:
				OsService_InlinePrint(OSS_INLINE_TEXT(" : IRR_BAD_PARAMETERS\n"));
			case IRR_OUT_OF_MEMORY:
				OsService_InlinePrint(OSS_INLINE_TEXT(" : IRR_OUT_OF_MEMORY\n"));
				break;
			default:
				OsService_InlinePrint(OSS_INLINE_TEXT("\n"));
				break;
		}
		return 0;
	}
}

void OsService_DdrLoBlockFree(uintptr_lo_t ptr) {
	IPI3_Msg msg = {
			.function = OSF_DDR_LO_BLOCK_FREE,
			.Payload.DdrLoBlockFree.free_blocks_starting_at = ((uintptr_t)ptr - MAINDDR4_0_BASE_ADDR) / (1024*1024),
			};

	IPI3_OsService_Submit(&msg);
}

uintptr_all_t OsService_DdrHiBlockAlloc(uint16_t blocks1MB) {
#if CPU_a53
	IPI3_Msg msg = {
			.function = OSF_DDR_HI_BLOCK_ALLOC,
			.Payload.DdrLoBlockAlloc.blocks1MB = blocks1MB,
	};
	IPI3_Response response;
	IPI3_OnService_SubmitAndFetchResponse(&msg, &response);
	if(response.result == IRR_SUCCESS) {
		return (uintptr_t) (MAINDDR4_1_BASE_ADDR + response.DdrLoBlockAlloc.block_1MB_Offset * (1024 * 1024));
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
		return 0;
	}
#else
	// only a53 can alloc hi ddr and use it
	return 0;
#endif
}

void OsService_DdrHiBlockFree(uintptr_all_t ptr) {
#if CPU_a53
	IPI3_Msg msg = {
			.function = OSF_DDR_HI_BLOCK_FREE,
			.Payload.DdrLoBlockFree.free_blocks_starting_at = ((uintptr_t)ptr - MAINDDR4_0_BASE_ADDR) / (1024*1024),
	};

	IPI3_OsService_Submit(&msg);
#else
	// only a53 can alloc hi ddr and use it
#endif
}

