#include "core/core.h"
#include "osservices/ipi3_transport.h"
#include "ipi3_os_server.hpp"
#include "../os_heap.hpp"
#include "dbg/ansi_escapes.h"
#include "zynqps8/dma/lpddma.hpp"

namespace IPI3_OsServer {

void BootComplete(IPI3_Msg const *msgBuffer) {
	memcpy(&osHeap->bootData, &msgBuffer->Payload.BootData, sizeof(BootData));

	osHeap->console.frameBufferWidth = msgBuffer->Payload.BootData.bootData.frameBufferWidth;
	osHeap->console.frameBufferHeight = msgBuffer->Payload.BootData.bootData.frameBufferHeight;
	osHeap->console.framebuffer = (uint8_t *) msgBuffer->Payload.BootData.bootData.videoBlock + 4096;
	if (osHeap->console.framebuffer != nullptr) {
		auto const frameBuffer = osHeap->console.framebuffer;
		auto const width = osHeap->console.frameBufferWidth;
		auto const height = osHeap->console.frameBufferHeight;
		osHeap->screenConsoleEnabled = true;
		GfxDebug::RGBA8 drawer(width, height, frameBuffer);
		drawer.backgroundColour = 0xFF808080;
		drawer.Clear();
		osHeap->console.console.PrintLn(ANSI_GREEN_PEN ANSI_BRIGHT_ON "Welcome to Intex Systems" ANSI_RESET_ATTRIBUTES);
	} else {
		osHeap->screenConsoleEnabled = false;
	}
	// back up boot program
	using namespace Dma::LpdDma;
	Stall(Channels::ChannelSevern);
	SimpleDmaCopy(Channels::ChannelSevern,
								osHeap->bootData.bootCodeStart,
								(uintptr_all_t) osHeap->bootOCMStore,
								osHeap->bootData.bootCodeSize);
	//	Stall(Channels::ChannelSevern);
	//	SimpleDmaSet8(Channels::ChannelSevern, 0, osHeap->bootData.bootCodeStart, osHeap->bootData.bootCodeSize);
}

void FetchBootData(IPI_Channel senderChannel, const IPI3_Msg *msgBuffer) {
	IPI3_Response responseBuffer;
	responseBuffer.result = IRR_SUCCESS;
	memcpy(&responseBuffer.BootData.bootData, &osHeap->bootData, sizeof(BootData));
	SubmitResponse(senderChannel, &responseBuffer);
}

} // end namespace