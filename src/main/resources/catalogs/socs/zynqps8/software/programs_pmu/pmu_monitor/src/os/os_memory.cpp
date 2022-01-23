#include "core/core.h"
#include "osservices/ipi3_transport.h"
#include "ipi3_os_server.hpp"
#include "../os_heap.hpp"

namespace IPI3_OsServer {

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

} // end namespace