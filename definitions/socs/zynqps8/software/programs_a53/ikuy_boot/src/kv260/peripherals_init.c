#include "core/core.h"
#include "platform/reg_access.h"
#include "utils/boot_psi.h"
//#include "platform/memory_map.h"
//#include "platform/registers/crl_apb.h"
//#include "platform/registers/crf_apb.h"

#include "psu_init.h"

unsigned long psu_peripherals_pre_init_data(void)
{
	/*
	* SYSMON CLOCK PRESET TO IOPLL AT 1500 MHZ FROM PBR TO MAKE AMS CLOCK UNDE
	* R RANGE
	*/
	/*
	* Register : AMS_REF_CTRL @ 0XFF5E0108

	* 6 bit divider
	*  PSU_CRL_APB_AMS_REF_CTRL_DIVISOR1                           1

	* 6 bit divider
	*  PSU_CRL_APB_AMS_REF_CTRL_DIVISOR0                           35

	* 000 = RPLL; 010 = IOPLL; 011 = DPLL; (This signal may only be toggled af
	* ter 4 cycles of the old clock and 4 cycles of the new clock. This is not
	*  usually an issue, but designers must be aware.)
	*  PSU_CRL_APB_AMS_REF_CTRL_SRCSEL                             2

	* Clock active signal. Switch to 0 to disable the clock
	*  PSU_CRL_APB_AMS_REF_CTRL_CLKACT                             1

	* This register controls this reference clock
	* (OFFSET, MASK, VALUE)      (0XFF5E0108, 0x013F3F07U ,0x01012302U)
	*/
	PSU_Mask_Write(CRL_APB_AMS_REF_CTRL_OFFSET,
	               0x013F3F07U, 0x01012302U);
	/*##################################################################### */

	/*
	* PUT QSPI IN RESET STATE
	*/
	/*
	* Register : RST_LPD_IOU2 @ 0XFF5E0238

	* Block level reset
	*  PSU_CRL_APB_RST_LPD_IOU2_QSPI_RESET                         1

	* Software control register for the IOU block. Each bit will cause a singl
	* erperipheral or part of the peripheral to be reset.
	* (OFFSET, MASK, VALUE)      (0XFF5E0238, 0x00000001U ,0x00000001U)
	*/
	PSU_Mask_Write(CRL_APB_RST_LPD_IOU2_OFFSET,
	               0x00000001U, 0x00000001U);
	/*##################################################################### */


	return 1;
}
unsigned long psu_peripherals_init_data(void)
{
	/*
	* COHERENCY
	*/
	/*
	* FPD RESET
	*/
	/*
	* Register : RST_FPD_TOP @ 0XFD1A0100

	* Display Port block level reset (includes DPDMA)
	*  PSU_CRF_APB_RST_FPD_TOP_DP_RESET                            0

	* FPD WDT reset
	*  PSU_CRF_APB_RST_FPD_TOP_SWDT_RESET                          0

	* GDMA block level reset
	*  PSU_CRF_APB_RST_FPD_TOP_GDMA_RESET                          0

	* Pixel Processor (submodule of GPU) block level reset
	*  PSU_CRF_APB_RST_FPD_TOP_GPU_PP0_RESET                       0

	* Pixel Processor (submodule of GPU) block level reset
	*  PSU_CRF_APB_RST_FPD_TOP_GPU_PP1_RESET                       0

	* GPU block level reset
	*  PSU_CRF_APB_RST_FPD_TOP_GPU_RESET                           0

	* GT block level reset
	*  PSU_CRF_APB_RST_FPD_TOP_GT_RESET                            0

	* FPD Block level software controlled reset
	* (OFFSET, MASK, VALUE)      (0XFD1A0100, 0x0001807CU ,0x00000000U)
	*/
	PSU_Mask_Write(CRF_APB_RST_FPD_TOP_OFFSET, 0x0001807CU, 0x00000000U);
	/*##################################################################### */

	/*
	* RESET BLOCKS
	*/
	/*
	* TIMESTAMP
	*/
	/*
	* Register : RST_LPD_IOU2 @ 0XFF5E0238

	* Block level reset
	*  PSU_CRL_APB_RST_LPD_IOU2_TIMESTAMP_RESET                    0

	* Block level reset
	*  PSU_CRL_APB_RST_LPD_IOU2_IOU_CC_RESET                       0

	* Block level reset
	*  PSU_CRL_APB_RST_LPD_IOU2_ADMA_RESET                         0

	* Software control register for the IOU block. Each bit will cause a singl
	* erperipheral or part of the peripheral to be reset.
	* (OFFSET, MASK, VALUE)      (0XFF5E0238, 0x001A0000U ,0x00000000U)
	*/
	PSU_Mask_Write(CRL_APB_RST_LPD_IOU2_OFFSET,
	               0x001A0000U, 0x00000000U);
	/*##################################################################### */

	/*
	* Register : RST_LPD_TOP @ 0XFF5E023C

	* Reset entire full power domain.
	*  PSU_CRL_APB_RST_LPD_TOP_FPD_RESET                           0

	* LPD SWDT
	*  PSU_CRL_APB_RST_LPD_TOP_LPD_SWDT_RESET                      0

	* Sysmonitor reset
	*  PSU_CRL_APB_RST_LPD_TOP_SYSMON_RESET                        0

	* Real Time Clock reset
	*  PSU_CRL_APB_RST_LPD_TOP_RTC_RESET                           0

	* APM reset
	*  PSU_CRL_APB_RST_LPD_TOP_APM_RESET                           0

	* IPI reset
	*  PSU_CRL_APB_RST_LPD_TOP_IPI_RESET                           0

	* reset entire RPU power island
	*  PSU_CRL_APB_RST_LPD_TOP_RPU_PGE_RESET                       0

	* reset ocm
	*  PSU_CRL_APB_RST_LPD_TOP_OCM_RESET                           0

	* Software control register for the LPD block.
	* (OFFSET, MASK, VALUE)      (0XFF5E023C, 0x0093C018U ,0x00000000U)
	*/
	PSU_Mask_Write(CRL_APB_RST_LPD_TOP_OFFSET, 0x0093C018U, 0x00000000U);
	/*##################################################################### */

	/*
	* ENET
	*/
	/*
	* Register : RST_LPD_IOU0 @ 0XFF5E0230

	* GEM 3 reset
	*  PSU_CRL_APB_RST_LPD_IOU0_GEM3_RESET                         0

	* Software controlled reset for the GEMs
	* (OFFSET, MASK, VALUE)      (0XFF5E0230, 0x00000008U ,0x00000000U)
	*/
	PSU_Mask_Write(CRL_APB_RST_LPD_IOU0_OFFSET,
	               0x00000008U, 0x00000000U);
	/*##################################################################### */

	/*
	* QSPI
	*/
	/*
	* Register : RST_LPD_IOU2 @ 0XFF5E0238

	* Block level reset
	*  PSU_CRL_APB_RST_LPD_IOU2_QSPI_RESET                         0

	* Software control register for the IOU block. Each bit will cause a singl
	* erperipheral or part of the peripheral to be reset.
	* (OFFSET, MASK, VALUE)      (0XFF5E0238, 0x00000001U ,0x00000000U)
	*/
	PSU_Mask_Write(CRL_APB_RST_LPD_IOU2_OFFSET,
	               0x00000001U, 0x00000000U);
	/*##################################################################### */

	/*
	* QSPI TAP DELAY
	*/
	/*
	* Register : IOU_TAPDLY_BYPASS @ 0XFF180390

	* 0: Do not by pass the tap delays on the Rx clock signal of LQSPI 1: Bypa
	* ss the Tap delay on the Rx clock signal of LQSPI
	*  PSU_IOU_SLCR_IOU_TAPDLY_BYPASS_LQSPI_RX                     1

	* IOU tap delay bypass for the LQSPI and NAND controllers
	* (OFFSET, MASK, VALUE)      (0XFF180390, 0x00000004U ,0x00000004U)
	*/
	PSU_Mask_Write(IOU_SLCR_IOU_TAPDLY_BYPASS_OFFSET,
	               0x00000004U, 0x00000004U);
	/*##################################################################### */

	/*
	* NAND
	*/
	/*
	* USB RESET
	*/
	/*
	* Register : RST_LPD_TOP @ 0XFF5E023C

	* USB 0 reset for control registers
	*  PSU_CRL_APB_RST_LPD_TOP_USB0_APB_RESET                      0

	* Software control register for the LPD block.
	* (OFFSET, MASK, VALUE)      (0XFF5E023C, 0x00000400U ,0x00000000U)
	*/
	PSU_Mask_Write(CRL_APB_RST_LPD_TOP_OFFSET, 0x00000400U, 0x00000000U);
	/*##################################################################### */

	/*
	* SD
	*/
	/*
	* Register : RST_LPD_IOU2 @ 0XFF5E0238

	* Block level reset
	*  PSU_CRL_APB_RST_LPD_IOU2_SDIO1_RESET                        0

	* Software control register for the IOU block. Each bit will cause a singl
	* erperipheral or part of the peripheral to be reset.
	* (OFFSET, MASK, VALUE)      (0XFF5E0238, 0x00000040U ,0x00000000U)
	*/
	PSU_Mask_Write(CRL_APB_RST_LPD_IOU2_OFFSET,
	               0x00000040U, 0x00000000U);
	/*##################################################################### */

	/*
	* Register : CTRL_REG_SD @ 0XFF180310

	* SD or eMMC selection on SDIO1 0: SD enabled 1: eMMC enabled
	*  PSU_IOU_SLCR_CTRL_REG_SD_SD1_EMMC_SEL                       0

	* SD eMMC selection
	* (OFFSET, MASK, VALUE)      (0XFF180310, 0x00008000U ,0x00000000U)
	*/
	PSU_Mask_Write(IOU_SLCR_CTRL_REG_SD_OFFSET,
	               0x00008000U, 0x00000000U);
	/*##################################################################### */

	/*
	* Register : SD_CONFIG_REG2 @ 0XFF180320

	* Should be set based on the final product usage 00 - Removable SCard Slot
	*  01 - Embedded Slot for One Device 10 - Shared Bus Slot 11 - Reserved
	*  PSU_IOU_SLCR_SD_CONFIG_REG2_SD1_SLOTTYPE                    0

	* 8-bit Support for Embedded Device 1: The Core supports 8-bit Interface 0
	* : Supports only 4-bit SD Interface
	*  PSU_IOU_SLCR_SD_CONFIG_REG2_SD1_8BIT                        1

	* 1.8V Support 1: 1.8V supported 0: 1.8V not supported support
	*  PSU_IOU_SLCR_SD_CONFIG_REG2_SD1_1P8V                        1

	* 3.0V Support 1: 3.0V supported 0: 3.0V not supported support
	*  PSU_IOU_SLCR_SD_CONFIG_REG2_SD1_3P0V                        0

	* 3.3V Support 1: 3.3V supported 0: 3.3V not supported support
	*  PSU_IOU_SLCR_SD_CONFIG_REG2_SD1_3P3V                        1

	* SD Config Register 2
	* (OFFSET, MASK, VALUE)      (0XFF180320, 0x33840000U ,0x02840000U)
	*/
	PSU_Mask_Write(IOU_SLCR_SD_CONFIG_REG2_OFFSET,
	               0x33840000U, 0x02840000U);
	/*##################################################################### */

	/*
	* SD1 BASE CLOCK
	*/
	/*
	* Register : SD_CONFIG_REG1 @ 0XFF18031C

	* Base Clock Frequency for SD Clock. This is the frequency of the xin_clk.
	*  PSU_IOU_SLCR_SD_CONFIG_REG1_SD1_BASECLK                     0xc8

	* Configures the Number of Taps (Phases) of the rxclk_in that is supported
	* .
	*  PSU_IOU_SLCR_SD_CONFIG_REG1_SD1_TUNIGCOUNT                  0x28

	* SD Config Register 1
	* (OFFSET, MASK, VALUE)      (0XFF18031C, 0x7FFE0000U ,0x64500000U)
	*/
	PSU_Mask_Write(IOU_SLCR_SD_CONFIG_REG1_OFFSET,
	               0x7FFE0000U, 0x64500000U);
	/*##################################################################### */

	/*
	* Register : SD_DLL_CTRL @ 0XFF180358

	* Reserved.
	*  PSU_IOU_SLCR_SD_DLL_CTRL_RESERVED                           1

	* SDIO status register
	* (OFFSET, MASK, VALUE)      (0XFF180358, 0x00000008U ,0x00000008U)
	*/
	PSU_Mask_Write(IOU_SLCR_SD_DLL_CTRL_OFFSET,
	               0x00000008U, 0x00000008U);
	/*##################################################################### */

	/*
	* SD1 RETUNER
	*/
	/*
	* Register : SD_CONFIG_REG3 @ 0XFF180324

	* This is the Timer Count for Re-Tuning Timer for Re-Tuning Mode 1 to 3. S
	* etting to 4'b0 disables Re-Tuning Timer. 0h - Get information via other
	* source 1h = 1 seconds 2h = 2 seconds 3h = 4 seconds 4h = 8 seconds -- n
	* = 2(n-1) seconds -- Bh = 1024 seconds Fh - Ch = Reserved
	*  PSU_IOU_SLCR_SD_CONFIG_REG3_SD1_RETUNETMR                   0X0

	* SD Config Register 3
	* (OFFSET, MASK, VALUE)      (0XFF180324, 0x03C00000U ,0x00000000U)
	*/
	PSU_Mask_Write(IOU_SLCR_SD_CONFIG_REG3_OFFSET,
	               0x03C00000U, 0x00000000U);
	/*##################################################################### */

	/*
	* CAN
	*/
	/*
	* I2C
	*/
	/*
	* Register : RST_LPD_IOU2 @ 0XFF5E0238

	* Block level reset
	*  PSU_CRL_APB_RST_LPD_IOU2_I2C1_RESET                         0

	* Software control register for the IOU block. Each bit will cause a singl
	* erperipheral or part of the peripheral to be reset.
	* (OFFSET, MASK, VALUE)      (0XFF5E0238, 0x00000400U ,0x00000000U)
	*/
	PSU_Mask_Write(CRL_APB_RST_LPD_IOU2_OFFSET,
	               0x00000400U, 0x00000000U);
	/*##################################################################### */

	/*
	* SWDT
	*/
	/*
	* Register : RST_LPD_IOU2 @ 0XFF5E0238

	* Block level reset
	*  PSU_CRL_APB_RST_LPD_IOU2_SWDT_RESET                         0

	* Software control register for the IOU block. Each bit will cause a singl
	* erperipheral or part of the peripheral to be reset.
	* (OFFSET, MASK, VALUE)      (0XFF5E0238, 0x00008000U ,0x00000000U)
	*/
	PSU_Mask_Write(CRL_APB_RST_LPD_IOU2_OFFSET,
	               0x00008000U, 0x00000000U);
	/*##################################################################### */

	/*
	* SPI
	*/
	/*
	* Register : RST_LPD_IOU2 @ 0XFF5E0238

	* Block level reset
	*  PSU_CRL_APB_RST_LPD_IOU2_SPI1_RESET                         0

	* Software control register for the IOU block. Each bit will cause a singl
	* erperipheral or part of the peripheral to be reset.
	* (OFFSET, MASK, VALUE)      (0XFF5E0238, 0x00000010U ,0x00000000U)
	*/
	PSU_Mask_Write(CRL_APB_RST_LPD_IOU2_OFFSET,
	               0x00000010U, 0x00000000U);
	/*##################################################################### */

	/*
	* TTC
	*/
	/*
	* Register : RST_LPD_IOU2 @ 0XFF5E0238

	* Block level reset
	*  PSU_CRL_APB_RST_LPD_IOU2_TTC0_RESET                         0

	* Block level reset
	*  PSU_CRL_APB_RST_LPD_IOU2_TTC1_RESET                         0

	* Block level reset
	*  PSU_CRL_APB_RST_LPD_IOU2_TTC2_RESET                         0

	* Block level reset
	*  PSU_CRL_APB_RST_LPD_IOU2_TTC3_RESET                         0

	* Software control register for the IOU block. Each bit will cause a singl
	* erperipheral or part of the peripheral to be reset.
	* (OFFSET, MASK, VALUE)      (0XFF5E0238, 0x00007800U ,0x00000000U)
	*/
	PSU_Mask_Write(CRL_APB_RST_LPD_IOU2_OFFSET,
	               0x00007800U, 0x00000000U);
	/*##################################################################### */

	/*
	* UART
	*/
	/*
	* Register : RST_LPD_IOU2 @ 0XFF5E0238

	* Block level reset
	*  PSU_CRL_APB_RST_LPD_IOU2_UART1_RESET                        0

	* Software control register for the IOU block. Each bit will cause a singl
	* erperipheral or part of the peripheral to be reset.
	* (OFFSET, MASK, VALUE)      (0XFF5E0238, 0x00000004U ,0x00000000U)
	*/
	PSU_Mask_Write(CRL_APB_RST_LPD_IOU2_OFFSET,
	               0x00000004U, 0x00000000U);
	/*##################################################################### */

	/*
	* UART BAUD RATE
	*/
	/*
	* Register : Baud_rate_divider_reg0 @ 0XFF010034

	* Baud rate divider value: 0 - 3: ignored 4 - 255: Baud rate
	*  PSU_UART1_BAUD_RATE_DIVIDER_REG0_BDIV                       0x6

	* Baud Rate Divider Register
	* (OFFSET, MASK, VALUE)      (0XFF010034, 0x000000FFU ,0x00000006U)
	*/
	PSU_Mask_Write(UART1_BAUD_RATE_DIVIDER_REG0_OFFSET,
	               0x000000FFU, 0x00000006U);
	/*##################################################################### */

	/*
	* Register : Baud_rate_gen_reg0 @ 0XFF010018

	* Baud Rate Clock Divisor Value: 0: Disables baud_sample 1: Clock divisor
	* bypass (baud_sample = sel_clk) 2 - 65535: baud_sample
	*  PSU_UART1_BAUD_RATE_GEN_REG0_CD                             0x7c

	* Baud Rate Generator Register.
	* (OFFSET, MASK, VALUE)      (0XFF010018, 0x0000FFFFU ,0x0000007CU)
	*/
	PSU_Mask_Write(UART1_BAUD_RATE_GEN_REG0_OFFSET,
	               0x0000FFFFU, 0x0000007CU);
	/*##################################################################### */

	/*
	* Register : Control_reg0 @ 0XFF010000

	* Stop transmitter break: 0: no affect 1: stop transmission of the break a
	* fter a minimum of one character length and transmit a high level during
	* 12 bit periods. It can be set regardless of the value of STTBRK.
	*  PSU_UART1_CONTROL_REG0_STPBRK                               0x0

	* Start transmitter break: 0: no affect 1: start to transmit a break after
	*  the characters currently present in the FIFO and the transmit shift reg
	* ister have been transmitted. It can only be set if STPBRK (Stop transmit
	* ter break) is not high.
	*  PSU_UART1_CONTROL_REG0_STTBRK                               0x0

	* Restart receiver timeout counter: 1: receiver timeout counter is restart
	* ed. This bit is self clearing once the restart has completed.
	*  PSU_UART1_CONTROL_REG0_RSTTO                                0x0

	* Transmit disable: 0: enable transmitter 1: disable transmitter
	*  PSU_UART1_CONTROL_REG0_TXDIS                                0x0

	* Transmit enable: 0: disable transmitter 1: enable transmitter, provided
	* the TXDIS field is set to 0.
	*  PSU_UART1_CONTROL_REG0_TXEN                                 0x1

	* Receive disable: 0: enable 1: disable, regardless of the value of RXEN
	*  PSU_UART1_CONTROL_REG0_RXDIS                                0x0

	* Receive enable: 0: disable 1: enable When set to one, the receiver logic
	*  is enabled, provided the RXDIS field is set to zero.
	*  PSU_UART1_CONTROL_REG0_RXEN                                 0x1

	* Software reset for Tx data path: 0: no affect 1: transmitter logic is re
	* set and all pending transmitter data is discarded This bit is self clear
	* ing once the reset has completed.
	*  PSU_UART1_CONTROL_REG0_TXRES                                0x1

	* Software reset for Rx data path: 0: no affect 1: receiver logic is reset
	*  and all pending receiver data is discarded. This bit is self clearing o
	* nce the reset has completed.
	*  PSU_UART1_CONTROL_REG0_RXRES                                0x1

	* UART Control Register
	* (OFFSET, MASK, VALUE)      (0XFF010000, 0x000001FFU ,0x00000017U)
	*/
	PSU_Mask_Write(UART1_CONTROL_REG0_OFFSET, 0x000001FFU, 0x00000017U);
	/*##################################################################### */

	/*
	* Register : mode_reg0 @ 0XFF010004

	* Channel mode: Defines the mode of operation of the UART. 00: normal 01:
	* automatic echo 10: local loopback 11: remote loopback
	*  PSU_UART1_MODE_REG0_CHMODE                                  0x0

	* Number of stop bits: Defines the number of stop bits to detect on receiv
	* e and to generate on transmit. 00: 1 stop bit 01: 1.5 stop bits 10: 2 st
	* op bits 11: reserved
	*  PSU_UART1_MODE_REG0_NBSTOP                                  0x0

	* Parity type select: Defines the expected parity to check on receive and
	* the parity to generate on transmit. 000: even parity 001: odd parity 010
	* : forced to 0 parity (space) 011: forced to 1 parity (mark) 1xx: no pari
	* ty
	*  PSU_UART1_MODE_REG0_PAR                                     0x4

	* Character length select: Defines the number of bits in each character. 1
	* 1: 6 bits 10: 7 bits 0x: 8 bits
	*  PSU_UART1_MODE_REG0_CHRL                                    0x0

	* Clock source select: This field defines whether a pre-scalar of 8 is app
	* lied to the baud rate generator input clock. 0: clock source is uart_ref
	* _clk 1: clock source is uart_ref_clk/8
	*  PSU_UART1_MODE_REG0_CLKS                                    0x0

	* UART Mode Register
	* (OFFSET, MASK, VALUE)      (0XFF010004, 0x000003FFU ,0x00000020U)
	*/
	PSU_Mask_Write(UART1_MODE_REG0_OFFSET, 0x000003FFU, 0x00000020U);
	/*##################################################################### */

	/*
	* GPIO
	*/
	/*
	* Register : RST_LPD_IOU2 @ 0XFF5E0238

	* Block level reset
	*  PSU_CRL_APB_RST_LPD_IOU2_GPIO_RESET                         0

	* Software control register for the IOU block. Each bit will cause a singl
	* erperipheral or part of the peripheral to be reset.
	* (OFFSET, MASK, VALUE)      (0XFF5E0238, 0x00040000U ,0x00000000U)
	*/
	PSU_Mask_Write(CRL_APB_RST_LPD_IOU2_OFFSET,
	               0x00040000U, 0x00000000U);
	/*##################################################################### */

	/*
	* ADMA TZ
	*/
	/*
	* Register : slcr_adma @ 0XFF4B0024

	* TrustZone Classification for ADMA
	*  PSU_LPD_SLCR_SECURE_SLCR_ADMA_TZ                            0XFF

	* RPU TrustZone settings
	* (OFFSET, MASK, VALUE)      (0XFF4B0024, 0x000000FFU ,0x000000FFU)
	*/
	PSU_Mask_Write(LPD_SLCR_SECURE_SLCR_ADMA_OFFSET,
	               0x000000FFU, 0x000000FFU);
	/*##################################################################### */

	/*
	* CSU TAMPERING
	*/
	/*
	* CSU TAMPER STATUS
	*/
	/*
	* Register : tamper_status @ 0XFFCA5000

	* CSU regsiter
	*  PSU_CSU_TAMPER_STATUS_TAMPER_0                              0

	* External MIO
	*  PSU_CSU_TAMPER_STATUS_TAMPER_1                              0

	* JTAG toggle detect
	*  PSU_CSU_TAMPER_STATUS_TAMPER_2                              0

	* PL SEU error
	*  PSU_CSU_TAMPER_STATUS_TAMPER_3                              0

	* AMS over temperature alarm for LPD
	*  PSU_CSU_TAMPER_STATUS_TAMPER_4                              0

	* AMS over temperature alarm for APU
	*  PSU_CSU_TAMPER_STATUS_TAMPER_5                              0

	* AMS voltage alarm for VCCPINT_FPD
	*  PSU_CSU_TAMPER_STATUS_TAMPER_6                              0

	* AMS voltage alarm for VCCPINT_LPD
	*  PSU_CSU_TAMPER_STATUS_TAMPER_7                              0

	* AMS voltage alarm for VCCPAUX
	*  PSU_CSU_TAMPER_STATUS_TAMPER_8                              0

	* AMS voltage alarm for DDRPHY
	*  PSU_CSU_TAMPER_STATUS_TAMPER_9                              0

	* AMS voltage alarm for PSIO bank 0/1/2
	*  PSU_CSU_TAMPER_STATUS_TAMPER_10                             0

	* AMS voltage alarm for PSIO bank 3 (dedicated pins)
	*  PSU_CSU_TAMPER_STATUS_TAMPER_11                             0

	* AMS voltaage alarm for GT
	*  PSU_CSU_TAMPER_STATUS_TAMPER_12                             0

	* Tamper Response Status
	* (OFFSET, MASK, VALUE)      (0XFFCA5000, 0x00001FFFU ,0x00000000U)
	*/
	PSU_Mask_Write(CSU_TAMPER_STATUS_OFFSET, 0x00001FFFU, 0x00000000U);
	/*##################################################################### */

	/*
	* CSU TAMPER RESPONSE
	*/
	/*
	* CPU QOS DEFAULT
	*/
	/*
	* Register : ACE_CTRL @ 0XFD5C0060

	* Set ACE outgoing AWQOS value
	*  PSU_APU_ACE_CTRL_AWQOS                                      0X0

	* Set ACE outgoing ARQOS value
	*  PSU_APU_ACE_CTRL_ARQOS                                      0X0

	* ACE Control Register
	* (OFFSET, MASK, VALUE)      (0XFD5C0060, 0x000F000FU ,0x00000000U)
	*/
	PSU_Mask_Write(APU_ACE_CTRL_OFFSET, 0x000F000FU, 0x00000000U);
	/*##################################################################### */

	/*
	* ENABLES RTC SWITCH TO BATTERY WHEN VCC_PSAUX IS NOT AVAILABLE
	*/
	/*
	* Register : CONTROL @ 0XFFA60040

	* Enables the RTC. By writing a 0 to this bit, RTC will be powered off and
	*  the only module that potentially draws current from the battery will be
	*  BBRAM. The value read through this bit does not necessarily reflect whe
	* ther RTC is enabled or not. It is expected that RTC is enabled every tim
	* e it is being configured. If RTC is not used in the design, FSBL will di
	* sable it by writing a 0 to this bit.
	*  PSU_RTC_CONTROL_BATTERY_DISABLE                             0X1

	* This register controls various functionalities within the RTC
	* (OFFSET, MASK, VALUE)      (0XFFA60040, 0x80000000U ,0x80000000U)
	*/
	PSU_Mask_Write(RTC_CONTROL_OFFSET, 0x80000000U, 0x80000000U);
	/*##################################################################### */

	/*
	* TIMESTAMP COUNTER
	*/
	/*
	* Register : base_frequency_ID_register @ 0XFF260020

	* Frequency in number of ticks per second. Valid range from 10 MHz to 100
	* MHz.
	*  PSU_IOU_SCNTRS_BASE_FREQUENCY_ID_REGISTER_FREQ              0x5f5dd18

	* Program this register to match the clock frequency of the timestamp gene
	* rator, in ticks per second. For example, for a 50 MHz clock, program 0x0
	* 2FAF080. This register is not accessible to the read-only programming in
	* terface.
	* (OFFSET, MASK, VALUE)      (0XFF260020, 0xFFFFFFFFU ,0x05F5DD18U)
	*/
	PSU_Mask_Write(IOU_SCNTRS_BASE_FREQUENCY_ID_REGISTER_OFFSET,
	               0xFFFFFFFFU, 0x05F5DD18U);
	/*##################################################################### */

	/*
	* Register : counter_control_register @ 0XFF260000

	* Enable 0: The counter is disabled and not incrementing. 1: The counter i
	* s enabled and is incrementing.
	*  PSU_IOU_SCNTRS_COUNTER_CONTROL_REGISTER_EN                  0x1

	* Controls the counter increments. This register is not accessible to the
	* read-only programming interface.
	* (OFFSET, MASK, VALUE)      (0XFF260000, 0x00000001U ,0x00000001U)
	*/
	PSU_Mask_Write(IOU_SCNTRS_COUNTER_CONTROL_REGISTER_OFFSET,
	               0x00000001U, 0x00000001U);
	/*##################################################################### */

	/*
	* TTC SRC SELECT
	*/
	/*
	* USB RESET
	*/
	/*
	* USB RESET WITH BOOT PIN MODE
	*/
	/*
	* BOOT PIN HIGH
	*/
	/*
	* Register : BOOT_PIN_CTRL @ 0XFF5E0250

	* Value driven onto the mode pins, when out_en = 1
	*  PSU_CRL_APB_BOOT_PIN_CTRL_OUT_VAL                           0X2

	* When 0, the pins will be inputs from the board to the PS. When 1, the PS
	*  will drive these pins
	*  PSU_CRL_APB_BOOT_PIN_CTRL_OUT_EN                            0X2

	* Used to control the mode pins after boot.
	* (OFFSET, MASK, VALUE)      (0XFF5E0250, 0x00000F0FU ,0x00000202U)
	*/
	PSU_Mask_Write(CRL_APB_BOOT_PIN_CTRL_OFFSET,
	               0x00000F0FU, 0x00000202U);
	/*##################################################################### */

	/*
	* ADD 1US DELAY
	*/
	mask_delay(1);

	/*##################################################################### */

	/*
	* BOOT PIN LOW
	*/
	/*
	* Register : BOOT_PIN_CTRL @ 0XFF5E0250

	* Value driven onto the mode pins, when out_en = 1
	*  PSU_CRL_APB_BOOT_PIN_CTRL_OUT_VAL                           0X0

	* When 0, the pins will be inputs from the board to the PS. When 1, the PS
	*  will drive these pins
	*  PSU_CRL_APB_BOOT_PIN_CTRL_OUT_EN                            0X2

	* Used to control the mode pins after boot.
	* (OFFSET, MASK, VALUE)      (0XFF5E0250, 0x00000F0FU ,0x00000002U)
	*/
	PSU_Mask_Write(CRL_APB_BOOT_PIN_CTRL_OFFSET,
	               0x00000F0FU, 0x00000002U);
	/*##################################################################### */

	/*
	* ADD 5US DELAY
	*/
	mask_delay(5);

	/*##################################################################### */

	/*
	* BOOT PIN HIGH
	*/
	/*
	* Register : BOOT_PIN_CTRL @ 0XFF5E0250

	* Value driven onto the mode pins, when out_en = 1
	*  PSU_CRL_APB_BOOT_PIN_CTRL_OUT_VAL                           0X2

	* When 0, the pins will be inputs from the board to the PS. When 1, the PS
	*  will drive these pins
	*  PSU_CRL_APB_BOOT_PIN_CTRL_OUT_EN                            0X2

	* Used to control the mode pins after boot.
	* (OFFSET, MASK, VALUE)      (0XFF5E0250, 0x00000F0FU ,0x00000202U)
	*/
	PSU_Mask_Write(CRL_APB_BOOT_PIN_CTRL_OFFSET,
	               0x00000F0FU, 0x00000202U);
	/*##################################################################### */

	/*
	* GPIO POLARITY INITIALIZATION
	*/

	return 1;
}

void init_peripheral(void)
{
	/*SMMU_REG Interrrupt Enable: Followig register need to be written all the time to properly catch SMMU messages.*/
	PSU_Mask_Write(0xFD5F0018, 0x8000001FU, 0x8000001FU);
}

