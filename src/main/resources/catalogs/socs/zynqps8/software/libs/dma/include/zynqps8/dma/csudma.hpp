#pragma once

// CsuDma library includes the PCAP part of the chip
namespace Dma::CsuDma {

bool PcapInit();

void SimpleDmaCopy(Channels channel, uintptr_all_t src, uintptr_all_t dest, uint32_t size);
void DmaTransferToPCAP( uintptr_all_t src, uint32_t size );
//void DmaTransferViaAES(Channels channel, uintptr_all_t src, uintptr_all_t dest, uint32_t size);
//void DmaTransferViaSHA3(Channels channel, uintptr_all_t src, uintptr_all_t dest, uint32_t size);

void Stall();

};

}