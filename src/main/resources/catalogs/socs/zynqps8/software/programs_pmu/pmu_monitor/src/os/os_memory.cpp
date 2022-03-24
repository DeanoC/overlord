#include "core/core.h"
#include "osservices/ipi3_transport.h"
#include "ipi3_os_server.hpp"
#include "../os_heap.hpp"

namespace IPI3_OsServer {

void DdrLoBlockAlloc(IPI_Channel senderChannel, IPI3_Msg const *msgBuffer) {
	IPI3_Response responseBuffer;
	uint32_t blocksWanted = msgBuffer->Payload.DdrLoBlockAlloc.blocks64KB;

	debug_printf("Request for %lu KB from low memory\n", blocksWanted * 64);

	uintptr_t address = osHeap->ddrLoAllocator.Alloc(blocksWanted);
	if (address == (uintptr_t) ~0) {
		responseBuffer.result = IRR_OUT_OF_MEMORY;
	} else {
//		debug_printf("SUCCESS %x\n", address);
//		osHeap->ddrLoAllocator.DebugDumpMasks(4);

		responseBuffer.result = IRR_SUCCESS;
		responseBuffer.DdrLoBlockAlloc.offset = address;
	}

	SubmitResponse(senderChannel, &responseBuffer);
}

void DdrLoBlockFree(IPI3_Msg const *msgBuffer) {
	uintptr_t const address = msgBuffer->Payload.DdrLoBlockFree.offset;
	uint16_t const blockCount = msgBuffer->Payload.DdrLoBlockFree.blockCount;
//	osHeap->ddrLoAllocator.DebugDumpMasks(4);
	debug_printf("Free of low %d KB %x\n", blockCount * 64, address);

	osHeap->ddrLoAllocator.Free(address, blockCount);
}

void DdrHiBlockAlloc(IPI_Channel senderChannel, IPI3_Msg const *msgBuffer) {

	IPI3_Response responseBuffer;
	uint32_t blocksWanted = msgBuffer->Payload.DdrHiBlockAlloc.blocks64KB;

	debug_printf("Request for %lu KB from hi memory\n", blocksWanted * 64);

	uintptr_t address = osHeap->ddrHiAllocator.Alloc(blocksWanted);
	if (address == (uintptr_t) ~0) {
		responseBuffer.result = IRR_OUT_OF_MEMORY;
	} else {
		responseBuffer.result = IRR_SUCCESS;
		responseBuffer.DdrHiBlockAlloc.offset = address;
	}

	SubmitResponse(senderChannel, &responseBuffer);
}

void DdrHiBlockFree(IPI3_Msg const *msgBuffer) {
	uintptr_t const address = msgBuffer->Payload.DdrHiBlockFree.offset;
	uint16_t const blockCount = msgBuffer->Payload.DdrHiBlockFree.blockCount;
	debug_printf("Free of hi %d KB %x\n", blockCount * 64, address);
	osHeap->ddrHiAllocator.Free(address, blockCount);
}

} // end namespace