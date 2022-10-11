#include "core/core.h"
#include "osservices/ipi3_transport.h"
#include "ipi3_os_server.hpp"
#include "../os_heap.hpp"
#include "dbg/ansi_escapes.h"
#include "zynqps8/dma/lpddma.hpp"

namespace IPI3_OsServer {

void BootComplete(IPI3_Msg const *msgBuffer) {
	memcpy(&osHeap->bootData, &msgBuffer->Payload.BootData, sizeof(BootData));

	// back up boot program
	using namespace Dma::LpdDma;
	Stall(Channels::ChannelSevern);
	SimpleDmaCopy(Channels::ChannelSevern,
								osHeap->bootData.bootCodeStart,
								(uintptr_all_t) osHeap->bootOCMStore,
								osHeap->bootData.bootCodeSize);
}

void FetchBootData(IPI_Channel senderChannel, const IPI3_Msg *msgBuffer) {
	IPI3_Response responseBuffer;
	responseBuffer.result = IRR_SUCCESS;
	memcpy(&responseBuffer.BootData.bootData, &osHeap->bootData, sizeof(BootData));
	SubmitResponse(senderChannel, &responseBuffer);
}

} // end namespace