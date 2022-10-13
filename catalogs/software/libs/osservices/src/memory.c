#include "core/core.h"
#include "osservices/osservices.h"
#include "osservices/ipi3_transport.h"
#include "platform/memory_map.h"

uintptr_lo_t OsService_DdrLoBlockAlloc(uint32_t blocks64KB_, uint32_t tag_) {
	IPI3_Msg msg = {
			.function = OSF_DDR_LO_BLOCK_ALLOC,
			.Payload.DdrLoBlockAlloc.blocks64KB = blocks64KB_,
			.Payload.DdrLoBlockAlloc.tag = tag_,
		};
	IPI3_Response response;
	IPI3_OnService_SubmitAndFetchResponse(&msg, &response);
	if(response.result == IRR_SUCCESS) {
		return (uintptr_lo_t)(DDR_0_BASE_ADDR + response.DdrLoBlockAlloc.offset);
	} else {
		OsService_InlinePrint(OSS_INLINE_TEXT("DdrLoBlockAlloc failed"));
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

void OsService_DdrLoBlockFree(uintptr_lo_t ptr_, uint32_t blockCount_, uint32_t tag_) {
	IPI3_Msg msg = {
			.function = OSF_DDR_LO_BLOCK_FREE,
			.Payload.DdrLoBlockFree.offset = ((uintptr_t)ptr_ - DDR_0_BASE_ADDR),
			.Payload.DdrLoBlockFree.blockCount = blockCount_,
			.Payload.DdrLoBlockFree.tag = tag_,
	};

	IPI3_OsService_Submit(&msg);
}

uintptr_all_t OsService_DdrHiBlockAlloc(uint32_t blocks64KB_, uint32_t tag_) {
#if CPU_a53
	IPI3_Msg msg = {
			.function = OSF_DDR_HI_BLOCK_ALLOC,
			.Payload.DdrHiBlockAlloc.blocks64KB = blocks64KB_,
			.Payload.DdrHiBlockAlloc.tag = tag_,
	};
	IPI3_Response response;
	IPI3_OnService_SubmitAndFetchResponse(&msg, &response);
	if(response.result == IRR_SUCCESS) {
		return (uintptr_t) (DDR_1_BASE_ADDR + response.DdrLoBlockAlloc.offset);
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

void OsService_DdrHiBlockFree(uintptr_all_t ptr, uint32_t blockCount_, uint32_t tag_) {
#if CPU_a53
	IPI3_Msg msg = {
			.function = OSF_DDR_HI_BLOCK_FREE,
			.Payload.DdrHiBlockFree.offset = ((uintptr_t)ptr - DDR_0_BASE_ADDR),
			.Payload.DdrHiBlockFree.blockCount = blockCount_,
			.Payload.DdrHiBlockFree.tag = tag_,
	};

	IPI3_OsService_Submit(&msg);
#else
	// only a53 can alloc hi ddr and use it
#endif
}

