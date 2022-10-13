#include "core/core.h"
#include "platform/memory_map.h"
#include "osservices/ipi3_transport.h"
#include "ipi3_os_server.hpp"
#include "../os_heap.hpp"
#include "dbg/assert.h"
#include "zynqps8/dma/lpddma.hpp"

OsHeap *osHeap;

namespace IPI3_OsServer {

void Init() {
}

static bool IsFireAndForget(OS_ServiceFunc func) {
	return func & OSF_FIRE_AND_FORGET_BIT;
}

void HandleFireAndForget(const IPI3_Msg *const msgBuffer) {
	assert(IsFireAndForget(msgBuffer->function) == true);
	switch (msgBuffer->function) {
		case OSF_INLINE_PRINT: DebugInlinePrint(msgBuffer);
			break;
		case OSF_DDR_LO_BLOCK_FREE: DdrLoBlockFree(msgBuffer);
			break;
		case OSF_DDR_HI_BLOCK_FREE: DdrHiBlockFree(msgBuffer);
			break;
		case OSF_BOOT_COMPLETE: BootComplete(msgBuffer);
			break;
		case OSF_CPU_WAKE_OR_SLEEP: CpuWakeOrSleep(msgBuffer);
			break;
		default: debug_printf("Invalid function 0x%x in fire and forget handler IPI3\n", msgBuffer->function);
	}
}

void HandleNeedResponse(IPI_Channel const senderChannel, const IPI3_Msg *const msgBuffer) {
	assert(IsFireAndForget(msgBuffer->function) == false);

	switch (msgBuffer->function) {
		case OSF_PTR_PRINT: DebugPtrPrint(senderChannel, msgBuffer);
			break;
		case OSF_DDR_LO_BLOCK_ALLOC: DdrLoBlockAlloc(senderChannel, msgBuffer);
			break;
		case OSF_DDR_HI_BLOCK_ALLOC: DdrHiBlockAlloc(senderChannel, msgBuffer);
			break;
		case OSF_FETCH_BOOT_DATA: FetchBootData(senderChannel, msgBuffer);
			break;
		default: debug_printf("Invalid function 0x%x in need response handler IPI3\n", msgBuffer->function);
	}
}

void SubmitResponse(IPI_Channel senderChannel, const IPI3_Response *const response) {
	memcpy(IPI_RESPONSE(IPI_ChannelToBuffer(senderChannel), IA_PMU), response, 32);
}

void Handler(IPI_Channel senderChannel) {
	auto msgBuffer = (const IPI3_Msg *) IPI_MSG(IPI_ChannelToBuffer(senderChannel), IA_PMU);
//	if(senderChannel != IC_PMU_3) {
//			raw_debug_printf("S senderChannel 0x%x %p function %d\n", senderChannel, msgBuffer, msgBuffer->function);
//	}

	if (IsFireAndForget(msgBuffer->function)) {
		HandleFireAndForget(msgBuffer);
		return;
	} else {
		uintptr_t addr = (uintptr_t) msgBuffer;
		if (msgBuffer->ddrPtrFlag != 0) {
			if (msgBuffer->Payload.DdrPacket.packetDdrAddress > UINT32_MAX) {
				if (msgBuffer->Payload.DdrPacket.packetSize < OsHeap::BounceBufferSize) {
					Dma::LpdDma::Stall(Dma::LpdDma::Channels::ChannelSevern);
					Dma::LpdDma::SimpleDmaCopy(Dma::LpdDma::Channels::ChannelSevern,
																		 msgBuffer->Payload.DdrPacket.packetDdrAddress,
																		 (uintptr_all_t) osHeap->bounceBuffer,
																		 msgBuffer->Payload.DdrPacket.packetSize);
					Dma::LpdDma::Stall(Dma::LpdDma::Channels::ChannelSevern);
					addr = (uintptr_t) osHeap->bounceBuffer;
				} else {
					// 64 bit memory space and bigger than bounceBuffer might be able to
					// cope if its fits in our tmp space thats left...
					uint32_t const blockCount = BitOp::PowerOfTwoContaining(msgBuffer->Payload.DdrPacket.packetSize / 64);
					raw_debug_printf("blockCOunt %lu\n",blockCount);
					if(blockCount > 128) {
						IPI3_Response responseBuffer;
						responseBuffer.result = IRR_BAD_PARAMETERS;
						SubmitResponse(senderChannel, &responseBuffer);
						return;
					}
					auto tmp = osHeap->tmpOsBufferAllocator.Alloc(blockCount);
					if(tmp == ~0U) {
						IPI3_Response responseBuffer;
						responseBuffer.result = IRR_BAD_PARAMETERS;
						SubmitResponse(senderChannel, &responseBuffer);
						return;
					}
					Dma::LpdDma::Stall(Dma::LpdDma::Channels::ChannelSevern);
					Dma::LpdDma::SimpleDmaCopy(Dma::LpdDma::Channels::ChannelSevern,
																		 msgBuffer->Payload.DdrPacket.packetDdrAddress,
																		 (uintptr_all_t) tmp,
																		 msgBuffer->Payload.DdrPacket.packetSize);
					Dma::LpdDma::Stall(Dma::LpdDma::Channels::ChannelSevern);
					HandleNeedResponse(senderChannel, (IPI3_Msg *) tmp);
					osHeap->tmpOsBufferAllocator.Free(tmp, blockCount);
					return;
				}

			} else {
				addr = (uintptr_t) msgBuffer->Payload.DdrPacket.packetDdrAddress;
			}
		}
		HandleNeedResponse(senderChannel, (IPI3_Msg *) addr);
	}
}

} // end namespace
