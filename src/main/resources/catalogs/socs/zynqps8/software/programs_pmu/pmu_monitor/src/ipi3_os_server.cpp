#include "core/core.h"
#include "hw/memory_map.h"
#include "hw/reg_access.h"
#include "hw_regs/ipi.h"
#include "hw_regs/uart.h"
#include "osservices/ipi3_transport.h"
#include "ipi3_os_server.h"
#include "os_heap.h"
#include "dbg/assert.h"
#include "utils/string_utils.hpp"
#include "zynqps8/dma/lpddma.hpp"

static struct {
	bool replyBufferInUse[IA_PMU + 1];
} IPI3_Data;

extern uint32_t uart0TransmitLast;
extern uint32_t uart0TransmitHead;

OsHeap *osHeap;

extern "C" void IPI3_OSServiceInit() {
	// allocate the 1MB DDR heap for the OS
	osHeap = (OsHeap*) DDR_DDR4_0_BASE_ADDR;

	// gcc warns osHeap is null, but really its just DDR start is 0x0 address
	// so turn off the null checker just here.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnonnull"
	memset(osHeap, 0, (1024*1024));
#pragma GCC diagnostic pop

	osHeap->ddrLoAllocator.Init(OsHeap::TotalSize);
	osHeap->ddrHiAllocator.Init(0);

	// fill in a poison 'null' page
	memset((void*)(osHeap->nullBlock), 0xDC, sizeof(osHeap->nullBlock));

	osHeap->tmpOsBufferAllocator.Init((uintptr_lo_t)osHeap->tmpBuffer);

}

static bool IPI3_IsFireAndForget(OS_ServiceFunc func) {
	return func & OSF_FIRE_AND_FORGET_BIT;
}

void IPI3_OSServiceHandlerFireAndForget(const IPI3_Msg * const msgBuffer) {
	assert(IPI3_IsFireAndForget(msgBuffer->function) == true);
	switch (msgBuffer->function) {
		case OSF_INLINE_PRINT: IPI3_OSServer_InlinePrint(msgBuffer); break;
		case OSF_DDR_LO_BLOCK_FREE: IPI3_OSServer_DdrLoBlockFree(msgBuffer); break;
		case OSF_DDR_HI_BLOCK_FREE: IPI3_OSServer_DdrHiBlockFree(msgBuffer); break;

		default: debug_printf("Invalid function 0x%x in fire and forget handler IPI3\n", msgBuffer->function);
	}
}

void IPI3_OSServiceHandlerNeedResponse(IPI_Channel const senderChannel, const IPI3_Msg * const msgBuffer) {
	assert(IPI3_IsFireAndForget(msgBuffer->function) == false);

	switch (msgBuffer->function) {
		case OSF_PTR_PRINT: IPI3_OSServer_PtrPrint(senderChannel, msgBuffer); break;
		case OSF_DDR_LO_BLOCK_ALLOC: IPI3_OSServer_DdrLoBlockAlloc(senderChannel, msgBuffer); break;
		case OSF_DDR_HI_BLOCK_ALLOC: IPI3_OSServer_DdrHiBlockAlloc(senderChannel, msgBuffer); break;
		default: debug_printf("Invalid function 0x%x in need response handler IPI3\n", msgBuffer->function);
	}

	// tell the sender the reply buffer is full and that it needs to tell us
	// when its copied the data out so we can use it again
	IPI3_Data.replyBufferInUse[IPI_ChannelToAgent(senderChannel)] = true;

}

bool IPI3_OSServiceHandler(IPI_Channel senderChannel) {
	const IPI3_Msg* msgBuffer = (const IPI3_Msg*) IPI_MSG(IPI_ChannelToBuffer(senderChannel), IA_PMU);
//	if(senderChannel != IC_PMU_3) {
//		raw_debug_printf("S senderChannel 0x%x %p function %d\n", senderChannel, msgBuffer, msgBuffer->function);
//	}
	// special case reply buffer free
	if(msgBuffer->function == OSF_RESPONSE_BUFFER_FREE) {
		IPI3_Data.replyBufferInUse[IPI_ChannelToAgent(senderChannel)] = false;
		HW_REG_SET(IPI, PMU_3_ISR, senderChannel);
		return true;
	}

	if(IPI3_IsFireAndForget(msgBuffer->function)) {
		// free it up early (allows some better pipelining) as fire and forget
		// copy the msg buffer
		IPI3_Msg msgBufferCopy;
		memcpy(&msgBufferCopy, msgBuffer, 32);
		HW_REG_SET(IPI, PMU_3_ISR, senderChannel);
		IPI3_OSServiceHandlerFireAndForget(&msgBufferCopy);
		return true;
	} else {
		uintptr_t addr = (uintptr_t) msgBuffer;
		if(msgBuffer->ddrPtrFlag != 0) {
			if(msgBuffer->Payload.DdrPacket.packetDdrAddress > UINT32_MAX) {
				if(msgBuffer->Payload.DdrPacket.packetSize < OsHeap::BounceBufferSize) {
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
					auto tmp = osHeap->tmpOsBufferAllocator.Alloc(BitOp::PowerOfTwoContaining(msgBuffer->Payload.DdrPacket.packetSize/64));
					Dma::LpdDma::Stall(Dma::LpdDma::Channels::ChannelSevern);
					Dma::LpdDma::SimpleDmaCopy(Dma::LpdDma::Channels::ChannelSevern,
																		 msgBuffer->Payload.DdrPacket.packetDdrAddress,
																		 (uintptr_all_t) tmp,
																		 msgBuffer->Payload.DdrPacket.packetSize);
					Dma::LpdDma::Stall(Dma::LpdDma::Channels::ChannelSevern);
					IPI3_OSServiceHandlerNeedResponse(senderChannel, (IPI3_Msg*)tmp);
					osHeap->tmpOsBufferAllocator.Free(tmp);
					return false;
				}

			} else {
				addr = (uintptr_t)msgBuffer->Payload.DdrPacket.packetDdrAddress;
			}
		}
		IPI3_OSServiceHandlerNeedResponse(senderChannel, (IPI3_Msg*)addr);
	}
	return false;
}

