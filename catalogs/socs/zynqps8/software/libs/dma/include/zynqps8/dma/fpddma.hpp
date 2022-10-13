#pragma once

#include "core/core.h"

namespace Dma::FpdDma {
enum class Channels : uint8_t {
	ChannelZero = 0,
	ChannelOne,
	ChannelTwo,
	ChannelThree,
	ChannelFour,
	ChannelFive,
	ChannelSix,
	ChannelSevern,
};
typedef Channels AsyncToken;

AsyncToken SimpleDmaSet8(Channels channel, uint32_t data, uintptr_all_t address, uint32_t size);
AsyncToken SimpleDmaSet32(Channels channel, uint8_t data, uintptr_all_t address, uint32_t size);

AsyncToken SimpleDmaCopy(Channels channel, uintptr_all_t src, uintptr_all_t dest, uint32_t size);

void Stall(Channels channel);

void StallForToken(AsyncToken token);
}