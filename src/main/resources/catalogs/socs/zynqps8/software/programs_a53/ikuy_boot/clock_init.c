#include "core/core.h"
#include "hw/boot_psi.h"
#include "hw/reg_access.h"
#include "hw/memory_map.h"
#include "hw_regs/crl_apb.h"
#include "hw_regs/crf_apb.h"
#include "hw_regs/iou_slcr.h"
#include "hw_regs/fpd_slcr.h"
#include "hw_regs/lpd_slcr.h"

__attribute__((__section__(".hwregs")))
static PSI_IWord const clock_init[] = {
		PSI_SET_REGISTER_BANK(CRL_APB),

		PSI_WRITE_MASKED_32(CRL_APB, GEM3_REF_CTRL, 0x063F3F07U, 0x06010C00U),
		PSI_WRITE_MASKED_32(CRL_APB, GEM_TSU_REF_CTRL, 0x013F3F07U, 0x01010600U),
		PSI_WRITE_MASKED_32(CRL_APB, USB0_BUS_REF_CTRL, 0x023F3F07U, 0x02010600U),
		PSI_WRITE_MASKED_32(CRL_APB, USB3_DUAL_REF_CTRL, 0x023F3F07U, 0x02031900U),

		// QSPI, SDIO0, SDIO1 and UART0 share a mask
		PSI_MULTI_WRITE_MASKED_32(CRL_APB, QSPI_REF_CTRL, 4, 0x013F3F07U,
															HW_REG_ENCODE_FIELD(CRL_APB, QSPI_REF_CTRL, CLKACT, 1) |          // QSPI active
																	HW_REG_ENCODE_FIELD(CRL_APB, QSPI_REF_CTRL, DIVISOR1, 1) |    // /1
																	HW_REG_ENCODE_FIELD(CRL_APB, QSPI_REF_CTRL, DIVISOR0, 0x5) |  // / 5 = 300Mhz
																	HW_REG_ENCODE_FIELD(CRL_APB, QSPI_REF_CTRL, SRCSEL, 0),       // IOPLL (1500Mhz)
															HW_REG_ENCODE_FIELD(CRL_APB, SDIO0_REF_CTRL, CLKACT, 1) |         // SDIO 0 active
																	HW_REG_ENCODE_FIELD(CRL_APB, SDIO0_REF_CTRL, DIVISOR1, 1) |   // / 1
																	HW_REG_ENCODE_FIELD(CRL_APB, SDIO0_REF_CTRL, DIVISOR0, 0x4) | // / 4  = 200Mhz
																	HW_REG_ENCODE_FIELD(CRL_APB, SDIO0_REF_CTRL, SRCSEL, 2),      // RPLL (800Mhz)
															HW_REG_ENCODE_FIELD(CRL_APB, SDIO1_REF_CTRL, CLKACT, 1) |         // SDIO 1active
																	HW_REG_ENCODE_FIELD(CRL_APB, SDIO1_REF_CTRL, DIVISOR1, 1) |   // / 1
																	HW_REG_ENCODE_FIELD(CRL_APB, SDIO1_REF_CTRL, DIVISOR0, 0x4) | // / 4 = 200Mhz
																	HW_REG_ENCODE_FIELD(CRL_APB, SDIO1_REF_CTRL, SRCSEL, 2),      // RPLL (800Mhz)
															HW_REG_ENCODE_FIELD(CRL_APB, UART0_REF_CTRL, CLKACT, 1) |         // UART0 active
																	HW_REG_ENCODE_FIELD(CRL_APB, UART0_REF_CTRL, DIVISOR1, 1) |   //
																	HW_REG_ENCODE_FIELD(CRL_APB, UART0_REF_CTRL, DIVISOR0, 0xF) | //
																	HW_REG_ENCODE_FIELD(CRL_APB, UART0_REF_CTRL, SRCSEL, 0)       // IOPLL (1500Mhz)
		),

		// I2C 0 and 1 are the same
		PSI_WRITE_N_MASKED_32(CRL_APB, I2C0_REF_CTRL, 2, 0x013F3F07U,
													HW_REG_ENCODE_FIELD(CRL_APB, I2C0_REF_CTRL, CLKACT, 1) |        // I2C0/1 active
															HW_REG_ENCODE_FIELD(CRL_APB, I2C0_REF_CTRL, DIVISOR1, 1) |        // / 1
															HW_REG_ENCODE_FIELD(CRL_APB, I2C0_REF_CTRL, DIVISOR0, 0xF) |      // / 15 = 100Mhz
															HW_REG_ENCODE_FIELD(CRL_APB, I2C0_REF_CTRL, SRCSEL, 0)            // IOPLL (1500Mhz)
		),

		PSI_WRITE_MASKED_32(CRL_APB, CAN1_REF_CTRL, 0x013F3F07U, 0x01010F00U),

		PSI_WRITE_MASKED_32(CRL_APB, CPU_R5_CTRL, 0x01003F07U, 0x01000302U),
		PSI_WRITE_MASKED_32(CRL_APB, IOU_SWITCH_CTRL, 0x01003F07U, 0x01000300U),
		PSI_WRITE_MASKED_32(CRL_APB, PCAP_CTRL, 0x01003F07U, 0x01000800U),
		PSI_WRITE_MASKED_32(CRL_APB, LPD_SWITCH_CTRL, 0x01003F07U, 0x01000302U),

		PSI_WRITE_MASKED_32(CRL_APB, LPD_LSBUS_CTRL, 0x01003F07U,
												HW_REG_ENCODE_FIELD(CRL_APB, LPD_LSBUS_CTRL, CLKACT, 1) |     // LPB_LSBUS active
														HW_REG_ENCODE_FIELD(CRL_APB, LPD_LSBUS_CTRL, DIVISOR0, 0xF) |   // / 15 = 100Mhz
														HW_REG_ENCODE_FIELD(CRL_APB, LPD_LSBUS_CTRL, SRCSEL, 2)         // IOPLL (1500Mhz)
		),

		PSI_WRITE_MASKED_32(CRL_APB, DBG_LPD_CTRL, 0x01003F07U, 0x01000602U),
		PSI_WRITE_MASKED_32(CRL_APB, LPD_DMA_REF_CTRL, 0x01003F07U, 0x01000302U),
		PSI_WRITE_MASKED_32(CRL_APB, PL0_REF_CTRL, 0x013F3F07U, 0x01010802U),
		PSI_WRITE_MASKED_32(CRL_APB, PSSYSMON_REF_CTRL, 0x013F3F07U, 0x01011E02U),
		PSI_WRITE_MASKED_32(CRL_APB, DLL_REF_CTRL, 0x00000007U, 0x00000000U),
		PSI_WRITE_MASKED_32(CRL_APB, TIMESTAMP_REF_CTRL, 0x01003F07U, 0x01000104U),

		PSI_SET_REGISTER_BANK(CRF_APB),
		PSI_WRITE_MASKED_32(CRF_APB, PCIE_REF_CTRL, 0x01003F07U, 0x01000200U),
		PSI_WRITE_MASKED_32(CRF_APB, DP_VIDEO_REF_CTRL, 0x013F3F07U, 0x01010500U),
		PSI_WRITE_MASKED_32(CRF_APB, DP_AUDIO_REF_CTRL, 0x013F3F07U, 0x01011003U),
		PSI_WRITE_MASKED_32(CRF_APB, DP_STC_REF_CTRL, 0x013F3F07U, 0x01010F03U),
		PSI_WRITE_MASKED_32(CRF_APB, ACPU_CTRL, 0x03003F07U, 0x03000100U),
		PSI_WRITE_MASKED_32(CRF_APB, DBG_FPD_CTRL, 0x01003F07U, 0x01000200U),
		PSI_WRITE_MASKED_32(CRF_APB, DDR_CTRL, 0x00003F07U, 0x00000200U),
		PSI_WRITE_MASKED_32(CRF_APB, GPU_REF_CTRL, 0x07003F07U, 0x07000203U),
		PSI_WRITE_MASKED_32(CRF_APB, FPD_DMA_REF_CTRL, 0x01003F07U, 0x01000203U),
		PSI_WRITE_MASKED_32(CRF_APB, DPDMA_REF_CTRL, 0x01003F07U, 0x01000203U),
		PSI_WRITE_MASKED_32(CRF_APB, TOPSW_MAIN_CTRL, 0x01003F07U, 0x01000303U),
		PSI_WRITE_MASKED_32(CRF_APB, TOPSW_LSBUS_CTRL, 0x01003F07U, 0x01000502U),
		PSI_WRITE_MASKED_32(CRF_APB, DBG_TSTMP_CTRL, 0x00003F07U, 0x00000200U),

		PSI_SET_REGISTER_BANK(IOU_SLCR),
		PSI_WRITE_MASKED_32(IOU_SLCR, SDIO_CLK_CTRL, 0x00020003U, 0x00000000U),
		PSI_WRITE_MASKED_32(IOU_SLCR, WDT_CLK_SEL, 0x00000001U, 0x00000000U),
		PSI_WRITE_MASKED_32(IOU_SLCR, IOU_TTC_APB_CLK, 0x000000FFU, 0x00000000U),

		PSI_FAR_WRITE_MASKED_32(FPD_SLCR, WDT_CLK_SEL, 0x00000001U, 0x00000000U),
		PSI_FAR_WRITE_MASKED_32(LPD_SLCR, CSUPMU_WDT_CLK_SEL, 0x00000001U, 0x00000000U),

		PSI_END_PROGRAM
};

void clockRunInitProgram() {
	psi_run_register_program(clock_init);
}