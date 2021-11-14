#include "core/core.h"

#include "zynqps8/dma/lpddma.hpp"

#include "hw/memory_map.h"
#include "hw/reg_access.h"
#include "hw_regs/zdma.h"
#include "dbg/assert.h"
#include "dbg/raw_print.h"

#define ZDMA_REG(reg) ZDMA_##reg##_OFFSET
#define ZDMA_REG_FIELD_MASK(reg, field) ZDMA_##reg##_##field##_MASK
#define ZDMA_REG_FIELD_LSHIFT(reg, field) ZDMA_##reg##_##field##_LSHIFT
#define ZDMA_REG_ENCODE_FIELD(reg, field, value) (value << ZDMA_REG_FIELD_LSHIFT(reg, field))
#define ZDMA_REG_DECODE_FIELD(reg, field, value) ((value & (ZDMA_REG_FIELD_MASK(reg, field))) >> ZDMA_REG_FIELD_LSHIFT(reg, field))

#define ZDMA_TRANSFER_MAX_LEN (0x3FFFFFFFU - 7U)

namespace Dma::LpdDma {

AsyncToken SimpleDmaSet(const Channels channel, const uint32_t data, uintptr_all_t address, uint32_t size) {
	assert(size < ZDMA_TRANSFER_MAX_LEN)

	uintptr_lo_t baseAddr = LPD_DMA_CH0_BASE_ADDR;
	switch(channel) {
		case Channels::ChannelZero: 	baseAddr = LPD_DMA_CH0_BASE_ADDR; break;
		case Channels::ChannelOne:		baseAddr = LPD_DMA_CH1_BASE_ADDR; break;
		case Channels::ChannelTwo:		baseAddr = LPD_DMA_CH2_BASE_ADDR; break;
		case Channels::ChannelThree:	baseAddr = LPD_DMA_CH3_BASE_ADDR; break;
		case Channels::ChannelFour:		baseAddr = LPD_DMA_CH4_BASE_ADDR; break;
		case Channels::ChannelFive:		baseAddr = LPD_DMA_CH5_BASE_ADDR; break;
		case Channels::ChannelSix:		baseAddr = LPD_DMA_CH6_BASE_ADDR; break;
		case Channels::ChannelSevern:	baseAddr = LPD_DMA_CH7_BASE_ADDR; break;
	}

	// write only AKA Set mode, using 128 bits of data to set
	hw_RegWrite(baseAddr, ZDMA_REG(ZDMA_CH_WR_ONLY_WORD0), data);
	hw_RegWrite(baseAddr, ZDMA_REG(ZDMA_CH_WR_ONLY_WORD1), data);
	hw_RegWrite(baseAddr, ZDMA_REG(ZDMA_CH_WR_ONLY_WORD2), data);
	hw_RegWrite(baseAddr, ZDMA_REG(ZDMA_CH_WR_ONLY_WORD3), data);

	// Write Destination Address - 64 bit address
	hw_RegWrite(baseAddr, ZDMA_REG(ZDMA_CH_DST_DSCR_WORD0), (uint32_t)(address & 0xFFFFFFFF));
	hw_RegWrite(baseAddr, ZDMA_REG(ZDMA_CH_DST_DSCR_WORD1), (uint32_t)((address >> 32U) & 0X0001FFFFU));

	// Size to be Transferred. Recommended to set both src and dest sizes
	hw_RegWrite(baseAddr, ZDMA_REG(ZDMA_CH_SRC_DSCR_WORD2), size);
	hw_RegWrite(baseAddr, ZDMA_REG(ZDMA_CH_DST_DSCR_WORD2), size);

	hw_RegWrite(baseAddr, ZDMA_REG(ZDMA_CH_CTRL0),
							ZDMA_REG_ENCODE_FIELD(ZDMA_CH_CTRL0, OVR_FETCH, 1) |
							ZDMA_REG_ENCODE_FIELD(ZDMA_CH_CTRL0, POINT_TYPE, 0) |
							ZDMA_REG_ENCODE_FIELD(ZDMA_CH_CTRL0, MODE, ZDMA_ZDMA_CH_CTRL0_MODE_WRITE_ONLY) |
							ZDMA_REG_ENCODE_FIELD(ZDMA_CH_CTRL0, RATE_CTRL, 0) |
							ZDMA_REG_ENCODE_FIELD(ZDMA_CH_CTRL0, CONT_ADDR, 0) |
							ZDMA_REG_ENCODE_FIELD(ZDMA_CH_CTRL0, CONT, 1) );


	return channel;
}
AsyncToken SimpleDmaSet(const Channels channel, const uint8_t data, uintptr_all_t address, uint32_t size) {
	const uint32_t data32 = (const uint32_t) data << 24 |
			(const uint32_t) data << 16 |
			(const uint32_t) data << 8 |
			(const uint32_t) data << 0;

	return SimpleDmaSet(channel, data32, address,size);
}

AsyncToken SimpleDmaCopy(const Channels channel, uintptr_all_t src, uintptr_all_t dest, uint32_t size) {
	assert(size < ZDMA_TRANSFER_MAX_LEN)

	uintptr_lo_t baseAddr = LPD_DMA_CH0_BASE_ADDR;
	switch(channel) {
		case Channels::ChannelZero: 	baseAddr = LPD_DMA_CH0_BASE_ADDR; break;
		case Channels::ChannelOne:		baseAddr = LPD_DMA_CH1_BASE_ADDR; break;
		case Channels::ChannelTwo:		baseAddr = LPD_DMA_CH2_BASE_ADDR; break;
		case Channels::ChannelThree:	baseAddr = LPD_DMA_CH3_BASE_ADDR; break;
		case Channels::ChannelFour:		baseAddr = LPD_DMA_CH4_BASE_ADDR; break;
		case Channels::ChannelFive:		baseAddr = LPD_DMA_CH5_BASE_ADDR; break;
		case Channels::ChannelSix:		baseAddr = LPD_DMA_CH6_BASE_ADDR; break;
		case Channels::ChannelSevern:	baseAddr = LPD_DMA_CH7_BASE_ADDR; break;
	}
	// clear dma done status
	hw_RegWrite(baseAddr, ZDMA_REG(ZDMA_CH_ISR),
							hw_RegRead(baseAddr, ZDMA_REG(ZDMA_CH_ISR)) & ~ZDMA_ZDMA_CH_ISR_DMA_DONE);

	// Write Source Address - 64 bit address
	hw_RegWrite(baseAddr, ZDMA_REG(ZDMA_CH_SRC_DSCR_WORD0), (uint32_t)(src & 0xFFFFFFFF));
	hw_RegWrite(baseAddr, ZDMA_REG(ZDMA_CH_SRC_DSCR_WORD1), (uint32_t)((src >> 32U) & 0X0001FFFFU));
	hw_RegWrite(baseAddr, ZDMA_REG(ZDMA_CH_SRC_DSCR_WORD2), size);

	// Write Destination Address - 64 bit address
	hw_RegWrite(baseAddr, ZDMA_REG(ZDMA_CH_DST_DSCR_WORD0), (uint32_t)(dest & 0xFFFFFFFF));
	hw_RegWrite(baseAddr, ZDMA_REG(ZDMA_CH_DST_DSCR_WORD1), (uint32_t)((dest >> 32U) & 0X0001FFFFU));
	hw_RegWrite(baseAddr, ZDMA_REG(ZDMA_CH_DST_DSCR_WORD2), size);

	hw_RegWrite(baseAddr, ZDMA_REG(ZDMA_CH_CTRL0),
							ZDMA_REG_ENCODE_FIELD(ZDMA_CH_CTRL0, OVR_FETCH, 1) |
							ZDMA_REG_ENCODE_FIELD(ZDMA_CH_CTRL0, POINT_TYPE, 0) |
							ZDMA_REG_ENCODE_FIELD(ZDMA_CH_CTRL0, MODE, ZDMA_ZDMA_CH_CTRL0_MODE_NORMAL) |
							ZDMA_REG_ENCODE_FIELD(ZDMA_CH_CTRL0, RATE_CTRL, 0) |
							ZDMA_REG_ENCODE_FIELD(ZDMA_CH_CTRL0, CONT_ADDR, 0) );

	hw_RegWrite(baseAddr, ZDMA_REG(ZDMA_CH_CTRL2), ZDMA_ZDMA_CH_CTRL2_EN);

	if (hw_RegRead(baseAddr, ZDMA_REG(ZDMA_CH_STATUS)) == 0x3) {
		raw_debug_print("ZDMA ERROR after enable!\n");
		return channel;
	}
	return channel;
}

void Stall(Channels channel) {
	StallForToken((AsyncToken)channel);
}

void StallForToken(AsyncToken token) {
	uintptr_lo_t baseAddr = LPD_DMA_CH0_BASE_ADDR;
	uint32_t volatile RegVal;
	switch(token) {
		case Channels::ChannelZero: 	baseAddr = LPD_DMA_CH0_BASE_ADDR; break;
		case Channels::ChannelOne:		baseAddr = LPD_DMA_CH1_BASE_ADDR; break;
		case Channels::ChannelTwo:		baseAddr = LPD_DMA_CH2_BASE_ADDR; break;
		case Channels::ChannelThree:	baseAddr = LPD_DMA_CH3_BASE_ADDR; break;
		case Channels::ChannelFour:		baseAddr = LPD_DMA_CH4_BASE_ADDR; break;
		case Channels::ChannelFive:		baseAddr = LPD_DMA_CH5_BASE_ADDR; break;
		case Channels::ChannelSix:		baseAddr = LPD_DMA_CH6_BASE_ADDR; break;
		case Channels::ChannelSevern:	baseAddr = LPD_DMA_CH7_BASE_ADDR; break;
	}

	// Wait until the DMA is in idle state
	do {
		RegVal = ZDMA_REG_DECODE_FIELD(ZDMA_CH_STATUS, STATE, hw_RegRead(baseAddr,ZDMA_REG(ZDMA_CH_STATUS)));
	} while ((RegVal != 0U) && (RegVal != 3U)); // OK or ERR need enum support in overlord

	hw_RegWrite(baseAddr, ZDMA_REG(ZDMA_CH_ISR),hw_RegRead(baseAddr, ZDMA_REG(ZDMA_CH_ISR)) & ~ ZDMA_ZDMA_CH_ISR_DMA_DONE);
	// Read the channel status for errors
	if (hw_RegRead(baseAddr, ZDMA_REG(ZDMA_CH_STATUS)) == 0x3) {
		raw_debug_print("ZDMA Error!\n");
		return;
	}
}

} // end namespace