void IPI3_OsService_SubmitResponse(IPI_Channel senderChannel, const IPI3_Response *const response) {
	const IPI_Agent senderAgent = IPI_ChannelToAgent(senderChannel);

	// is the sender ipi reply buffer okay to reuse?
	while (IPI3_Data.replyBufferInUse[senderAgent]) {
		// stall till buffer is free to use
	}

	memcpy(IPI_RESPONSE(IPI_ChannelToBuffer(senderChannel), IA_PMU), response, 32);
}

void PutSizedData(uint32_t size, const uint8_t * text) {
	if(size == 0) return;

	//put text into to transmit buffer, interrupts will send it to host

	// split at buffer end
	if(uart0TransmitHead + size >= OsHeap::UartBufferSize) {
		const uint32_t firstBlockSize = OsHeap::UartBufferSize - uart0TransmitHead;
		if(firstBlockSize > 0) {
			memcpy(&osHeap->uart0TransmitBuffer[uart0TransmitHead], text, firstBlockSize);
			text += firstBlockSize;
			size -= firstBlockSize;
		}
		uart0TransmitHead = 0;
	}

	if(size > 0)
	{
		memcpy(&osHeap->uart0TransmitBuffer[uart0TransmitHead], text, size);
		uart0TransmitHead += (uint32_t)size;
	}

	HW_REG_SET(UART0, INTRPT_EN, UART_INTRPT_EN_TEMPTY);
}


#define IsTransmitFull() (HW_REG_GET_BIT(UART0, CHANNEL_STS, TNFUL))

void IPI3_OSServer_InlinePrint(const IPI3_Msg * const msgBuffer) {
	uint8_t size = msgBuffer->Payload.InlinePrint.size;
	const auto* text = (const uint8_t *)msgBuffer->Payload.InlinePrint.text;
	PutSizedData(size, text);
}

void IPI3_OSServer_PtrPrint(IPI_Channel senderChannel, const IPI3_Msg * const msgBuffer) {
	uint32_t size = msgBuffer->Payload.DdrPacket.packetSize - 32;
	const auto* text = (const uint8_t *)(msgBuffer + 1);
	PutSizedData(size, text);
}

void IPI3_OSServer_DdrLoBlockAlloc(IPI_Channel senderChannel, const IPI3_Msg * const msgBuffer) {
	IPI3_Response responseBuffer;
	const uint32_t blocksWanted = msgBuffer->Payload.DdrLoBlockAlloc.blocks1MB;
	responseBuffer.result = IRR_SUCCESS;
	uintptr_t address =	osHeap->ddrLoAllocator.Alloc(blocksWanted);
	if(address == (uintptr_t)~0) {
		responseBuffer.result = IRR_OUT_OF_MEMORY;
	} else {
		responseBuffer.DdrLoBlockAlloc.block_1MB_Offset = address / (1024*1024);
	}

	IPI3_OsService_SubmitResponse(senderChannel, &responseBuffer);
}

void IPI3_OSServer_DdrLoBlockFree(const IPI3_Msg * const msgBuffer) {
	uintptr_t address = msgBuffer->Payload.DdrLoBlockFree.free_blocks_starting_at * (1024*1024);
	osHeap->ddrLoAllocator.Free(address);
}

void IPI3_OSServer_DdrHiBlockAlloc(IPI_Channel senderChannel, const IPI3_Msg * const msgBuffer) {
	IPI3_Response responseBuffer;
	const uint32_t blocksWanted = msgBuffer->Payload.DdrHiBlockAlloc.blocks1MB;
	responseBuffer.result = IRR_SUCCESS;
	uintptr_t address =	osHeap->ddrHiAllocator.Alloc(blocksWanted);
	if(address == (uintptr_t)~0) {
		responseBuffer.result = IRR_OUT_OF_MEMORY;
	} else {
		responseBuffer.DdrHiBlockAlloc.block_1MB_Offset = address / (1024*1024);
	}

	IPI3_OsService_SubmitResponse(senderChannel, &responseBuffer);
}

void IPI3_OSServer_DdrHiBlockFree(const IPI3_Msg * const msgBuffer) {
	uintptr_t address = msgBuffer->Payload.DdrHiBlockFree.free_blocks_starting_at * (1024*1024);
	osHeap->ddrHiAllocator.Free(address);
}

// override the weak print, to write directly into the buffer
extern "C" void OsService_Print(const char *const text) {
	unsigned int count = Utils::StringLength(text);;
	PutSizedData(count, (const uint8_t*)text);
}

extern "C" void OsService_PrintWithSize(unsigned int count, const char *const text) {
	PutSizedData(count, (const uint8_t*)text);
}

