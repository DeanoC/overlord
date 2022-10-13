#include "core/core.h"
#include "platform/reg_access.h"
#include "utils/boot_psi.h"
#include "platform/memory_map.h"
#include "platform/registers/crl_apb.h"
#include "platform/registers/crf_apb.h"
#include "platform/registers/iou_slcr.h"
#include "platform/registers/fpd_slcr.h"
#include "platform/registers/lpd_slcr.h"

__attribute__((__section__(".hwregs")))
static PSI_IWord const clock_init[] = {
	PSI_SET_REGISTER_BANK( CRL_APB ),

	PSI_WRITE_MASKED_32( CRL_APB, GEM3_REF_CTRL,                                      // GEM3_REF_CTRL
	                     HW_REG_FIELD_MASK( CRL_APB, GEM3_REF_CTRL, RX_CLKACT ) |
	                     HW_REG_FIELD_MASK( CRL_APB, GEM3_REF_CTRL, CLKACT ) |
	                     HW_REG_FIELD_MASK( CRL_APB, GEM3_REF_CTRL, DIVISOR1 ) |
	                     HW_REG_FIELD_MASK( CRL_APB, GEM3_REF_CTRL, DIVISOR0 ) |
	                     HW_REG_FIELD_MASK( CRL_APB, GEM3_REF_CTRL, SRCSEL ),
	                     HW_REG_ENCODE_FIELD( CRL_APB, GEM3_REF_CTRL, RX_CLKACT, 1 ) |
	                     HW_REG_ENCODE_FIELD( CRL_APB, GEM3_REF_CTRL, CLKACT, 1 ) |
	                     HW_REG_ENCODE_FIELD( CRL_APB, GEM3_REF_CTRL, DIVISOR1, 1 ) |      // / 1
	                     HW_REG_ENCODE_FIELD( CRL_APB, GEM3_REF_CTRL, DIVISOR0, 12 ) |     // / 12 = 125Mhz
	                     HW_REG_ENCODE_ENUM( CRL_APB, GEM3_REF_CTRL, SRCSEL, IOPLL )       // IOPLL (1500Mhz)
	),
	PSI_WRITE_MASKED_32( CRL_APB, GEM_TSU_REF_CTRL,                                   // GEM_TSU_REF_CTRL
	                     HW_REG_FIELD_MASK( CRL_APB, GEM_TSU_REF_CTRL, CLKACT ) |
	                     HW_REG_FIELD_MASK( CRL_APB, GEM_TSU_REF_CTRL, DIVISOR1 ) |
	                     HW_REG_FIELD_MASK( CRL_APB, GEM_TSU_REF_CTRL, DIVISOR0 ) |
	                     HW_REG_FIELD_MASK( CRL_APB, GEM_TSU_REF_CTRL, SRCSEL ),
	                     HW_REG_ENCODE_FIELD( CRL_APB, GEM_TSU_REF_CTRL, CLKACT, 1 ) |
	                     HW_REG_ENCODE_FIELD( CRL_APB, GEM_TSU_REF_CTRL, DIVISOR1, 1 ) |   // / 1
	                     HW_REG_ENCODE_FIELD( CRL_APB, GEM_TSU_REF_CTRL, DIVISOR0, 6 ) |   // / 6 = 250Mhz
	                     HW_REG_ENCODE_ENUM( CRL_APB, GEM_TSU_REF_CTRL, SRCSEL, IOPLL )    // IOPLL (1500Mhz)
	),
	PSI_WRITE_MASKED_32( CRL_APB, USB0_BUS_REF_CTRL,                                   // USB0_BUS_REF_CTRL
	                     HW_REG_FIELD_MASK( CRL_APB, USB0_BUS_REF_CTRL, CLKACT ) |
	                     HW_REG_FIELD_MASK( CRL_APB, USB0_BUS_REF_CTRL, DIVISOR1 ) |
	                     HW_REG_FIELD_MASK( CRL_APB, USB0_BUS_REF_CTRL, DIVISOR0 ) |
	                     HW_REG_FIELD_MASK( CRL_APB, USB0_BUS_REF_CTRL, SRCSEL ),
	                     HW_REG_ENCODE_FIELD( CRL_APB, USB0_BUS_REF_CTRL, CLKACT, 1 ) |
	                     HW_REG_ENCODE_FIELD( CRL_APB, USB0_BUS_REF_CTRL, DIVISOR1, 1 ) |  // / 1
	                     HW_REG_ENCODE_FIELD( CRL_APB, USB0_BUS_REF_CTRL, DIVISOR0, 6 ) |  // / 6 = 250Mhz
	                     HW_REG_ENCODE_ENUM( CRL_APB, USB0_BUS_REF_CTRL, SRCSEL, IOPLL )   // IOPLL (1500Mhz)
	),

	PSI_WRITE_MASKED_32( CRL_APB, USB3_DUAL_REF_CTRL,                                            // USB3_DUAL_REF_CTRL
	                     HW_REG_FIELD_MASK( CRL_APB, USB3_DUAL_REF_CTRL, CLKACT ) |
	                     HW_REG_FIELD_MASK( CRL_APB, USB3_DUAL_REF_CTRL, DIVISOR1 ) |
	                     HW_REG_FIELD_MASK( CRL_APB, USB3_DUAL_REF_CTRL, DIVISOR0 ) |
	                     HW_REG_FIELD_MASK( CRL_APB, USB3_DUAL_REF_CTRL, SRCSEL ),
	                     HW_REG_ENCODE_FIELD( CRL_APB, USB3_DUAL_REF_CTRL, CLKACT, 1 ) |
	                     HW_REG_ENCODE_FIELD( CRL_APB, USB3_DUAL_REF_CTRL, DIVISOR1, 3 ) |     // / 3
	                     HW_REG_ENCODE_FIELD( CRL_APB, USB3_DUAL_REF_CTRL, DIVISOR0, 25 ) |    // / (3*25) = 20Mhz
	                     HW_REG_ENCODE_ENUM( CRL_APB, USB3_DUAL_REF_CTRL, SRCSEL, IOPLL )      // IOPLL (1500Mhz)
	),

	// QSPI, SDIO0, SDIO1 and UART0, UART1 share a mask and are continuous
	PSI_MULTI_WRITE_MASKED_32( CRL_APB, QSPI_REF_CTRL, 5,
	                           HW_REG_FIELD_MASK( CRL_APB, QSPI_REF_CTRL, CLKACT ) |
	                           HW_REG_FIELD_MASK( CRL_APB, QSPI_REF_CTRL, DIVISOR1 ) |
	                           HW_REG_FIELD_MASK( CRL_APB, QSPI_REF_CTRL, DIVISOR0 ) |
	                           HW_REG_FIELD_MASK( CRL_APB, QSPI_REF_CTRL, SRCSEL ),
	                           HW_REG_ENCODE_FIELD( CRL_APB, QSPI_REF_CTRL, CLKACT, 1 ) |      // QSPI active
	                           HW_REG_ENCODE_FIELD( CRL_APB, QSPI_REF_CTRL, DIVISOR1, 1 ) |    // / 1
	                           HW_REG_ENCODE_FIELD( CRL_APB, QSPI_REF_CTRL, DIVISOR0, 12 ) |    // / 12 = 125 Mhz
	                           HW_REG_ENCODE_ENUM( CRL_APB, QSPI_REF_CTRL, SRCSEL, IOPLL ),    // IOPLL (1500Mhz)
	                           HW_REG_ENCODE_FIELD( CRL_APB, SDIO0_REF_CTRL, CLKACT, 0 ) |     // SDIO 0 inactive
	                           HW_REG_ENCODE_FIELD( CRL_APB, SDIO0_REF_CTRL, DIVISOR1, 1 ) |   // / 1
	                           HW_REG_ENCODE_FIELD( CRL_APB, SDIO0_REF_CTRL, DIVISOR0, 8 ) |  // / 8  = 187.5 Mhz
	                           HW_REG_ENCODE_ENUM( CRL_APB, SDIO0_REF_CTRL, SRCSEL, IOPLL ),   // IOPLL (1500Mhz)
	                           HW_REG_ENCODE_FIELD( CRL_APB, SDIO1_REF_CTRL, CLKACT, 1 ) |     // SDIO 1 active
	                           HW_REG_ENCODE_FIELD( CRL_APB, SDIO1_REF_CTRL, DIVISOR1, 1 ) |   // / 1
	                           HW_REG_ENCODE_FIELD( CRL_APB, SDIO1_REF_CTRL, DIVISOR0, 8 ) |  // / 8 = 187.5 Mhz
	                           HW_REG_ENCODE_ENUM( CRL_APB, SDIO1_REF_CTRL, SRCSEL, IOPLL ),   // IOPLL (1500Mhz)
	                           HW_REG_ENCODE_FIELD( CRL_APB, UART0_REF_CTRL, CLKACT, 1 ) |     // UART0 active
	                           HW_REG_ENCODE_FIELD( CRL_APB, UART0_REF_CTRL, DIVISOR1, 1 ) |   //
	                           HW_REG_ENCODE_FIELD( CRL_APB, UART0_REF_CTRL, DIVISOR0, 15 ) |  // / 15 = 100Mhz
                            HW_REG_ENCODE_ENUM( CRL_APB, UART0_REF_CTRL, SRCSEL, IOPLL ),    // IOPLL (1500Mhz)
	                           HW_REG_ENCODE_FIELD( CRL_APB, UART1_REF_CTRL, CLKACT, 1 ) |     // UART0 active
	                           HW_REG_ENCODE_FIELD( CRL_APB, UART1_REF_CTRL, DIVISOR1, 1 ) |   //
	                           HW_REG_ENCODE_FIELD( CRL_APB, UART1_REF_CTRL, DIVISOR0, 15 ) |  // / 15 = 100Mhz
	                           HW_REG_ENCODE_ENUM( CRL_APB, UART1_REF_CTRL, SRCSEL, IOPLL )    // IOPLL (1500Mhz)
	),

	// I2C 1 only
	PSI_WRITE_MASKED_32( CRL_APB, I2C1_REF_CTRL,
	                     HW_REG_FIELD_MASK( CRL_APB, I2C1_REF_CTRL, CLKACT ) |
	                     HW_REG_FIELD_MASK( CRL_APB, I2C1_REF_CTRL, DIVISOR1 ) |
	                     HW_REG_FIELD_MASK( CRL_APB, I2C1_REF_CTRL, DIVISOR0 ) |
	                     HW_REG_FIELD_MASK( CRL_APB, I2C1_REF_CTRL, SRCSEL ),
	                     HW_REG_ENCODE_FIELD( CRL_APB, I2C1_REF_CTRL, CLKACT, 1 ) |      // I2C1 active
	                     HW_REG_ENCODE_FIELD( CRL_APB, I2C1_REF_CTRL, DIVISOR1, 1 ) |    // / 1
	                     HW_REG_ENCODE_FIELD( CRL_APB, I2C1_REF_CTRL, DIVISOR0, 15 ) |   // / 15 = 100Mhz
	                     HW_REG_ENCODE_ENUM( CRL_APB, I2C1_REF_CTRL, SRCSEL, IOPLL )     // IOPLL (1500Mhz)
	),
	// SPI 1 only
	PSI_WRITE_MASKED_32( CRL_APB, SPI1_REF_CTRL,
	                     HW_REG_FIELD_MASK( CRL_APB, SPI1_REF_CTRL, CLKACT ) |
	                     HW_REG_FIELD_MASK( CRL_APB, SPI1_REF_CTRL, DIVISOR1 ) |
	                     HW_REG_FIELD_MASK( CRL_APB, SPI1_REF_CTRL, DIVISOR0 ) |
	                     HW_REG_FIELD_MASK( CRL_APB, SPI1_REF_CTRL, SRCSEL ),
	                     HW_REG_ENCODE_FIELD( CRL_APB, SPI1_REF_CTRL, CLKACT, 1 ) |      // I2C1 active
	                     HW_REG_ENCODE_FIELD( CRL_APB, SPI1_REF_CTRL, DIVISOR1, 1 ) |    // / 1
	                     HW_REG_ENCODE_FIELD( CRL_APB, SPI1_REF_CTRL, DIVISOR0, 8 ) |    // / 8 = 187.5Mhz
	                     HW_REG_ENCODE_ENUM( CRL_APB, SPI1_REF_CTRL, SRCSEL, IOPLL )     // IOPLL (1500Mhz)
	),

	PSI_WRITE_MASKED_32( CRL_APB, CPU_R5_CTRL,                                            // CPU_R5_CTRL
	                     HW_REG_FIELD_MASK( CRL_APB, CPU_R5_CTRL, CLKACT_CORE ) |
	                     HW_REG_FIELD_MASK( CRL_APB, CPU_R5_CTRL, CLKACT ) |
	                     HW_REG_FIELD_MASK( CRL_APB, CPU_R5_CTRL, DIVISOR0 ) |
	                     HW_REG_FIELD_MASK( CRL_APB, CPU_R5_CTRL, SRCSEL ),
	                     HW_REG_ENCODE_FIELD( CRL_APB, CPU_R5_CTRL, CLKACT_CORE, 1 ) |
	                     HW_REG_ENCODE_FIELD( CRL_APB, CPU_R5_CTRL, CLKACT, 1 ) |
	                     HW_REG_ENCODE_FIELD( CRL_APB, CPU_R5_CTRL, DIVISOR0, 2 ) |        // / 2 = 533Mhz
	                     HW_REG_ENCODE_ENUM( CRL_APB, CPU_R5_CTRL, SRCSEL, RPLL )          // RPLL (1064Mhz)
	),
	PSI_WRITE_MASKED_32( CRL_APB, IOU_SWITCH_CTRL,                                        // IOU_SWITCH_CTRL
	                     HW_REG_FIELD_MASK( CRL_APB, IOU_SWITCH_CTRL, CLKACT ) |
	                     HW_REG_FIELD_MASK( CRL_APB, IOU_SWITCH_CTRL, DIVISOR0 ) |
	                     HW_REG_FIELD_MASK( CRL_APB, IOU_SWITCH_CTRL, SRCSEL ),
	                     HW_REG_ENCODE_FIELD( CRL_APB, IOU_SWITCH_CTRL, CLKACT, 1 ) |
	                     HW_REG_ENCODE_FIELD( CRL_APB, IOU_SWITCH_CTRL, DIVISOR0, 6 ) |  // / 3 = 250 Mhz
	                     HW_REG_ENCODE_ENUM( CRL_APB, IOU_SWITCH_CTRL, SRCSEL, IOPLL )   // IOPLL (1500Mhz)
	),
	PSI_WRITE_MASKED_32( CRL_APB, PCAP_CTRL,                                                // PCAP_CTRL
	                     HW_REG_FIELD_MASK( CRL_APB, PCAP_CTRL, CLKACT ) |
	                     HW_REG_FIELD_MASK( CRL_APB, PCAP_CTRL, DIVISOR0 ) |
	                     HW_REG_FIELD_MASK( CRL_APB, PCAP_CTRL, SRCSEL ),
	                     HW_REG_ENCODE_FIELD( CRL_APB, PCAP_CTRL, CLKACT, 1 ) |
	                     HW_REG_ENCODE_FIELD( CRL_APB, PCAP_CTRL, DIVISOR0, 8 ) |            // / 8 = 187.5Mhz
	                     HW_REG_ENCODE_ENUM( CRL_APB, PCAP_CTRL, SRCSEL, IOPLL )              // IOPLL (1500Mhz)
	),
	PSI_WRITE_MASKED_32( CRL_APB, LPD_SWITCH_CTRL,                                          // LPD_SWITCH_CTRL
	                     HW_REG_FIELD_MASK( CRL_APB, LPD_SWITCH_CTRL, CLKACT ) |
	                     HW_REG_FIELD_MASK( CRL_APB, LPD_SWITCH_CTRL, DIVISOR0 ) |
	                     HW_REG_FIELD_MASK( CRL_APB, LPD_SWITCH_CTRL, SRCSEL ),
	                     HW_REG_ENCODE_FIELD( CRL_APB, LPD_SWITCH_CTRL, CLKACT, 1 ) |
	                     HW_REG_ENCODE_FIELD( CRL_APB, LPD_SWITCH_CTRL, DIVISOR0, 3 ) |      // / 3 = 500Mhz
	                     HW_REG_ENCODE_ENUM( CRL_APB, LPD_SWITCH_CTRL, SRCSEL, IOPLL )       // IOPLL (1500Mhz)
	),

	PSI_WRITE_MASKED_32( CRL_APB, LPD_LSBUS_CTRL,                                        // LPD_LSBUS_CTRL
	                     HW_REG_FIELD_MASK( CRL_APB, LPD_LSBUS_CTRL, CLKACT ) |
	                     HW_REG_FIELD_MASK( CRL_APB, LPD_LSBUS_CTRL, DIVISOR0 ) |
	                     HW_REG_FIELD_MASK( CRL_APB, LPD_LSBUS_CTRL, SRCSEL ),
	                     HW_REG_ENCODE_FIELD( CRL_APB, LPD_LSBUS_CTRL, CLKACT, 1 ) |
	                     HW_REG_ENCODE_FIELD( CRL_APB, LPD_LSBUS_CTRL, DIVISOR0, 15 ) |    // / 15 = 100Mhz
	                     HW_REG_ENCODE_ENUM( CRL_APB, LPD_LSBUS_CTRL, SRCSEL, IOPLL )      // IOPLL (1500Mhz)
	),

	PSI_WRITE_MASKED_32( CRL_APB, DBG_LPD_CTRL,                                          // DBG_LPD_CTRL
	                     HW_REG_FIELD_MASK( CRL_APB, DBG_LPD_CTRL, CLKACT ) |
	                     HW_REG_FIELD_MASK( CRL_APB, DBG_LPD_CTRL, DIVISOR0 ) |
	                     HW_REG_FIELD_MASK( CRL_APB, DBG_LPD_CTRL, SRCSEL ),
	                     HW_REG_ENCODE_FIELD( CRL_APB, DBG_LPD_CTRL, CLKACT, 1 ) |
	                     HW_REG_ENCODE_FIELD( CRL_APB, DBG_LPD_CTRL, DIVISOR0, 6 ) |        // / 6 = 250Mhz
	                     HW_REG_ENCODE_ENUM( CRL_APB, DBG_LPD_CTRL, SRCSEL, IOPLL )        // IOPLL (1500Mhz)
	),
	PSI_WRITE_MASKED_32( CRL_APB, LPD_DMA_REF_CTRL,                                      // LPD_DMA_REF_CTRL
	                     HW_REG_FIELD_MASK( CRL_APB, LPD_DMA_REF_CTRL, CLKACT ) |
	                     HW_REG_FIELD_MASK( CRL_APB, LPD_DMA_REF_CTRL, DIVISOR0 ) |
	                     HW_REG_FIELD_MASK( CRL_APB, LPD_DMA_REF_CTRL, SRCSEL ),
	                     HW_REG_ENCODE_FIELD( CRL_APB, LPD_DMA_REF_CTRL, CLKACT, 1 ) |
	                     HW_REG_ENCODE_FIELD( CRL_APB, LPD_DMA_REF_CTRL, DIVISOR0, 3 ) |    // / 3 = 500Mhz
	                     HW_REG_ENCODE_ENUM( CRL_APB, LPD_DMA_REF_CTRL, SRCSEL, IOPLL )    // IOPLL (1500Mhz)
	),
	// PL 0 and 1 clock
	PSI_WRITE_N_MASKED_32( CRL_APB, PL0_REF_CTRL, 2,                                      // PL[0|1]_REF_CTRL
	                       HW_REG_FIELD_MASK( CRL_APB, PL0_REF_CTRL, CLKACT ) |
	                       HW_REG_FIELD_MASK( CRL_APB, PL0_REF_CTRL, DIVISOR1 ) |
	                       HW_REG_FIELD_MASK( CRL_APB, PL0_REF_CTRL, DIVISOR0 ) |
	                       HW_REG_FIELD_MASK( CRL_APB, PL0_REF_CTRL, SRCSEL ),
	                       HW_REG_ENCODE_FIELD( CRL_APB, PL0_REF_CTRL, CLKACT, 1 ) |
	                       HW_REG_ENCODE_FIELD( CRL_APB, PL0_REF_CTRL, DIVISOR1, 1 ) |
	                       HW_REG_ENCODE_FIELD( CRL_APB, PL0_REF_CTRL, DIVISOR0, 15 ) |      // / 15 = 100Mhz
	                       HW_REG_ENCODE_ENUM( CRL_APB, PL0_REF_CTRL, SRCSEL, IOPLL )        // IOPLL (1500Mhz)
	),

	PSI_WRITE_MASKED_32( CRL_APB, PSSYSMON_REF_CTRL,                                      // PSSYSMON_REF_CTRL
	                     HW_REG_FIELD_MASK( CRL_APB, PSSYSMON_REF_CTRL, CLKACT ) |
	                     HW_REG_FIELD_MASK( CRL_APB, PSSYSMON_REF_CTRL, DIVISOR1 ) |
	                     HW_REG_FIELD_MASK( CRL_APB, PSSYSMON_REF_CTRL, DIVISOR0 ) |
	                     HW_REG_FIELD_MASK( CRL_APB, PSSYSMON_REF_CTRL, SRCSEL ),
	                     HW_REG_ENCODE_FIELD( CRL_APB, PSSYSMON_REF_CTRL, CLKACT, 1 ) |
	                     HW_REG_ENCODE_FIELD( CRL_APB, PSSYSMON_REF_CTRL, DIVISOR1, 1 ) |
	                     HW_REG_ENCODE_FIELD( CRL_APB, PSSYSMON_REF_CTRL, DIVISOR0, 30 ) |    // / 30 = 50Mhz
	                     HW_REG_ENCODE_ENUM( CRL_APB, PSSYSMON_REF_CTRL, SRCSEL, IOPLL )     // IOPLL (1500Mhz)
	),
	PSI_WRITE_MASKED_32( CRL_APB, DLL_REF_CTRL,                                            // DLL_REF_CTRL
	                     HW_REG_FIELD_MASK( CRL_APB, DLL_REF_CTRL, SRCSEL ),
	                     HW_REG_ENCODE_ENUM( CRL_APB, DLL_REF_CTRL, SRCSEL, IOPLL )          // IOPLL (1500Mhz)
	),

	PSI_WRITE_MASKED_32( CRL_APB, TIMESTAMP_REF_CTRL,                                      // TIMESTAMP_REF_CTRL
	                     HW_REG_FIELD_MASK( CRL_APB, TIMESTAMP_REF_CTRL, CLKACT ) |
	                     HW_REG_FIELD_MASK( CRL_APB, TIMESTAMP_REF_CTRL, DIVISOR0 ) |
	                     HW_REG_FIELD_MASK( CRL_APB, TIMESTAMP_REF_CTRL, SRCSEL ),
	                     HW_REG_ENCODE_FIELD( CRL_APB, TIMESTAMP_REF_CTRL, CLKACT, 1 ) |
	                     HW_REG_ENCODE_FIELD( CRL_APB, TIMESTAMP_REF_CTRL, DIVISOR0, 15 ) |  // / 15 = 100Mhz
	                     HW_REG_ENCODE_ENUM( CRL_APB, TIMESTAMP_REF_CTRL, SRCSEL, IOPLL )    // IOPLL (1500Mhz)
	),
	PSI_SET_REGISTER_BANK( CRF_APB ),
	PSI_WRITE_MASKED_32( CRF_APB, DP_VIDEO_REF_CTRL,                                          // DP_VIDEO_REF_CTRL
	                     HW_REG_FIELD_MASK( CRF_APB, DP_VIDEO_REF_CTRL, CLKACT ) |
	                     HW_REG_FIELD_MASK( CRF_APB, DP_VIDEO_REF_CTRL, DIVISOR1 ) |
	                     HW_REG_FIELD_MASK( CRF_APB, DP_VIDEO_REF_CTRL, DIVISOR0 ) |
	                     HW_REG_FIELD_MASK( CRF_APB, DP_VIDEO_REF_CTRL, SRCSEL ),
	                     HW_REG_ENCODE_FIELD( CRF_APB, DP_VIDEO_REF_CTRL, CLKACT, 1 ) |
	                     HW_REG_ENCODE_FIELD( CRF_APB, DP_VIDEO_REF_CTRL, DIVISOR1, 1 ) |
	                     HW_REG_ENCODE_FIELD( CRF_APB, DP_VIDEO_REF_CTRL, DIVISOR0, 5 ) |      // / 5 = 300Mhz
	                     HW_REG_ENCODE_ENUM( CRF_APB, DP_VIDEO_REF_CTRL, SRCSEL, VPLL )        // VPLL (1500Mhz)
	),
	PSI_WRITE_MASKED_32( CRF_APB, DP_AUDIO_REF_CTRL,                                          // DP_AUDIO_REF_CTRL
	                     HW_REG_FIELD_MASK( CRF_APB, DP_AUDIO_REF_CTRL, CLKACT ) |
	                     HW_REG_FIELD_MASK( CRF_APB, DP_AUDIO_REF_CTRL, DIVISOR1 ) |
	                     HW_REG_FIELD_MASK( CRF_APB, DP_AUDIO_REF_CTRL, DIVISOR0 ) |
	                     HW_REG_FIELD_MASK( CRF_APB, DP_AUDIO_REF_CTRL, SRCSEL ),
	                     HW_REG_ENCODE_FIELD( CRF_APB, DP_AUDIO_REF_CTRL, CLKACT, 1 ) |
	                     HW_REG_ENCODE_FIELD( CRF_APB, DP_AUDIO_REF_CTRL, DIVISOR1, 1 ) |
	                     HW_REG_ENCODE_FIELD( CRF_APB, DP_AUDIO_REF_CTRL, DIVISOR0, 22 ) |    // / 22 = 24.18Mhz
	                     HW_REG_ENCODE_ENUM( CRF_APB, DP_AUDIO_REF_CTRL, SRCSEL, RPLL_TO_FPD ) // RPLL_TO_FPD (532Mhz)
	),
	PSI_WRITE_MASKED_32( CRF_APB, DP_STC_REF_CTRL,                                            // DP_STC_REF_CTRL
	                     HW_REG_FIELD_MASK( CRF_APB, DP_STC_REF_CTRL, CLKACT ) |
	                     HW_REG_FIELD_MASK( CRF_APB, DP_STC_REF_CTRL, DIVISOR1 ) |
	                     HW_REG_FIELD_MASK( CRF_APB, DP_STC_REF_CTRL, DIVISOR0 ) |
	                     HW_REG_FIELD_MASK( CRF_APB, DP_STC_REF_CTRL, SRCSEL ),
	                     HW_REG_ENCODE_FIELD( CRF_APB, DP_STC_REF_CTRL, CLKACT, 1 ) |
	                     HW_REG_ENCODE_FIELD( CRF_APB, DP_STC_REF_CTRL, DIVISOR1, 1 ) |
	                     HW_REG_ENCODE_FIELD( CRF_APB, DP_STC_REF_CTRL, DIVISOR0, 20 ) |      // / 20 = 26.6Mhz
	                     HW_REG_ENCODE_ENUM( CRF_APB, DP_STC_REF_CTRL, SRCSEL, RPLL_TO_FPD )   // RPLL_TO_FPD (532Mhz)
	),
	PSI_WRITE_MASKED_32( CRF_APB, ACPU_CTRL,                                                  //ACPU_CTRL
	                     HW_REG_FIELD_MASK( CRF_APB, ACPU_CTRL, CLKACT_HALF ) |
	                     HW_REG_FIELD_MASK( CRF_APB, ACPU_CTRL, CLKACT_FULL ) |
	                     HW_REG_FIELD_MASK( CRF_APB, ACPU_CTRL, DIVISOR0 ) |
	                     HW_REG_FIELD_MASK( CRF_APB, ACPU_CTRL, SRCSEL ),
	                     HW_REG_ENCODE_FIELD( CRF_APB, ACPU_CTRL, CLKACT_HALF, 1 ) |
	                     HW_REG_ENCODE_FIELD( CRF_APB, ACPU_CTRL, CLKACT_FULL, 1 ) |
	                     HW_REG_ENCODE_FIELD( CRF_APB, ACPU_CTRL, DIVISOR0, 1 ) |            // / 1 = 1332Mhz
	                     HW_REG_ENCODE_ENUM( CRF_APB, ACPU_CTRL, SRCSEL, APLL )              // APLL (1332Mhz)
	),
	PSI_WRITE_MASKED_32( CRF_APB, DBG_FPD_CTRL,                                            // DBG_FPD_CTRL
	                     HW_REG_FIELD_MASK( CRF_APB, DBG_FPD_CTRL, CLKACT ) |
	                     HW_REG_FIELD_MASK( CRF_APB, DBG_FPD_CTRL, DIVISOR0 ) |
	                     HW_REG_FIELD_MASK( CRF_APB, DBG_FPD_CTRL, SRCSEL ),
	                     HW_REG_ENCODE_FIELD( CRF_APB, DBG_FPD_CTRL, CLKACT, 1 ) |
	                     HW_REG_ENCODE_FIELD( CRF_APB, DBG_FPD_CTRL, DIVISOR0, 2 ) |        // / 2 = 250Mhz
	                     HW_REG_ENCODE_ENUM( CRF_APB, DBG_FPD_CTRL, SRCSEL, IOPLL_TO_FPD )   // IOPLL_TO_FPD (500Mhz)
	),
	PSI_WRITE_MASKED_32( CRF_APB, DDR_CTRL,                                                // DDR_CTRL
	                     HW_REG_FIELD_MASK( CRF_APB, DDR_CTRL, DIVISOR0 ) |
	                     HW_REG_FIELD_MASK( CRF_APB, DDR_CTRL, SRCSEL ),
	                     HW_REG_ENCODE_FIELD( CRF_APB, DDR_CTRL, DIVISOR0, 2 ) |            // / 2 = 600Mhz
	                     HW_REG_ENCODE_ENUM( CRF_APB, DDR_CTRL, SRCSEL, DPLL )              // DPLL (1200Mhz)
	),
	PSI_WRITE_MASKED_32( CRF_APB, GPU_REF_CTRL,                                            // GPU_REF_CTRL
	                     HW_REG_FIELD_MASK( CRF_APB, GPU_REF_CTRL, PP0_CLKACT ) |
	                     HW_REG_FIELD_MASK( CRF_APB, GPU_REF_CTRL, PP1_CLKACT ) |
	                     HW_REG_FIELD_MASK( CRF_APB, GPU_REF_CTRL, CLKACT ) |
	                     HW_REG_FIELD_MASK( CRF_APB, GPU_REF_CTRL, DIVISOR0 ) |
	                     HW_REG_FIELD_MASK( CRF_APB, GPU_REF_CTRL, SRCSEL ),
	                     HW_REG_ENCODE_FIELD( CRF_APB, GPU_REF_CTRL, PP0_CLKACT, 1 ) |
	                     HW_REG_ENCODE_FIELD( CRF_APB, GPU_REF_CTRL, PP1_CLKACT, 1 ) |
	                     HW_REG_ENCODE_FIELD( CRF_APB, GPU_REF_CTRL, CLKACT, 1 ) |
	                     HW_REG_ENCODE_FIELD( CRF_APB, GPU_REF_CTRL, DIVISOR0, 1 ) |      // / 1 = 500Mhz
	                     HW_REG_ENCODE_ENUM( CRF_APB, GPU_REF_CTRL, SRCSEL, IOPLL_TO_FPD ) // IOPLL_TO_FPD (500Mhz)
	),
	PSI_WRITE_MASKED_32( CRF_APB, FPD_DMA_REF_CTRL,                                      // FPD_DMA_REF_CTRL
	                     HW_REG_FIELD_MASK( CRF_APB, FPD_DMA_REF_CTRL, CLKACT ) |
	                     HW_REG_FIELD_MASK( CRF_APB, FPD_DMA_REF_CTRL, DIVISOR0 ) |
	                     HW_REG_FIELD_MASK( CRF_APB, FPD_DMA_REF_CTRL, SRCSEL ),
	                     HW_REG_ENCODE_FIELD( CRF_APB, FPD_DMA_REF_CTRL, CLKACT, 1 ) |
	                     HW_REG_ENCODE_FIELD( CRF_APB, FPD_DMA_REF_CTRL, DIVISOR0, 2 ) | // / 2 = 533Mhz
	                     HW_REG_ENCODE_ENUM( CRF_APB, FPD_DMA_REF_CTRL, SRCSEL, DPLL )   // DPLL (1066Mhz)
	),
	PSI_WRITE_MASKED_32( CRF_APB, DPDMA_REF_CTRL,                                      // DPDMA_REF_CTRL
	                     HW_REG_FIELD_MASK( CRF_APB, DPDMA_REF_CTRL, CLKACT ) |
	                     HW_REG_FIELD_MASK( CRF_APB, DPDMA_REF_CTRL, DIVISOR0 ) |
	                     HW_REG_FIELD_MASK( CRF_APB, DPDMA_REF_CTRL, SRCSEL ),
	                     HW_REG_ENCODE_FIELD( CRF_APB, DPDMA_REF_CTRL, CLKACT, 1 ) |
	                     HW_REG_ENCODE_FIELD( CRF_APB, DPDMA_REF_CTRL, DIVISOR0, 3 ) |  // / 3 = 444Mhz
	                     HW_REG_ENCODE_ENUM( CRF_APB, DPDMA_REF_CTRL, SRCSEL, APLL )     // APLL (1332Mhz)
	),
	PSI_WRITE_MASKED_32( CRF_APB, TOPSW_MAIN_CTRL,                                      // TOPSW_MAIN_CTRL FPD AXI clock
	                     HW_REG_FIELD_MASK( CRF_APB, TOPSW_MAIN_CTRL, CLKACT ) |
	                     HW_REG_FIELD_MASK( CRF_APB, TOPSW_MAIN_CTRL, DIVISOR0 ) |
	                     HW_REG_FIELD_MASK( CRF_APB, TOPSW_MAIN_CTRL, SRCSEL ),
	                     HW_REG_ENCODE_FIELD( CRF_APB, TOPSW_MAIN_CTRL, CLKACT, 1 ) |
	                     HW_REG_ENCODE_FIELD( CRF_APB, TOPSW_MAIN_CTRL, DIVISOR0, 2 ) |  // / 3 = 533Mhz
	                     HW_REG_ENCODE_ENUM( CRF_APB, TOPSW_MAIN_CTRL, SRCSEL, DPLL )    // DPLL (1066Mhz)
	),
	PSI_WRITE_MASKED_32( CRF_APB, TOPSW_LSBUS_CTRL,                                    // TOPSW_LSBUS FPD APB Clock
	                     HW_REG_FIELD_MASK( CRF_APB, TOPSW_LSBUS_CTRL, CLKACT ) |
	                     HW_REG_FIELD_MASK( CRF_APB, TOPSW_LSBUS_CTRL, DIVISOR0 ) |
	                     HW_REG_FIELD_MASK( CRF_APB, TOPSW_LSBUS_CTRL, SRCSEL ),
	                     HW_REG_ENCODE_FIELD( CRF_APB, TOPSW_LSBUS_CTRL, CLKACT, 1 ) |
	                     HW_REG_ENCODE_FIELD( CRF_APB, TOPSW_LSBUS_CTRL, DIVISOR0, 5 ) |      // / 5 = 100Mhz
	                     HW_REG_ENCODE_ENUM( CRF_APB, TOPSW_LSBUS_CTRL, SRCSEL, IOPLL_TO_FPD ) // IOPLL_TO_FPD (500Mhz)
	),
	PSI_WRITE_MASKED_32( CRF_APB, DBG_TSTMP_CTRL,                                            // DBG_TSTMP_CTRL
	                     HW_REG_FIELD_MASK( CRF_APB, DBG_TSTMP_CTRL, DIVISOR0 ) |
	                     HW_REG_FIELD_MASK( CRF_APB, DBG_TSTMP_CTRL, SRCSEL ),
	                     HW_REG_ENCODE_FIELD( CRF_APB, DBG_TSTMP_CTRL, DIVISOR0, 2 ) |        // / 2 = 250Mhz
	                     HW_REG_ENCODE_ENUM( CRF_APB, DBG_TSTMP_CTRL, SRCSEL, IOPLL_TO_FPD )  // IOPLL_TO_FPD (500Mhz)
	),
	PSI_SET_REGISTER_BANK( IOU_SLCR ),
	PSI_WRITE_MASKED_32( IOU_SLCR, IOU_TTC_APB_CLK,
	                     HW_REG_FIELD_MASK( IOU_SLCR, IOU_TTC_APB_CLK, TTC0_SEL ) |
	                     HW_REG_FIELD_MASK( IOU_SLCR, IOU_TTC_APB_CLK, TTC1_SEL ) |
	                     HW_REG_FIELD_MASK( IOU_SLCR, IOU_TTC_APB_CLK, TTC2_SEL ) |
	                     HW_REG_FIELD_MASK( IOU_SLCR, IOU_TTC_APB_CLK, TTC3_SEL ),

	                     HW_REG_ENCODE_ENUM( IOU_SLCR, IOU_TTC_APB_CLK, TTC0_SEL, APB_SWITCH_CLK ) |
	                     HW_REG_ENCODE_ENUM( IOU_SLCR, IOU_TTC_APB_CLK, TTC1_SEL, APB_SWITCH_CLK ) |
	                     HW_REG_ENCODE_ENUM( IOU_SLCR, IOU_TTC_APB_CLK, TTC2_SEL, APB_SWITCH_CLK ) |
	                     HW_REG_ENCODE_ENUM( IOU_SLCR, IOU_TTC_APB_CLK, TTC3_SEL, APB_SWITCH_CLK )
	),
	PSI_WRITE_MASKED_32( IOU_SLCR, WDT_CLK_SEL,
	                     HW_REG_FIELD_MASK( IOU_SLCR, WDT_CLK_SEL, SELECT ),
	                     HW_REG_ENCODE_ENUM( IOU_SLCR, WDT_CLK_SEL, SELECT, APB_CLOCK )
	),

	PSI_WRITE_MASKED_32( IOU_SLCR, SDIO_CLK_CTRL,
	                     HW_REG_FIELD_MASK( IOU_SLCR, SDIO_CLK_CTRL, SDIO1_RX_SRC_SEL ),
	                     HW_REG_ENCODE_FIELD( IOU_SLCR, SDIO_CLK_CTRL, SDIO1_RX_SRC_SEL, 0 )
	),

	PSI_FAR_WRITE_MASKED_32( FPD_SLCR, WDT_CLK_SEL,
	                         HW_REG_FIELD_MASK( FPD_SLCR, WDT_CLK_SEL, SELECT ),
	                         HW_REG_ENCODE_ENUM( FPD_SLCR, WDT_CLK_SEL, SELECT, APB_CLOCK )
	),
	PSI_FAR_WRITE_MASKED_32( LPD_SLCR, CSUPMU_WDT_CLK_SEL,
	                         HW_REG_FIELD_MASK( LPD_SLCR, CSUPMU_WDT_CLK_SEL, SELECT ),
	                         HW_REG_ENCODE_ENUM( LPD_SLCR, CSUPMU_WDT_CLK_SEL, SELECT, APB_CLOCK )
	),

	PSI_END_PROGRAM
};
void clockRunInitProgram() {
	psi_RunRegisterProgram(clock_init);
}

