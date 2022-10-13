#pragma once

#include "core/core.h"

// CsuDma library includes the PCAP part of the chip
namespace Dma::CsuDma {

bool PcapInit();
void TransferToPcap( uintptr_all_t src_, uint32_t sizeInBytes_ );

//void SimpleDmaCopy(Channels channel, uintptr_all_t src, uintptr_all_t dest, uint32_t size);
//void DmaTransferViaAES(Channels channel, uintptr_all_t src, uintptr_all_t dest, uint32_t size);
//void DmaTransferViaSHA3(Channels channel, uintptr_all_t src, uintptr_all_t dest, uint32_t size);

void StallForPcap();

};