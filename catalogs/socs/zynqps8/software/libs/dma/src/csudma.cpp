#include "core/core.h"
#include "platform/memory_map.h"
#include "platform/reg_access.h"
#include "platform/registers/csu.h"
#include "platform/registers/csudma.h"
#include "dbg/assert.h"
#include "dbg/print.h"
#include "dbg/ansi_escapes.h"
#include "zynqps8/dma/csudma.hpp"
#include "utils/busy_sleep.h"


namespace Dma::CsuDma {

bool PcapInit() {
	HW_REG_WRITE1(CSU, PCAP_RESET, 1);
	Utils_BusyMilliSleep(1);
	HW_REG_WRITE1(CSU, PCAP_RESET, 0);

	// set to pcap write mode
	HW_REG_SET_FIELD1(CSU, PCAP_CTRL, PCAP_PR, 1);
	HW_REG_SET_FIELD1(CSU, PCAP_RDWR, PCAP_RDWR_B, 0);
	// TODO if powered down ask PMU to power the PL up
	debug_printf("PL power status %i\n", HW_REG_GET_BIT1(CSU, PCAP_STATUS, PL_GPWRDWN_B));

	// reset PL at least 250ns
	HW_REG_CLR_BIT1(CSU, PCAP_PROG, PCFG_PROG_B);
	while(!HW_REG_GET_BIT1(CSU, PCAP_STATUS, PL_INIT)) {}
	HW_REG_SET_BIT1(CSU, PCAP_PROG, PCFG_PROG_B);

	return true;
}
void TransferToPcap( uintptr_all_t src_, uint32_t sizeInBytes_ ) {
	assert((sizeInBytes_ & 0x3) == 0);

	// clear dma source status done bit (WTC)
	HW_REG_SET_BIT1(CSUDMA, CSUDMA_SRC_I_STS, DONE);

	// we don't support AES so stream source is DMA
	HW_REG_SET_FIELD1(CSU, CSU_SSS_CFG, PCAP_SSS, 0x5);

	// size will trigger dma
	HW_REG_WRITE1( CSUDMA, CSUDMA_SRC_ADDR, (uint32_t) (src_ & 0xFFFFFFFF));
	HW_REG_WRITE1( CSUDMA, CSUDMA_SRC_ADDR_MSB, (uint32_t) ((src_ >> 32U) & 0X0001FFFFU));
	HW_REG_WRITE1( CSUDMA, CSUDMA_SRC_SIZE, sizeInBytes_);
}

void StallForPcap() {
	// wait for the dma to be done
	while(HW_REG_GET_BIT1(CSUDMA, CSUDMA_SRC_I_STS, DONE)) {}
	// wait for the PL to be done
	while(HW_REG_GET_BIT1(CSU, PCAP_STATUS, PL_DONE)) {}

	HW_REG_WRITE1(CSU, PCAP_RESET, 1);
	Utils_BusyMilliSleep(1);
	HW_REG_WRITE1(CSU, PCAP_RESET, 0);

}

} // end namespace