#if 0
#include "psu_init.h"

unsigned long psu_clock_init_data( void ) {
	/*
	* CLOCK CONTROL SLCR REGISTER
	*/
	/*
	* Register : GEM3_REF_CTRL @ 0XFF5E005C

	* Clock active for the RX channel
	*  PSU_CRL_APB_GEM3_REF_CTRL_RX_CLKACT                         0x1

	* Clock active signal. Switch to 0 to disable the clock
	*  PSU_CRL_APB_GEM3_REF_CTRL_CLKACT                            0x1

	* 6 bit divider
	*  PSU_CRL_APB_GEM3_REF_CTRL_DIVISOR1                          0x1

	* 6 bit divider
	*  PSU_CRL_APB_GEM3_REF_CTRL_DIVISOR0                          0xc

	* 000 = IOPLL; 010 = RPLL; 011 = DPLL; (This signal may only be toggled af
	* ter 4 cycles of the old clock and 4 cycles of the new clock. This is not
	*  usually an issue, but designers must be aware.)
	*  PSU_CRL_APB_GEM3_REF_CTRL_SRCSEL                            0x0

	* This register controls this reference clock
	* (OFFSET, MASK, VALUE)      (0XFF5E005C, 0x063F3F07U ,0x06010C00U)
	*/
	PSU_Mask_Write( CRL_APB_GEM3_REF_CTRL_OFFSET,
	                0x063F3F07U, 0x06010C00U );
	/*##################################################################### */

	/*
	* Register : GEM_TSU_REF_CTRL @ 0XFF5E0100

	* 6 bit divider
	*  PSU_CRL_APB_GEM_TSU_REF_CTRL_DIVISOR0                       0x6

	* 000 = IOPLL; 010 = RPLL; 011 = DPLL; (This signal may only be toggled af
	* ter 4 cycles of the old clock and 4 cycles of the new clock. This is not
	*  usually an issue, but designers must be aware.)
	*  PSU_CRL_APB_GEM_TSU_REF_CTRL_SRCSEL                         0x0

	* 6 bit divider
	*  PSU_CRL_APB_GEM_TSU_REF_CTRL_DIVISOR1                       0x1

	* Clock active signal. Switch to 0 to disable the clock
	*  PSU_CRL_APB_GEM_TSU_REF_CTRL_CLKACT                         0x1

	* This register controls this reference clock
	* (OFFSET, MASK, VALUE)      (0XFF5E0100, 0x013F3F07U ,0x01010600U)
	*/
	PSU_Mask_Write( CRL_APB_GEM_TSU_REF_CTRL_OFFSET,
	                0x013F3F07U, 0x01010600U );
	/*##################################################################### */

	/*
	* Register : USB0_BUS_REF_CTRL @ 0XFF5E0060

	* Clock active signal. Switch to 0 to disable the clock
	*  PSU_CRL_APB_USB0_BUS_REF_CTRL_CLKACT                        0x1

	* 6 bit divider
	*  PSU_CRL_APB_USB0_BUS_REF_CTRL_DIVISOR1                      0x1

	* 6 bit divider
	*  PSU_CRL_APB_USB0_BUS_REF_CTRL_DIVISOR0                      0x6

	* 000 = IOPLL; 010 = RPLL; 011 = DPLL; (This signal may only be toggled af
	* ter 4 cycles of the old clock and 4 cycles of the new clock. This is not
	*  usually an issue, but designers must be aware.)
	*  PSU_CRL_APB_USB0_BUS_REF_CTRL_SRCSEL                        0x0

	* This register controls this reference clock
	* (OFFSET, MASK, VALUE)      (0XFF5E0060, 0x023F3F07U ,0x02010600U)
	*/
	PSU_Mask_Write( CRL_APB_USB0_BUS_REF_CTRL_OFFSET,
	                0x023F3F07U, 0x02010600U );
	/*##################################################################### */

	/*
	* Register : USB3_DUAL_REF_CTRL @ 0XFF5E004C

	* Clock active signal. Switch to 0 to disable the clock
	*  PSU_CRL_APB_USB3_DUAL_REF_CTRL_CLKACT                       0x1

	* 6 bit divider
	*  PSU_CRL_APB_USB3_DUAL_REF_CTRL_DIVISOR1                     0x3

	* 6 bit divider
	*  PSU_CRL_APB_USB3_DUAL_REF_CTRL_DIVISOR0                     0x19

	* 000 = IOPLL; 010 = RPLL; 011 = DPLL. (This signal may only be toggled af
	* ter 4 cycles of the old clock and 4 cycles of the new clock. This is not
	*  usually an issue, but designers must be aware.)
	*  PSU_CRL_APB_USB3_DUAL_REF_CTRL_SRCSEL                       0x0

	* This register controls this reference clock
	* (OFFSET, MASK, VALUE)      (0XFF5E004C, 0x023F3F07U ,0x02031900U)
	*/
	PSU_Mask_Write( CRL_APB_USB3_DUAL_REF_CTRL_OFFSET,
	                0x023F3F07U, 0x02031900U );
	/*##################################################################### */

	/*
	* Register : QSPI_REF_CTRL @ 0XFF5E0068

	* Clock active signal. Switch to 0 to disable the clock
	*  PSU_CRL_APB_QSPI_REF_CTRL_CLKACT                            0x1

	* 6 bit divider
	*  PSU_CRL_APB_QSPI_REF_CTRL_DIVISOR1                          0x1

	* 6 bit divider
	*  PSU_CRL_APB_QSPI_REF_CTRL_DIVISOR0                          0xc

	* 000 = IOPLL; 010 = RPLL; 011 = DPLL; (This signal may only be toggled af
	* ter 4 cycles of the old clock and 4 cycles of the new clock. This is not
	*  usually an issue, but designers must be aware.)
	*  PSU_CRL_APB_QSPI_REF_CTRL_SRCSEL                            0x0

	* This register controls this reference clock
	* (OFFSET, MASK, VALUE)      (0XFF5E0068, 0x013F3F07U ,0x01010C00U)
	*/
	PSU_Mask_Write( CRL_APB_QSPI_REF_CTRL_OFFSET,
	                0x013F3F07U, 0x01010C00U );
	/*##################################################################### */

	/*
	* Register : SDIO1_REF_CTRL @ 0XFF5E0070

	* Clock active signal. Switch to 0 to disable the clock
	*  PSU_CRL_APB_SDIO1_REF_CTRL_CLKACT                           0x1

	* 6 bit divider
	*  PSU_CRL_APB_SDIO1_REF_CTRL_DIVISOR1                         0x1

	* 6 bit divider
	*  PSU_CRL_APB_SDIO1_REF_CTRL_DIVISOR0                         0x8

	* 000 = IOPLL; 010 = RPLL; 011 = VPLL; (This signal may only be toggled af
	* ter 4 cycles of the old clock and 4 cycles of the new clock. This is not
	*  usually an issue, but designers must be aware.)
	*  PSU_CRL_APB_SDIO1_REF_CTRL_SRCSEL                           0x0

	* This register controls this reference clock
	* (OFFSET, MASK, VALUE)      (0XFF5E0070, 0x013F3F07U ,0x01010800U)
	*/
	PSU_Mask_Write( CRL_APB_SDIO1_REF_CTRL_OFFSET,
	                0x013F3F07U, 0x01010800U );
	/*##################################################################### */

	/*
	* Register : SDIO_CLK_CTRL @ 0XFF18030C

	* MIO pad selection for sdio1_rx_clk (feedback clock from the PAD) 0: MIO
	* [51] 1: MIO [76]
	*  PSU_IOU_SLCR_SDIO_CLK_CTRL_SDIO1_RX_SRC_SEL                 0

	* SoC Debug Clock Control
	* (OFFSET, MASK, VALUE)      (0XFF18030C, 0x00020000U ,0x00000000U)
	*/
	PSU_Mask_Write( IOU_SLCR_SDIO_CLK_CTRL_OFFSET,
	                0x00020000U, 0x00000000U );
	/*##################################################################### */

	/*
	* Register : UART1_REF_CTRL @ 0XFF5E0078

	* Clock active signal. Switch to 0 to disable the clock
	*  PSU_CRL_APB_UART1_REF_CTRL_CLKACT                           0x1

	* 6 bit divider
	*  PSU_CRL_APB_UART1_REF_CTRL_DIVISOR1                         0x1

	* 6 bit divider
	*  PSU_CRL_APB_UART1_REF_CTRL_DIVISOR0                         0xf

	* 000 = IOPLL; 010 = RPLL; 011 = DPLL; (This signal may only be toggled af
	* ter 4 cycles of the old clock and 4 cycles of the new clock. This is not
	*  usually an issue, but designers must be aware.)
	*  PSU_CRL_APB_UART1_REF_CTRL_SRCSEL                           0x0

	* This register controls this reference clock
	* (OFFSET, MASK, VALUE)      (0XFF5E0078, 0x013F3F07U ,0x01010F00U)
	*/
	PSU_Mask_Write( CRL_APB_UART1_REF_CTRL_OFFSET,
	                0x013F3F07U, 0x01010F00U );
	/*##################################################################### */

	/*
	* Register : I2C1_REF_CTRL @ 0XFF5E0124

	* Clock active signal. Switch to 0 to disable the clock
	*  PSU_CRL_APB_I2C1_REF_CTRL_CLKACT                            0x1

	* 6 bit divider
	*  PSU_CRL_APB_I2C1_REF_CTRL_DIVISOR1                          0x1

	* 6 bit divider
	*  PSU_CRL_APB_I2C1_REF_CTRL_DIVISOR0                          0xf

	* 000 = IOPLL; 010 = RPLL; 011 = DPLL; (This signal may only be toggled af
	* ter 4 cycles of the old clock and 4 cycles of the new clock. This is not
	*  usually an issue, but designers must be aware.)
	*  PSU_CRL_APB_I2C1_REF_CTRL_SRCSEL                            0x0

	* This register controls this reference clock
	* (OFFSET, MASK, VALUE)      (0XFF5E0124, 0x013F3F07U ,0x01010F00U)
	*/
	PSU_Mask_Write( CRL_APB_I2C1_REF_CTRL_OFFSET,
	                0x013F3F07U, 0x01010F00U );
	/*##################################################################### */

	/*
	* Register : SPI1_REF_CTRL @ 0XFF5E0080

	* Clock active signal. Switch to 0 to disable the clock
	*  PSU_CRL_APB_SPI1_REF_CTRL_CLKACT                            0x1

	* 6 bit divider
	*  PSU_CRL_APB_SPI1_REF_CTRL_DIVISOR1                          0x1

	* 6 bit divider
	*  PSU_CRL_APB_SPI1_REF_CTRL_DIVISOR0                          0x8

	* 000 = IOPLL; 010 = RPLL; 011 = DPLL; (This signal may only be toggled af
	* ter 4 cycles of the old clock and 4 cycles of the new clock. This is not
	*  usually an issue, but designers must be aware.)
	*  PSU_CRL_APB_SPI1_REF_CTRL_SRCSEL                            0x0

	* This register controls this reference clock
	* (OFFSET, MASK, VALUE)      (0XFF5E0080, 0x013F3F07U ,0x01010800U)
	*/
	PSU_Mask_Write( CRL_APB_SPI1_REF_CTRL_OFFSET,
	                0x013F3F07U, 0x01010800U );
	/*##################################################################### */

	/*
	* Register : CPU_R5_CTRL @ 0XFF5E0090

	* Turing this off will shut down the OCM, some parts of the APM, and preve
	* nt transactions going from the FPD to the LPD and could lead to system h
	* ang
	*  PSU_CRL_APB_CPU_R5_CTRL_CLKACT                              0x1

	* 6 bit divider
	*  PSU_CRL_APB_CPU_R5_CTRL_DIVISOR0                            0x2

	* 000 = RPLL; 010 = IOPLL; 011 = DPLL; (This signal may only be toggled af
	* ter 4 cycles of the old clock and 4 cycles of the new clock. This is not
	*  usually an issue, but designers must be aware.)
	*  PSU_CRL_APB_CPU_R5_CTRL_SRCSEL                              0x0

	* This register controls this reference clock
	* (OFFSET, MASK, VALUE)      (0XFF5E0090, 0x01003F07U ,0x01000200U)
	*/
	PSU_Mask_Write( CRL_APB_CPU_R5_CTRL_OFFSET, 0x01003F07U, 0x01000200U );
	/*##################################################################### */

	/*
	* Register : IOU_SWITCH_CTRL @ 0XFF5E009C

	* Clock active signal. Switch to 0 to disable the clock
	*  PSU_CRL_APB_IOU_SWITCH_CTRL_CLKACT                          0x1

	* 6 bit divider
	*  PSU_CRL_APB_IOU_SWITCH_CTRL_DIVISOR0                        0x6

	* 000 = RPLL; 010 = IOPLL; 011 = DPLL; (This signal may only be toggled af
	* ter 4 cycles of the old clock and 4 cycles of the new clock. This is not
	*  usually an issue, but designers must be aware.)
	*  PSU_CRL_APB_IOU_SWITCH_CTRL_SRCSEL                          0x2

	* This register controls this reference clock
	* (OFFSET, MASK, VALUE)      (0XFF5E009C, 0x01003F07U ,0x01000602U)
	*/
	PSU_Mask_Write( CRL_APB_IOU_SWITCH_CTRL_OFFSET,
	                0x01003F07U, 0x01000602U );
	/*##################################################################### */

	/*
	* Register : PCAP_CTRL @ 0XFF5E00A4

	* Clock active signal. Switch to 0 to disable the clock
	*  PSU_CRL_APB_PCAP_CTRL_CLKACT                                0x1

	* 6 bit divider
	*  PSU_CRL_APB_PCAP_CTRL_DIVISOR0                              0x8

	* 000 = IOPLL; 010 = RPLL; 011 = DPLL; (This signal may only be toggled af
	* ter 4 cycles of the old clock and 4 cycles of the new clock. This is not
	*  usually an issue, but designers must be aware.)
	*  PSU_CRL_APB_PCAP_CTRL_SRCSEL                                0x0

	* This register controls this reference clock
	* (OFFSET, MASK, VALUE)      (0XFF5E00A4, 0x01003F07U ,0x01000800U)
	*/
	PSU_Mask_Write( CRL_APB_PCAP_CTRL_OFFSET, 0x01003F07U, 0x01000800U );
	/*##################################################################### */

	/*
	* Register : LPD_SWITCH_CTRL @ 0XFF5E00A8

	* Clock active signal. Switch to 0 to disable the clock
	*  PSU_CRL_APB_LPD_SWITCH_CTRL_CLKACT                          0x1

	* 6 bit divider
	*  PSU_CRL_APB_LPD_SWITCH_CTRL_DIVISOR0                        0x3

	* 000 = RPLL; 010 = IOPLL; 011 = DPLL; (This signal may only be toggled af
	* ter 4 cycles of the old clock and 4 cycles of the new clock. This is not
	*  usually an issue, but designers must be aware.)
	*  PSU_CRL_APB_LPD_SWITCH_CTRL_SRCSEL                          0x2

	* This register controls this reference clock
	* (OFFSET, MASK, VALUE)      (0XFF5E00A8, 0x01003F07U ,0x01000302U)
	*/
	PSU_Mask_Write( CRL_APB_LPD_SWITCH_CTRL_OFFSET,
	                0x01003F07U, 0x01000302U );
	/*##################################################################### */

	/*
	* Register : LPD_LSBUS_CTRL @ 0XFF5E00AC

	* Clock active signal. Switch to 0 to disable the clock
	*  PSU_CRL_APB_LPD_LSBUS_CTRL_CLKACT                           0x1

	* 6 bit divider
	*  PSU_CRL_APB_LPD_LSBUS_CTRL_DIVISOR0                         0xf

	* 000 = RPLL; 010 = IOPLL; 011 = DPLL; (This signal may only be toggled af
	* ter 4 cycles of the old clock and 4 cycles of the new clock. This is not
	*  usually an issue, but designers must be aware.)
	*  PSU_CRL_APB_LPD_LSBUS_CTRL_SRCSEL                           0x2

	* This register controls this reference clock
	* (OFFSET, MASK, VALUE)      (0XFF5E00AC, 0x01003F07U ,0x01000F02U)
	*/
	PSU_Mask_Write( CRL_APB_LPD_LSBUS_CTRL_OFFSET,
	                0x01003F07U, 0x01000F02U );
	/*##################################################################### */

	/*
	* Register : DBG_LPD_CTRL @ 0XFF5E00B0

	* Clock active signal. Switch to 0 to disable the clock
	*  PSU_CRL_APB_DBG_LPD_CTRL_CLKACT                             0x1

	* 6 bit divider
	*  PSU_CRL_APB_DBG_LPD_CTRL_DIVISOR0                           0x6

	* 000 = RPLL; 010 = IOPLL; 011 = DPLL; (This signal may only be toggled af
	* ter 4 cycles of the old clock and 4 cycles of the new clock. This is not
	*  usually an issue, but designers must be aware.)
	*  PSU_CRL_APB_DBG_LPD_CTRL_SRCSEL                             0x2

	* This register controls this reference clock
	* (OFFSET, MASK, VALUE)      (0XFF5E00B0, 0x01003F07U ,0x01000602U)
	*/
	PSU_Mask_Write( CRL_APB_DBG_LPD_CTRL_OFFSET,
	                0x01003F07U, 0x01000602U );
	/*##################################################################### */

	/*
	* Register : ADMA_REF_CTRL @ 0XFF5E00B8

	* Clock active signal. Switch to 0 to disable the clock
	*  PSU_CRL_APB_ADMA_REF_CTRL_CLKACT                            0x1

	* 6 bit divider
	*  PSU_CRL_APB_ADMA_REF_CTRL_DIVISOR0                          0x3

	* 000 = RPLL; 010 = IOPLL; 011 = DPLL; (This signal may only be toggled af
	* ter 4 cycles of the old clock and 4 cycles of the new clock. This is not
	*  usually an issue, but designers must be aware.)
	*  PSU_CRL_APB_ADMA_REF_CTRL_SRCSEL                            0x2

	* This register controls this reference clock
	* (OFFSET, MASK, VALUE)      (0XFF5E00B8, 0x01003F07U ,0x01000302U)
	*/
	PSU_Mask_Write( CRL_APB_ADMA_REF_CTRL_OFFSET,
	                0x01003F07U, 0x01000302U );
	/*##################################################################### */

	/*
	* Register : PL0_REF_CTRL @ 0XFF5E00C0

	* Clock active signal. Switch to 0 to disable the clock
	*  PSU_CRL_APB_PL0_REF_CTRL_CLKACT                             0x1

	* 6 bit divider
	*  PSU_CRL_APB_PL0_REF_CTRL_DIVISOR1                           0x1

	* 6 bit divider
	*  PSU_CRL_APB_PL0_REF_CTRL_DIVISOR0                           0xf

	* 000 = IOPLL; 010 = RPLL; 011 = DPLL; (This signal may only be toggled af
	* ter 4 cycles of the old clock and 4 cycles of the new clock. This is not
	*  usually an issue, but designers must be aware.)
	*  PSU_CRL_APB_PL0_REF_CTRL_SRCSEL                             0x0

	* This register controls this reference clock
	* (OFFSET, MASK, VALUE)      (0XFF5E00C0, 0x013F3F07U ,0x01010F00U)
	*/
	PSU_Mask_Write( CRL_APB_PL0_REF_CTRL_OFFSET,
	                0x013F3F07U, 0x01010F00U );
	/*##################################################################### */

	/*
	* Register : PL1_REF_CTRL @ 0XFF5E00C4

	* Clock active signal. Switch to 0 to disable the clock
	*  PSU_CRL_APB_PL1_REF_CTRL_CLKACT                             0x1

	* 6 bit divider
	*  PSU_CRL_APB_PL1_REF_CTRL_DIVISOR1                           0x1

	* 6 bit divider
	*  PSU_CRL_APB_PL1_REF_CTRL_DIVISOR0                           0xf

	* 000 = IOPLL; 010 = RPLL; 011 = DPLL; (This signal may only be toggled af
	* ter 4 cycles of the old clock and 4 cycles of the new clock. This is not
	*  usually an issue, but designers must be aware.)
	*  PSU_CRL_APB_PL1_REF_CTRL_SRCSEL                             0x0

	* This register controls this reference clock
	* (OFFSET, MASK, VALUE)      (0XFF5E00C4, 0x013F3F07U ,0x01010F00U)
	*/
	PSU_Mask_Write( CRL_APB_PL1_REF_CTRL_OFFSET,
	                0x013F3F07U, 0x01010F00U );
	/*##################################################################### */

	/*
	* Register : AMS_REF_CTRL @ 0XFF5E0108

	* 6 bit divider
	*  PSU_CRL_APB_AMS_REF_CTRL_DIVISOR1                           0x1

	* 6 bit divider
	*  PSU_CRL_APB_AMS_REF_CTRL_DIVISOR0                           0x1e

	* 000 = RPLL; 010 = IOPLL; 011 = DPLL; (This signal may only be toggled af
	* ter 4 cycles of the old clock and 4 cycles of the new clock. This is not
	*  usually an issue, but designers must be aware.)
	*  PSU_CRL_APB_AMS_REF_CTRL_SRCSEL                             0x2

	* Clock active signal. Switch to 0 to disable the clock
	*  PSU_CRL_APB_AMS_REF_CTRL_CLKACT                             0x1

	* This register controls this reference clock
	* (OFFSET, MASK, VALUE)      (0XFF5E0108, 0x013F3F07U ,0x01011E02U)
	*/
	PSU_Mask_Write( CRL_APB_AMS_REF_CTRL_OFFSET,
	                0x013F3F07U, 0x01011E02U );
	/*##################################################################### */

	/*
	* Register : DLL_REF_CTRL @ 0XFF5E0104

	* 000 = IOPLL; 001 = RPLL; (This signal may only be toggled after 4 cycles
	*  of the old clock and 4 cycles of the new clock. This is not usually an
	* issue, but designers must be aware.)
	*  PSU_CRL_APB_DLL_REF_CTRL_SRCSEL                             0

	* This register controls this reference clock
	* (OFFSET, MASK, VALUE)      (0XFF5E0104, 0x00000007U ,0x00000000U)
	*/
	PSU_Mask_Write( CRL_APB_DLL_REF_CTRL_OFFSET,
	                0x00000007U, 0x00000000U );
	/*##################################################################### */

	/*
	* Register : TIMESTAMP_REF_CTRL @ 0XFF5E0128

	* 6 bit divider
	*  PSU_CRL_APB_TIMESTAMP_REF_CTRL_DIVISOR0                     0xf

	* 1XX = pss_ref_clk; 000 = IOPLL; 010 = RPLL; 011 = DPLL; (This signal may
	*  only be toggled after 4 cycles of the old clock and 4 cycles of the new
	*  clock. This is not usually an issue, but designers must be aware.)
	*  PSU_CRL_APB_TIMESTAMP_REF_CTRL_SRCSEL                       0x0

	* Clock active signal. Switch to 0 to disable the clock
	*  PSU_CRL_APB_TIMESTAMP_REF_CTRL_CLKACT                       0x1

	* This register controls this reference clock
	* (OFFSET, MASK, VALUE)      (0XFF5E0128, 0x01003F07U ,0x01000F00U)
	*/
	PSU_Mask_Write( CRL_APB_TIMESTAMP_REF_CTRL_OFFSET,
	                0x01003F07U, 0x01000F00U );
	/*##################################################################### */

	/*
	* Register : DP_VIDEO_REF_CTRL @ 0XFD1A0070

	* 6 bit divider
	*  PSU_CRF_APB_DP_VIDEO_REF_CTRL_DIVISOR1                      0x1

	* 6 bit divider
	*  PSU_CRF_APB_DP_VIDEO_REF_CTRL_DIVISOR0                      0x5

	* 000 = VPLL; 010 = DPLL; 011 = RPLL_TO_FPD - might be using extra mux; (T
	* his signal may only be toggled after 4 cycles of the old clock and 4 cyc
	* les of the new clock. This is not usually an issue, but designers must b
	* e aware.)
	*  PSU_CRF_APB_DP_VIDEO_REF_CTRL_SRCSEL                        0x0

	* Clock active signal. Switch to 0 to disable the clock
	*  PSU_CRF_APB_DP_VIDEO_REF_CTRL_CLKACT                        0x1

	* This register controls this reference clock
	* (OFFSET, MASK, VALUE)      (0XFD1A0070, 0x013F3F07U ,0x01010500U)
	*/
	PSU_Mask_Write( CRF_APB_DP_VIDEO_REF_CTRL_OFFSET,
	                0x013F3F07U, 0x01010500U );
	/*##################################################################### */

	/*
	* Register : DP_AUDIO_REF_CTRL @ 0XFD1A0074

	* 6 bit divider
	*  PSU_CRF_APB_DP_AUDIO_REF_CTRL_DIVISOR1                      0x1

	* 6 bit divider
	*  PSU_CRF_APB_DP_AUDIO_REF_CTRL_DIVISOR0                      0x16

	* 000 = VPLL; 010 = DPLL; 011 = RPLL_TO_FPD - might be using extra mux; (T
	* his signal may only be toggled after 4 cycles of the old clock and 4 cyc
	* les of the new clock. This is not usually an issue, but designers must b
	* e aware.)
	*  PSU_CRF_APB_DP_AUDIO_REF_CTRL_SRCSEL                        0x3

	* Clock active signal. Switch to 0 to disable the clock
	*  PSU_CRF_APB_DP_AUDIO_REF_CTRL_CLKACT                        0x1

	* This register controls this reference clock
	* (OFFSET, MASK, VALUE)      (0XFD1A0074, 0x013F3F07U ,0x01011603U)
	*/
	PSU_Mask_Write( CRF_APB_DP_AUDIO_REF_CTRL_OFFSET,
	                0x013F3F07U, 0x01011603U );
	/*##################################################################### */

	/*
	* Register : DP_STC_REF_CTRL @ 0XFD1A007C

	* 6 bit divider
	*  PSU_CRF_APB_DP_STC_REF_CTRL_DIVISOR1                        0x1

	* 6 bit divider
	*  PSU_CRF_APB_DP_STC_REF_CTRL_DIVISOR0                        0x14

	* 000 = VPLL; 010 = DPLL; 011 = RPLL_TO_FPD; (This signal may only be togg
	* led after 4 cycles of the old clock and 4 cycles of the new clock. This
	* is not usually an issue, but designers must be aware.)
	*  PSU_CRF_APB_DP_STC_REF_CTRL_SRCSEL                          0x3

	* Clock active signal. Switch to 0 to disable the clock
	*  PSU_CRF_APB_DP_STC_REF_CTRL_CLKACT                          0x1

	* This register controls this reference clock
	* (OFFSET, MASK, VALUE)      (0XFD1A007C, 0x013F3F07U ,0x01011403U)
	*/
	PSU_Mask_Write( CRF_APB_DP_STC_REF_CTRL_OFFSET,
	                0x013F3F07U, 0x01011403U );
	/*##################################################################### */

	/*
	* Register : ACPU_CTRL @ 0XFD1A0060

	* 6 bit divider
	*  PSU_CRF_APB_ACPU_CTRL_DIVISOR0                              0x1

	* 000 = APLL; 010 = DPLL; 011 = VPLL; (This signal may only be toggled aft
	* er 4 cycles of the old clock and 4 cycles of the new clock. This is not
	* usually an issue, but designers must be aware.)
	*  PSU_CRF_APB_ACPU_CTRL_SRCSEL                                0x0

	* Clock active signal. Switch to 0 to disable the clock. For the half spee
	* d APU Clock
	*  PSU_CRF_APB_ACPU_CTRL_CLKACT_HALF                           0x1

	* Clock active signal. Switch to 0 to disable the clock. For the full spee
	* d ACPUX Clock. This will shut off the high speed clock to the entire APU
	*  PSU_CRF_APB_ACPU_CTRL_CLKACT_FULL                           0x1

	* This register controls this reference clock
	* (OFFSET, MASK, VALUE)      (0XFD1A0060, 0x03003F07U ,0x03000100U)
	*/
	PSU_Mask_Write( CRF_APB_ACPU_CTRL_OFFSET, 0x03003F07U, 0x03000100U );
	/*##################################################################### */

	/*
	* Register : DBG_FPD_CTRL @ 0XFD1A0068

	* 6 bit divider
	*  PSU_CRF_APB_DBG_FPD_CTRL_DIVISOR0                           0x2

	* 000 = IOPLL_TO_FPD; 010 = DPLL; 011 = APLL; (This signal may only be tog
	* gled after 4 cycles of the old clock and 4 cycles of the new clock. This
	*  is not usually an issue, but designers must be aware.)
	*  PSU_CRF_APB_DBG_FPD_CTRL_SRCSEL                             0x0

	* Clock active signal. Switch to 0 to disable the clock
	*  PSU_CRF_APB_DBG_FPD_CTRL_CLKACT                             0x1

	* This register controls this reference clock
	* (OFFSET, MASK, VALUE)      (0XFD1A0068, 0x01003F07U ,0x01000200U)
	*/
	PSU_Mask_Write( CRF_APB_DBG_FPD_CTRL_OFFSET,
	                0x01003F07U, 0x01000200U );
	/*##################################################################### */

	/*
	* Register : DDR_CTRL @ 0XFD1A0080

	* 6 bit divider
	*  PSU_CRF_APB_DDR_CTRL_DIVISOR0                               0x2

	* 000 = DPLL; 001 = VPLL; (This signal may only be toggled after 4 cycles
	* of the old clock and 4 cycles of the new clock. This is not usually an i
	* ssue, but designers must be aware.)
	*  PSU_CRF_APB_DDR_CTRL_SRCSEL                                 0x0

	* This register controls this reference clock
	* (OFFSET, MASK, VALUE)      (0XFD1A0080, 0x00003F07U ,0x00000200U)
	*/
	PSU_Mask_Write( CRF_APB_DDR_CTRL_OFFSET, 0x00003F07U, 0x00000200U );
	/*##################################################################### */

	/*
	* Register : GPU_REF_CTRL @ 0XFD1A0084

	* 6 bit divider
	*  PSU_CRF_APB_GPU_REF_CTRL_DIVISOR0                           0x1

	* 000 = IOPLL_TO_FPD; 010 = VPLL; 011 = DPLL; (This signal may only be tog
	* gled after 4 cycles of the old clock and 4 cycles of the new clock. This
	*  is not usually an issue, but designers must be aware.)
	*  PSU_CRF_APB_GPU_REF_CTRL_SRCSEL                             0x0

	* Clock active signal. Switch to 0 to disable the clock, which will stop c
	* lock for GPU (and both Pixel Processors).
	*  PSU_CRF_APB_GPU_REF_CTRL_CLKACT                             0x1

	* Clock active signal for Pixel Processor. Switch to 0 to disable the cloc
	* k only to this Pixel Processor
	*  PSU_CRF_APB_GPU_REF_CTRL_PP0_CLKACT                         0x1

	* Clock active signal for Pixel Processor. Switch to 0 to disable the cloc
	* k only to this Pixel Processor
	*  PSU_CRF_APB_GPU_REF_CTRL_PP1_CLKACT                         0x1

	* This register controls this reference clock
	* (OFFSET, MASK, VALUE)      (0XFD1A0084, 0x07003F07U ,0x07000100U)
	*/
	PSU_Mask_Write( CRF_APB_GPU_REF_CTRL_OFFSET,
	                0x07003F07U, 0x07000100U );
	/*##################################################################### */

	/*
	* Register : GDMA_REF_CTRL @ 0XFD1A00B8

	* 6 bit divider
	*  PSU_CRF_APB_GDMA_REF_CTRL_DIVISOR0                          0x2

	* 000 = APLL; 010 = VPLL; 011 = DPLL; (This signal may only be toggled aft
	* er 4 cycles of the old clock and 4 cycles of the new clock. This is not
	* usually an issue, but designers must be aware.)
	*  PSU_CRF_APB_GDMA_REF_CTRL_SRCSEL                            0x3

	* Clock active signal. Switch to 0 to disable the clock
	*  PSU_CRF_APB_GDMA_REF_CTRL_CLKACT                            0x1

	* This register controls this reference clock
	* (OFFSET, MASK, VALUE)      (0XFD1A00B8, 0x01003F07U ,0x01000203U)
	*/
	PSU_Mask_Write( CRF_APB_GDMA_REF_CTRL_OFFSET,
	                0x01003F07U, 0x01000203U );
	/*##################################################################### */

	/*
	* Register : DPDMA_REF_CTRL @ 0XFD1A00BC

	* 6 bit divider
	*  PSU_CRF_APB_DPDMA_REF_CTRL_DIVISOR0                         0x3

	* 000 = APLL; 010 = VPLL; 011 = DPLL; (This signal may only be toggled aft
	* er 4 cycles of the old clock and 4 cycles of the new clock. This is not
	* usually an issue, but designers must be aware.)
	*  PSU_CRF_APB_DPDMA_REF_CTRL_SRCSEL                           0x0

	* Clock active signal. Switch to 0 to disable the clock
	*  PSU_CRF_APB_DPDMA_REF_CTRL_CLKACT                           0x1

	* This register controls this reference clock
	* (OFFSET, MASK, VALUE)      (0XFD1A00BC, 0x01003F07U ,0x01000300U)
	*/
	PSU_Mask_Write( CRF_APB_DPDMA_REF_CTRL_OFFSET,
	                0x01003F07U, 0x01000300U );
	/*##################################################################### */

	/*
	* Register : TOPSW_MAIN_CTRL @ 0XFD1A00C0

	* 6 bit divider
	*  PSU_CRF_APB_TOPSW_MAIN_CTRL_DIVISOR0                        0x2

	* 000 = APLL; 010 = VPLL; 011 = DPLL; (This signal may only be toggled aft
	* er 4 cycles of the old clock and 4 cycles of the new clock. This is not
	* usually an issue, but designers must be aware.)
	*  PSU_CRF_APB_TOPSW_MAIN_CTRL_SRCSEL                          0x3

	* Clock active signal. Switch to 0 to disable the clock
	*  PSU_CRF_APB_TOPSW_MAIN_CTRL_CLKACT                          0x1

	* This register controls this reference clock
	* (OFFSET, MASK, VALUE)      (0XFD1A00C0, 0x01003F07U ,0x01000203U)
	*/
	PSU_Mask_Write( CRF_APB_TOPSW_MAIN_CTRL_OFFSET,
	                0x01003F07U, 0x01000203U );
	/*##################################################################### */

	/*
	* Register : TOPSW_LSBUS_CTRL @ 0XFD1A00C4

	* 6 bit divider
	*  PSU_CRF_APB_TOPSW_LSBUS_CTRL_DIVISOR0                       0x5

	* 000 = APLL; 010 = IOPLL_TO_FPD; 011 = DPLL; (This signal may only be tog
	* gled after 4 cycles of the old clock and 4 cycles of the new clock. This
	*  is not usually an issue, but designers must be aware.)
	*  PSU_CRF_APB_TOPSW_LSBUS_CTRL_SRCSEL                         0x2

	* Clock active signal. Switch to 0 to disable the clock
	*  PSU_CRF_APB_TOPSW_LSBUS_CTRL_CLKACT                         0x1

	* This register controls this reference clock
	* (OFFSET, MASK, VALUE)      (0XFD1A00C4, 0x01003F07U ,0x01000502U)
	*/
	PSU_Mask_Write( CRF_APB_TOPSW_LSBUS_CTRL_OFFSET,
	                0x01003F07U, 0x01000502U );
	/*##################################################################### */

	/*
	* Register : DBG_TSTMP_CTRL @ 0XFD1A00F8

	* 6 bit divider
	*  PSU_CRF_APB_DBG_TSTMP_CTRL_DIVISOR0                         0x2

	* 000 = IOPLL_TO_FPD; 010 = DPLL; 011 = APLL; (This signal may only be tog
	* gled after 4 cycles of the old clock and 4 cycles of the new clock. This
	*  is not usually an issue, but designers must be aware.)
	*  PSU_CRF_APB_DBG_TSTMP_CTRL_SRCSEL                           0x0

	* This register controls this reference clock
	* (OFFSET, MASK, VALUE)      (0XFD1A00F8, 0x00003F07U ,0x00000200U)
	*/
	PSU_Mask_Write( CRF_APB_DBG_TSTMP_CTRL_OFFSET,
	                0x00003F07U, 0x00000200U );
	/*##################################################################### */

	/*
	* Register : IOU_TTC_APB_CLK @ 0XFF180380

	* 00" = Select the APB switch clock for the APB interface of TTC0'01" = Se
	* lect the PLL ref clock for the APB interface of TTC0'10" = Select the R5
	*  clock for the APB interface of TTC0
	*  PSU_IOU_SLCR_IOU_TTC_APB_CLK_TTC0_SEL                       0

	* 00" = Select the APB switch clock for the APB interface of TTC1'01" = Se
	* lect the PLL ref clock for the APB interface of TTC1'10" = Select the R5
	*  clock for the APB interface of TTC1
	*  PSU_IOU_SLCR_IOU_TTC_APB_CLK_TTC1_SEL                       0

	* 00" = Select the APB switch clock for the APB interface of TTC2'01" = Se
	* lect the PLL ref clock for the APB interface of TTC2'10" = Select the R5
	*  clock for the APB interface of TTC2
	*  PSU_IOU_SLCR_IOU_TTC_APB_CLK_TTC2_SEL                       0

	* 00" = Select the APB switch clock for the APB interface of TTC3'01" = Se
	* lect the PLL ref clock for the APB interface of TTC3'10" = Select the R5
	*  clock for the APB interface of TTC3
	*  PSU_IOU_SLCR_IOU_TTC_APB_CLK_TTC3_SEL                       0

	* TTC APB clock select
	* (OFFSET, MASK, VALUE)      (0XFF180380, 0x000000FFU ,0x00000000U)
	*/
	PSU_Mask_Write( IOU_SLCR_IOU_TTC_APB_CLK_OFFSET,
	                0x000000FFU, 0x00000000U );
	/*##################################################################### */

	/*
	* Register : WDT_CLK_SEL @ 0XFD610100

	* System watchdog timer clock source selection: 0: Internal APB clock 1: E
	* xternal (PL clock via EMIO or Pinout clock via MIO)
	*  PSU_FPD_SLCR_WDT_CLK_SEL_SELECT                             0

	* SWDT clock source select
	* (OFFSET, MASK, VALUE)      (0XFD610100, 0x00000001U ,0x00000000U)
	*/
	PSU_Mask_Write( FPD_SLCR_WDT_CLK_SEL_OFFSET,
	                0x00000001U, 0x00000000U );
	/*##################################################################### */

	/*
	* Register : WDT_CLK_SEL @ 0XFF180300

	* System watchdog timer clock source selection: 0: internal clock APB cloc
	* k 1: external clock from PL via EMIO, or from pinout via MIO
	*  PSU_IOU_SLCR_WDT_CLK_SEL_SELECT                             0

	* SWDT clock source select
	* (OFFSET, MASK, VALUE)      (0XFF180300, 0x00000001U ,0x00000000U)
	*/
	PSU_Mask_Write( IOU_SLCR_WDT_CLK_SEL_OFFSET,
	                0x00000001U, 0x00000000U );
	/*##################################################################### */

	/*
	* Register : CSUPMU_WDT_CLK_SEL @ 0XFF410050

	* System watchdog timer clock source selection: 0: internal clock APB cloc
	* k 1: external clock pss_ref_clk
	*  PSU_LPD_SLCR_CSUPMU_WDT_CLK_SEL_SELECT                      0

	* SWDT clock source select
	* (OFFSET, MASK, VALUE)      (0XFF410050, 0x00000001U ,0x00000000U)
	*/
	PSU_Mask_Write( LPD_SLCR_CSUPMU_WDT_CLK_SEL_OFFSET,
	                0x00000001U, 0x00000000U );
	/*##################################################################### */


	return 1;
}
#endif // if 0