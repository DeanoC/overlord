#include "core/core.h"
#include "zynqps8/dma/lpddma.hpp"
#include "platform/memory_map.h"
#include "dbg/assert.h"
#include "zdma.hpp"

namespace Dma::LpdDma {
static uintptr_lo_t GetBaseAddress(Channels const channel_) {
	uintptr_lo_t baseAddr = 0;
	switch (channel_) {
		case Channels::ChannelZero: baseAddr = LPD_DMA_CH0_BASE_ADDR; break;
		case Channels::ChannelOne: baseAddr = LPD_DMA_CH1_BASE_ADDR; break;
		case Channels::ChannelTwo: baseAddr = LPD_DMA_CH2_BASE_ADDR; break;
		case Channels::ChannelThree: baseAddr = LPD_DMA_CH3_BASE_ADDR; break;
		case Channels::ChannelFour: baseAddr = LPD_DMA_CH4_BASE_ADDR; break;
		case Channels::ChannelFive: baseAddr = LPD_DMA_CH5_BASE_ADDR; break;
		case Channels::ChannelSix: baseAddr = LPD_DMA_CH6_BASE_ADDR; break;
		case Channels::ChannelSevern: baseAddr = LPD_DMA_CH7_BASE_ADDR; break;
	}
	assert(baseAddr != 0);
	return baseAddr;
}
AsyncToken SimpleDmaSet32(Channels const channel, const uint32_t data, uintptr_all_t address, uint32_t size) {
	ZDma::SimpleDmaSet32(GetBaseAddress(channel), data, address, size);
	return channel;
}
AsyncToken SimpleDmaSet8(const Channels channel, const uint8_t data, uintptr_all_t address, uint32_t size) {
	uint32_t const data32 = (uint32_t) data << 24 | (uint32_t) data << 16 | (uint32_t) data << 8 | (uint32_t) data << 0;
	ZDma::SimpleDmaSet32(GetBaseAddress(channel), data32, address, size);
	return channel;
}

AsyncToken SimpleDmaCopy(const Channels channel, uintptr_all_t src, uintptr_all_t dest, uint32_t size) {
	ZDma::SimpleDmaCopy(GetBaseAddress(channel), src, dest, size);
	return channel;
}

void Stall(Channels channel) {
	ZDma::Stall( GetBaseAddress((channel)));
}

void StallForToken(AsyncToken token) {
	ZDma::Stall( GetBaseAddress(token));
}

} // end namespace
