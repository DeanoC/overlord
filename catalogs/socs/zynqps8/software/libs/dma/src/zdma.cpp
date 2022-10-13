#include "core/core.h"
#include "platform/reg_access.h"
#include "platform/registers/zdma.h"
#include "dbg/assert.h"
#include "dbg/print.h"
#include "dbg/ansi_escapes.h"
#include "zdma.hpp"

#define ZDMA_TRANSFER_MAX_LEN (0x3FFFFFFFU - 7U)

namespace Dma::ZDma {
void SimpleDmaSet32(uintptr_lo_t baseAddr, const uint32_t data, uintptr_all_t address, uint32_t size) {
	assert(size < ZDMA_TRANSFER_MAX_LEN)

	// clear dma done status
	HW_REG_RMW( baseAddr, ZDMA, ZDMA_CH_ISR, ZDMA_ZDMA_CH_ISR_DMA_DONE, ZDMA_ZDMA_CH_ISR_DMA_DONE);

	// write only AKA Set mode, using 128 bits of data to set
	HW_REG_WRITE( baseAddr, ZDMA, ZDMA_CH_WR_ONLY_WORD0, data);
	HW_REG_WRITE( baseAddr, ZDMA, ZDMA_CH_WR_ONLY_WORD1, data);
	HW_REG_WRITE( baseAddr, ZDMA, ZDMA_CH_WR_ONLY_WORD2, data);
	HW_REG_WRITE( baseAddr, ZDMA, ZDMA_CH_WR_ONLY_WORD3, data);

	// Write Destination Address - 64 bit address
	HW_REG_WRITE( baseAddr, ZDMA, ZDMA_CH_DST_DSCR_WORD0, (uint32_t) (address & 0xFFFFFFFF));
	HW_REG_WRITE( baseAddr, ZDMA, ZDMA_CH_DST_DSCR_WORD1, (uint32_t) ((address >> 32U) & 0X0001FFFFU));

	// Size to be Transferred. Recommended to set both src and dest sizes
	HW_REG_WRITE( baseAddr, ZDMA, ZDMA_CH_SRC_DSCR_WORD2, size);
	HW_REG_WRITE( baseAddr, ZDMA, ZDMA_CH_DST_DSCR_WORD2, size);

	// setup control setting
	HW_REG_WRITE( baseAddr, ZDMA, ZDMA_CH_CTRL0,
	              HW_REG_ENCODE_FIELD(ZDMA, ZDMA_CH_CTRL0, OVR_FETCH, 1) |
	              HW_REG_ENCODE_FIELD(ZDMA, ZDMA_CH_CTRL0, POINT_TYPE, 0) |
	              HW_REG_ENCODE_FIELD(ZDMA, ZDMA_CH_CTRL0, MODE, ZDMA_ZDMA_CH_CTRL0_MODE_WRITE_ONLY) |
	              HW_REG_ENCODE_FIELD(ZDMA, ZDMA_CH_CTRL0, RATE_CTRL, 0) |
	              HW_REG_ENCODE_FIELD(ZDMA, ZDMA_CH_CTRL0, CONT_ADDR, 0) |
	              HW_REG_ENCODE_FIELD(ZDMA, ZDMA_CH_CTRL0, CONT, 1)
	);

	// trigger
	HW_REG_WRITE( baseAddr, ZDMA, ZDMA_CH_CTRL2, ZDMA_ZDMA_CH_CTRL2_EN);
	if (HW_REG_READ( baseAddr, ZDMA, ZDMA_CH_STATUS) == 0x3) {
		debug_print(ANSI_RED_PAPER ANSI_BLINK_ON "ZDMA ERROR after enable!" ANSI_RESET_ATTRIBUTES "\n");
	}
	return;
}

void SimpleDmaCopy(uintptr_lo_t baseAddr, uintptr_all_t src, uintptr_all_t dest, uint32_t size) {
	assert(size < ZDMA_TRANSFER_MAX_LEN)

	// clear dma done status
	HW_REG_RMW( baseAddr, ZDMA, ZDMA_CH_ISR, ZDMA_ZDMA_CH_ISR_DMA_DONE, ZDMA_ZDMA_CH_ISR_DMA_DONE);

	// Write Source - 64 bit address and size
	HW_REG_WRITE( baseAddr, ZDMA, ZDMA_CH_SRC_DSCR_WORD0, (uint32_t) (src & 0xFFFFFFFF));
	HW_REG_WRITE( baseAddr, ZDMA, ZDMA_CH_SRC_DSCR_WORD1, (uint32_t) ((src >> 32U) & 0X0001FFFFU));
	HW_REG_WRITE( baseAddr, ZDMA, ZDMA_CH_SRC_DSCR_WORD2, size);

	// Write Destination - 64 bit address and size
	HW_REG_WRITE( baseAddr, ZDMA, ZDMA_CH_DST_DSCR_WORD0, (uint32_t) (dest & 0xFFFFFFFF));
	HW_REG_WRITE( baseAddr, ZDMA, ZDMA_CH_DST_DSCR_WORD1, (uint32_t) ((dest >> 32U) & 0X0001FFFFU));
	HW_REG_WRITE( baseAddr, ZDMA, ZDMA_CH_DST_DSCR_WORD2, size);

	HW_REG_WRITE(baseAddr,ZDMA, ZDMA_CH_CTRL0,
	             HW_REG_ENCODE_FIELD(ZDMA, ZDMA_CH_CTRL0, OVR_FETCH, 1) |
	             HW_REG_ENCODE_FIELD(ZDMA, ZDMA_CH_CTRL0, POINT_TYPE, 0) |
	             HW_REG_ENCODE_FIELD(ZDMA, ZDMA_CH_CTRL0, MODE, ZDMA_ZDMA_CH_CTRL0_MODE_NORMAL) |
	             HW_REG_ENCODE_FIELD(ZDMA, ZDMA_CH_CTRL0, RATE_CTRL, 0) |
	             HW_REG_ENCODE_FIELD(ZDMA, ZDMA_CH_CTRL0, CONT_ADDR, 0));

	// trigger
	HW_REG_WRITE( baseAddr, ZDMA, ZDMA_CH_CTRL2, ZDMA_ZDMA_CH_CTRL2_EN);
	if (HW_REG_READ( baseAddr, ZDMA, ZDMA_CH_STATUS) == 0x3) {
		debug_print(ANSI_RED_PAPER ANSI_BLINK_ON "ZDMA ERROR after enable!" ANSI_RESET_ATTRIBUTES "\n");
	}
}

void Stall(uintptr_lo_t baseAddr) {
	// Wait until the DMA is in idle state
	uint32_t volatile RegVal;
	do {
		RegVal = HW_REG_DECODE_FIELD(ZDMA, ZDMA_CH_STATUS, STATE, HW_REG_READ(baseAddr, ZDMA, ZDMA_CH_STATUS));
	} while ((RegVal != 0U) && (RegVal != 3U)); // OK or ERR need enum support in overlord

	// clear dma done status
	HW_REG_RMW( baseAddr, ZDMA, ZDMA_CH_ISR, ZDMA_ZDMA_CH_ISR_DMA_DONE, ZDMA_ZDMA_CH_ISR_DMA_DONE);

	// Read the channel status for errors
	if (HW_REG_READ( baseAddr, ZDMA, ZDMA_CH_STATUS) == 0x3) {
		debug_print(ANSI_RED_PAPER ANSI_BLINK_ON "ZDMA ERROR after stall!" ANSI_RESET_ATTRIBUTES "\n");
	}
}

}