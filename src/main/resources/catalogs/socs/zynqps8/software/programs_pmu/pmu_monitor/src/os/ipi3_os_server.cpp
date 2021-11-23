#include "core/core.h"
#include "hw/memory_map.h"
#include "osservices/ipi3_transport.h"
#include "ipi3_os_server.hpp"
#include "../os_heap.hpp"
#include "dbg/assert.h"
#include "zynqps8/dma/lpddma.hpp"
#include "osservices/osservices.h"
#include "dbg/ansi_escapes.h"

OsHeap *osHeap;

uint8_t textConsoleSkip;
uint8_t textConsoleSkipCurrent;

static void TextConsoleDrawCallback() {
	if(osHeap->console.framebuffer != nullptr && osHeap->screenConsoleEnabled) {
		if(textConsoleSkipCurrent == textConsoleSkip) {
			textConsoleSkipCurrent = 0;
			auto const frameBuffer = osHeap->console.framebuffer;
			auto const width = osHeap->console.frameBufferWidth;
			auto const height = osHeap->console.frameBufferHeight;

			GfxDebug::RGBA8 drawer(width, height, frameBuffer);
			osHeap->console.console.Display(&drawer, 0, 0);
		} else {
			textConsoleSkipCurrent++;
		}
	}
}

namespace IPI3_OsServer {

void Init() {
	osHeap->console.Init();
	osHeap->thirtyHzCallbacks[(int)ThirtyHzTasks::TEXT_CONSOLE] = &TextConsoleDrawCallback;
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
		case OSF_SCREEN_CONSOLE_INLINE_PRINT: ScreenConsoleInlinePrint(msgBuffer);
			break;
		case OSF_BOOT_COMPLETE: BootComplete(msgBuffer);
			break;
		case OSF_SCREEN_CONSOLE_ENABLE: ScreenConsoleEnable(msgBuffer);
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
		case OSF_SCREEN_CONSOLE_PTR_PRINT: ScreenConsolePtrPrint(senderChannel, msgBuffer);
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
					osHeap->tmpOsBufferAllocator.Free(tmp);
					return;
				}

			} else {
				addr = (uintptr_t) msgBuffer->Payload.DdrPacket.packetDdrAddress;
			}
		}
		HandleNeedResponse(senderChannel, (IPI3_Msg *) addr);
	}
}

void DdrLoBlockAlloc(IPI_Channel senderChannel, IPI3_Msg const *msgBuffer) {
	IPI3_Response responseBuffer;
	const uint32_t blocksWanted = msgBuffer->Payload.DdrLoBlockAlloc.blocks1MB;
	if(blocksWanted >= 128) {
		responseBuffer.result = IRR_BAD_PARAMETERS;
	} else {
		responseBuffer.result = IRR_SUCCESS;
		uintptr_t address = osHeap->ddrLoAllocator.Alloc(blocksWanted);
		if (address == (uintptr_t) ~0) {
			responseBuffer.result = IRR_OUT_OF_MEMORY;
		} else {
			responseBuffer.DdrLoBlockAlloc.block_1MB_Offset = address / (1024 * 1024);
		}
	}
	SubmitResponse(senderChannel, &responseBuffer);
}

void DdrLoBlockFree(IPI3_Msg const *msgBuffer) {
	uintptr_t address = msgBuffer->Payload.DdrLoBlockFree.free_blocks_starting_at * (1024 * 1024);
	osHeap->ddrLoAllocator.Free(address);
}

void DdrHiBlockAlloc(IPI_Channel senderChannel, IPI3_Msg const *msgBuffer) {
	IPI3_Response responseBuffer;
	const uint32_t blocksWanted = msgBuffer->Payload.DdrHiBlockAlloc.blocks1MB;
	if(blocksWanted >= 128) {
		responseBuffer.result = IRR_BAD_PARAMETERS;
	} else {
		responseBuffer.result = IRR_SUCCESS;
		uintptr_t address = osHeap->ddrHiAllocator.Alloc(blocksWanted);
		if (address == (uintptr_t) ~0) {
			responseBuffer.result = IRR_OUT_OF_MEMORY;
		} else {
			responseBuffer.DdrHiBlockAlloc.block_1MB_Offset = address / (1024 * 1024);
		}
	}
	SubmitResponse(senderChannel, &responseBuffer);
}

void DdrHiBlockFree(IPI3_Msg const *msgBuffer) {
	uintptr_t address = msgBuffer->Payload.DdrHiBlockFree.free_blocks_starting_at * (1024 * 1024);
	osHeap->ddrHiAllocator.Free(address);
}

void ScreenConsoleInlinePrint(IPI3_Msg const *msgBuffer) {
	uint8_t size = msgBuffer->Payload.InlinePrint.size;
	const auto *text = (const char *) msgBuffer->Payload.InlinePrint.text;
	osHeap->console.console.PrintWithSize(size, text);
}

void BootComplete(IPI3_Msg const *msgBuffer) {
	memcpy(&osHeap->bootData, &msgBuffer->Payload.BootData, sizeof(BootData));

	osHeap->console.frameBufferWidth = msgBuffer->Payload.BootData.bootData.frameBufferWidth;
	osHeap->console.frameBufferHeight = msgBuffer->Payload.BootData.bootData.frameBufferHeight;
	osHeap->console.framebuffer = (uint8_t*) msgBuffer->Payload.BootData.bootData.videoBlock + 4096;
	if(osHeap->console.framebuffer != nullptr) {
		auto const frameBuffer = osHeap->console.framebuffer;
		auto const width = osHeap->console.frameBufferWidth;
		auto const height = osHeap->console.frameBufferHeight;
		osHeap->screenConsoleEnabled = true;
		GfxDebug::RGBA8 drawer(width, height, frameBuffer);
		drawer.backgroundColour = 0xFF808080;
		drawer.Clear();
		osHeap->console.console.PrintLn( ANSI_GREEN_PEN ANSI_BRIGHT "Welcome to Intex Systems" ANSI_RESET_ATTRIBUTES);
	} else {
		osHeap->screenConsoleEnabled = false;
	}
	// back up boot program
	using namespace Dma::LpdDma;
	Stall(Channels::ChannelSevern);
	SimpleDmaCopy(Channels::ChannelSevern, osHeap->bootData.bootCodeStart, (uintptr_all_t) osHeap->bootOCMStore, osHeap->bootData.bootCodeSize);
//	Stall(Channels::ChannelSevern);
//	SimpleDmaSet8(Channels::ChannelSevern, 0, osHeap->bootData.bootCodeStart, osHeap->bootData.bootCodeSize);
}

void ScreenConsoleEnable(IPI3_Msg const *msgBuffer) {
	osHeap->screenConsoleEnabled  = msgBuffer->Payload.ScreenConsoleEnable.enabled;
}

void ScreenConsolePtrPrint(IPI_Channel senderChannel, IPI3_Msg const *msgBuffer) {
	uint32_t size = msgBuffer->Payload.DdrPacket.packetSize - IPI3_HEADER_SIZE - sizeof(IPI3_DdrPacket);
	const auto *text = (const char *) (msgBuffer->Payload.PtrPrint.text);
	osHeap->console.console.PrintWithSize(size, text);
}

} // end namespace
