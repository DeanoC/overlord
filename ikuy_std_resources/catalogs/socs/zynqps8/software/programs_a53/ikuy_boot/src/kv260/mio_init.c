#include "core/core.h"
#include "platform/reg_access.h"
#include "utils/boot_psi.h"
#include "platform/memory_map.h"
#include "platform/registers/iou_slcr.h"

#define MIO_PIN_MASK  HW_REG_FIELD_MASK( IOU_SLCR, MIO_PIN_0, L0_SEL ) | \
                      HW_REG_FIELD_MASK( IOU_SLCR, MIO_PIN_0, L1_SEL ) | \
                      HW_REG_FIELD_MASK( IOU_SLCR, MIO_PIN_0, L2_SEL ) | \
                      HW_REG_FIELD_MASK( IOU_SLCR, MIO_PIN_0, L3_SEL )

// NOTE: the same value on different pins doesn't mean the same function!
//       the macros try to help make it readible but BEWARE!
#define MIO_PIN_L0_QSPI  HW_REG_ENCODE_FIELD( IOU_SLCR, MIO_PIN_0, L0_SEL, 1 )
#define MIO_PIN_L0_GEM  HW_REG_ENCODE_FIELD( IOU_SLCR, MIO_PIN_0, L0_SEL, 1 )

#define MIO_PIN_L1_USB  HW_REG_ENCODE_FIELD( IOU_SLCR, MIO_PIN_0, L1_SEL, 1 )

#define MIO_PIN_L2(x)  HW_REG_ENCODE_FIELD( IOU_SLCR, MIO_PIN_0, L0_SEL, 0 ) | \
										HW_REG_ENCODE_FIELD( IOU_SLCR, MIO_PIN_0, L1_SEL, 0 ) | \
										HW_REG_ENCODE_FIELD( IOU_SLCR, MIO_PIN_0, L2_SEL, x ) | \
										HW_REG_ENCODE_FIELD( IOU_SLCR, MIO_PIN_0, L3_SEL, 0 )

#define MIO_PIN_L3(x)  HW_REG_ENCODE_FIELD( IOU_SLCR, MIO_PIN_0, L0_SEL, 0 ) | \
										HW_REG_ENCODE_FIELD( IOU_SLCR, MIO_PIN_0, L1_SEL, 0 ) | \
										HW_REG_ENCODE_FIELD( IOU_SLCR, MIO_PIN_0, L2_SEL, 0 ) | \
										HW_REG_ENCODE_FIELD( IOU_SLCR, MIO_PIN_0, L3_SEL, x )

#define MIO_PIN_GPIO MIO_PIN_L3(0)


__attribute__((__section__(".hwregs")))
static PSI_IWord const mio_init[] = {
	PSI_SET_REGISTER_BANK( IOU_SLCR ),
	PSI_MULTI_WRITE_MASKED_16( IOU_SLCR, MIO_PIN_0, 78, MIO_PIN_MASK,
	                          PSI_PACK_16( MIO_PIN_L0_QSPI, MIO_PIN_L0_QSPI ), // 0 QSPI_CLK, 1 QSPI_DATA[1]
	                          PSI_PACK_16( MIO_PIN_L0_QSPI, MIO_PIN_L0_QSPI ), // 2 QSPI_DATA[2], 3 QSPI_DATA[3]
	                          PSI_PACK_16( MIO_PIN_L0_QSPI, MIO_PIN_L0_QSPI ), // 4 QSPI_DATA[0], 5 QSPI_n_SS_OUT
	                          PSI_PACK_16( MIO_PIN_L3( 4 ), MIO_PIN_GPIO ), // 6 SPI1_CLK, 7
	                          PSI_PACK_16( MIO_PIN_GPIO, MIO_PIN_L3( 4 ) ), // 8, 9 SPI1_n_SS_IN
	                          PSI_PACK_16( MIO_PIN_L3( 4 ), MIO_PIN_L3( 4 ) ), // 10 SPI1_MISO, 11 SPI1_MOSI
	                          PSI_PACK_16( MIO_PIN_GPIO, MIO_PIN_GPIO ), // 12, 13
	                          PSI_PACK_16( MIO_PIN_GPIO, MIO_PIN_GPIO ), // 14, 15
	                          PSI_PACK_16( MIO_PIN_GPIO, MIO_PIN_GPIO ), // 16, 17
	                          PSI_PACK_16( MIO_PIN_GPIO, MIO_PIN_GPIO ), // 18, 19
	                          PSI_PACK_16( MIO_PIN_GPIO, MIO_PIN_GPIO ), // 20, 21
	                          PSI_PACK_16( MIO_PIN_GPIO, MIO_PIN_GPIO ), // 22, 23
	                          PSI_PACK_16( MIO_PIN_L3( 2 ), MIO_PIN_L3( 2 ) ), // 24 I2C1_SCL_INPUT, 25 I2C1_SDA_INPUT
	                          PSI_PACK_16( MIO_PIN_L2( 1 ), MIO_PIN_L2( 3 ) ), // 26 PMU_GPI[0], 27 PMU_GPI[1]
	                          PSI_PACK_16( MIO_PIN_L2( 3 ), MIO_PIN_L2( 3 ) ), // 28 DP_HOT_PLUG_DETECT, 29 DP_AUX_DATA_IN
	                          PSI_PACK_16( MIO_PIN_L2( 3 ), MIO_PIN_L2( 1 ) ), // 30 DP_AUX_PLUG_DETECT, 31 PMU_GPI[5]
	                          PSI_PACK_16( MIO_PIN_L2( 1 ), MIO_PIN_L2( 1 ) ), // 32 PMU_GPO[0], 33 PMU_GPO[1]
	                          PSI_PACK_16( 0, 0 ), // 34, 35 ?
	                          PSI_PACK_16( MIO_PIN_L3( 6 ), MIO_PIN_L3( 6 ) ), // 36 UART1_TXD, 37 UART1_RXD
	                          PSI_PACK_16( MIO_PIN_GPIO, MIO_PIN_L2( 2 ) ), // 38, 39 SD1_DATA[4]
	                          PSI_PACK_16( MIO_PIN_L2( 2 ), MIO_PIN_L2( 2 ) ), // 40 SD1_DATA[5], 41 SD_DATA[6]
	                          PSI_PACK_16( MIO_PIN_L2( 2 ), MIO_PIN_L2( 2 ) ), // 42 SD1_1_DATA[7], 43 SDIO1_BUS_POWER
	                          PSI_PACK_16( MIO_PIN_GPIO, MIO_PIN_L2( 2 ) ), // 44, 45 SDIO1_CD_n
	                          PSI_PACK_16( MIO_PIN_L2( 2 ), MIO_PIN_L2( 2 ) ), // 46 SD1_DATA[0], 47 SD1_DATA[1]
	                          PSI_PACK_16( MIO_PIN_L2( 2 ), MIO_PIN_L2( 2 ) ), // 48 SD1_DATA[2], 49 SD1_DATA[3]
	                          PSI_PACK_16( MIO_PIN_L2( 2 ), MIO_PIN_L2( 2 ) ), // 50 SD1_CMD, 51 SDIO1_CLK_OUT
	                          PSI_PACK_16( MIO_PIN_L1_USB, MIO_PIN_L1_USB ), // 52 USB0_ULPI_CLK_IN, 53 USB0_ULPI_DIR
	                          PSI_PACK_16( MIO_PIN_L1_USB, MIO_PIN_L1_USB ), // 54 USB0_ULPI_DATA[2], 55 USB0_ULPI_NXT
	                          PSI_PACK_16( MIO_PIN_L1_USB, MIO_PIN_L1_USB ), // 56 USB0_ULPI_DATA[0], 57 USB0_ULPI_DATA[1]
	                          PSI_PACK_16( MIO_PIN_L1_USB, MIO_PIN_L1_USB ), // 58 USB0_ULPI_STP, 59 USB0_ULPI_DATA[3]
	                          PSI_PACK_16( MIO_PIN_L1_USB, MIO_PIN_L1_USB ), // 60 USB0_ULPI_DATA[4], 61 USB0_ULPI_DATA[5]
	                          PSI_PACK_16( MIO_PIN_L1_USB, MIO_PIN_L1_USB ), // 62 USB0_ULPI_DATA[6], 63 USB0_ULPI_DATA[7]
	                          PSI_PACK_16( MIO_PIN_L0_GEM, MIO_PIN_L0_GEM ), // 64 GEM3_RGMII_TX_CLK, 65 GEM3_RGMII_TXD[0]
	                          PSI_PACK_16( MIO_PIN_L0_GEM, MIO_PIN_L0_GEM ), // 66 GEM3_RGMII_TXD[1], 67 GEM3_RGMII_TXD[2]
	                          PSI_PACK_16( MIO_PIN_L0_GEM, MIO_PIN_L0_GEM ), // 68 GEM3_RGMII_TXD[3], 69 GEM3_RGMII_TX_CTL
	                          PSI_PACK_16( MIO_PIN_L0_GEM, MIO_PIN_L0_GEM ), // 70 GEM3_RGMII_RX_CLK, 71 GEM3_RGMII_RXD[0]
	                          PSI_PACK_16( MIO_PIN_L0_GEM, MIO_PIN_L0_GEM ), // 72 GEM3_RGMII_RXD[1], 73 GEM3_RGMII_RXD[2]
	                          PSI_PACK_16( MIO_PIN_L0_GEM, MIO_PIN_L0_GEM ), // 74 GEM3_RGMII_RXD[3], 75 GEM3_RGMII_RX_CTL
	                          PSI_PACK_16( MIO_PIN_L2( 3 ), MIO_PIN_L2( 3 ) )  // 76 GEM3_MDIO_CLK, 77 GEM3_MDIO_DATA
														),
	PSI_MULTI_WRITE_32( IOU_SLCR,  MIO_MST_TRI0, 3,
	                      0xD4000000U,  // Tri state enable pin 26, 28, 30, 31
	                      0x00B02020U,  // Tri state enable pin 37, 45, 52, 53, 55
	                      0x00000FC0U   // Tri state enable pin 70, 71, 72, 73, 74, 75
												),
	PSI_MULTI_WRITE_32( IOU_SLCR,  BANK0_CTRL0, 5,
											0x0,           // DRIVE
											0x03FFFFFFU,   // SCHMITT_CMOS_N
											0x0,           // PULL_HIGH_LOW_N
											0x0,           // PULL_ENABLE
	                    0x03FFFFFFU    // SLOW_FAST_SLEW_N
											),
	PSI_MULTI_WRITE_32( IOU_SLCR,  BANK1_CTRL0, 5,
	                    0x00080835U,   // DRIVE
	                    0x03FFFFFFU,   // SCHMITT_CMOS_N
	                    0x03FFFFFFU,   // PULL_HIGH_LOW_N
	                    0x03FFFFFFU,   // PULL_ENABLE
	                    0x03F7F7CAU    // SLOW_FAST_SLEW_N
	),
	PSI_MULTI_WRITE_32( IOU_SLCR,  BANK2_CTRL0, 5,
	                    0x00FC000BU,   // DRIVE
	                    0x03FFFFFFU,   // SCHMITT_CMOS_N
	                    0x00000000U,   // PULL_HIGH_LOW_N
	                    0x03FFFFFFU,   // PULL_ENABLE
	                    0x03FFFFFFU    // SLOW_FAST_SLEW_N
	),

	PSI_WRITE_32( IOU_SLCR, MIO_LOOPBACK, 0), // Loop backs off

	PSI_END_PROGRAM
};

void mioRunInitProgram() {
	psi_RunRegisterProgram( mio_init );
}

#include "psu_init.h"

unsigned long psu_mio_init_data( void ) {
	/*
	* MIO PROGRAMMING
	*/
	/*
	* Register : MIO_PIN_0 @ 0XFF180000

	* Level 0 Mux Select 0= Level 1 Mux Output 1= qspi, Output, qspi_sclk_out-
	*  (QSPI Clock)
	*  PSU_IOU_SLCR_MIO_PIN_0_L0_SEL                               1

	* Level 1 Mux Select 0= Level 2 Mux Output 1= Not Used
	*  PSU_IOU_SLCR_MIO_PIN_0_L1_SEL                               0

	* Level 2 Mux Select 0= Level 3 Mux Output 1= Not Used 2= test_scan, Input
	* , test_scan_in[0]- (Test Scan Port) = test_scan, Output, test_scan_out[0
	* ]- (Test Scan Port) 3= Not Used
	*  PSU_IOU_SLCR_MIO_PIN_0_L2_SEL                               0

	* Level 3 Mux Select 0= gpio0, Input, gpio_0_pin_in[0]- (GPIO bank 0) 0= g
	* pio0, Output, gpio_0_pin_out[0]- (GPIO bank 0) 1= can1, Output, can1_phy
	* _tx- (Can TX signal) 2= i2c1, Input, i2c1_scl_input- (SCL signal) 2= i2c
	* 1, Output, i2c1_scl_out- (SCL signal) 3= pjtag, Input, pjtag_tck- (PJTAG
	*  TCK) 4= spi0, Input, spi0_sclk_in- (SPI Clock) 4= spi0, Output, spi0_sc
	* lk_out- (SPI Clock) 5= ttc3, Input, ttc3_clk_in- (TTC Clock) 6= ua1, Out
	* put, ua1_txd- (UART transmitter serial output) 7= trace, Output, trace_c
	* lk- (Trace Port Clock)
	*  PSU_IOU_SLCR_MIO_PIN_0_L3_SEL                               0

	* Configures MIO Pin 0 peripheral interface mapping. S
	* (OFFSET, MASK, VALUE)      (0XFF180000, 0x000000FEU ,0x00000002U)
	*/
	PSU_Mask_Write( IOU_SLCR_MIO_PIN_0_OFFSET, 0x000000FEU, 0x00000002U );
	/*##################################################################### */

	/*
	* Register : MIO_PIN_1 @ 0XFF180004

	* Level 0 Mux Select 0= Level 1 Mux Output 1= qspi, Input, qspi_mi_mi1- (Q
	* SPI Databus) 1= qspi, Output, qspi_so_mo1- (QSPI Databus)
	*  PSU_IOU_SLCR_MIO_PIN_1_L0_SEL                               1

	* Level 1 Mux Select 0= Level 2 Mux Output 1= Not Used
	*  PSU_IOU_SLCR_MIO_PIN_1_L1_SEL                               0

	* Level 2 Mux Select 0= Level 3 Mux Output 1= Not Used 2= test_scan, Input
	* , test_scan_in[1]- (Test Scan Port) = test_scan, Output, test_scan_out[1
	* ]- (Test Scan Port) 3= Not Used
	*  PSU_IOU_SLCR_MIO_PIN_1_L2_SEL                               0

	* Level 3 Mux Select 0= gpio0, Input, gpio_0_pin_in[1]- (GPIO bank 0) 0= g
	* pio0, Output, gpio_0_pin_out[1]- (GPIO bank 0) 1= can1, Input, can1_phy_
	* rx- (Can RX signal) 2= i2c1, Input, i2c1_sda_input- (SDA signal) 2= i2c1
	* , Output, i2c1_sda_out- (SDA signal) 3= pjtag, Input, pjtag_tdi- (PJTAG
	* TDI) 4= spi0, Output, spi0_n_ss_out[2]- (SPI Master Selects) 5= ttc3, Ou
	* tput, ttc3_wave_out- (TTC Waveform Clock) 6= ua1, Input, ua1_rxd- (UART
	* receiver serial input) 7= trace, Output, trace_ctl- (Trace Port Control
	* Signal)
	*  PSU_IOU_SLCR_MIO_PIN_1_L3_SEL                               0

	* Configures MIO Pin 1 peripheral interface mapping
	* (OFFSET, MASK, VALUE)      (0XFF180004, 0x000000FEU ,0x00000002U)
	*/
	PSU_Mask_Write( IOU_SLCR_MIO_PIN_1_OFFSET, 0x000000FEU, 0x00000002U );
	/*##################################################################### */

	/*
	* Register : MIO_PIN_2 @ 0XFF180008

	* Level 0 Mux Select 0= Level 1 Mux Output 1= qspi, Input, qspi_mi2- (QSPI
	*  Databus) 1= qspi, Output, qspi_mo2- (QSPI Databus)
	*  PSU_IOU_SLCR_MIO_PIN_2_L0_SEL                               1

	* Level 1 Mux Select 0= Level 2 Mux Output 1= Not Used
	*  PSU_IOU_SLCR_MIO_PIN_2_L1_SEL                               0

	* Level 2 Mux Select 0= Level 3 Mux Output 1= Not Used 2= test_scan, Input
	* , test_scan_in[2]- (Test Scan Port) = test_scan, Output, test_scan_out[2
	* ]- (Test Scan Port) 3= Not Used
	*  PSU_IOU_SLCR_MIO_PIN_2_L2_SEL                               0

	* Level 3 Mux Select 0= gpio0, Input, gpio_0_pin_in[2]- (GPIO bank 0) 0= g
	* pio0, Output, gpio_0_pin_out[2]- (GPIO bank 0) 1= can0, Input, can0_phy_
	* rx- (Can RX signal) 2= i2c0, Input, i2c0_scl_input- (SCL signal) 2= i2c0
	* , Output, i2c0_scl_out- (SCL signal) 3= pjtag, Output, pjtag_tdo- (PJTAG
	*  TDO) 4= spi0, Output, spi0_n_ss_out[1]- (SPI Master Selects) 5= ttc2, I
	* nput, ttc2_clk_in- (TTC Clock) 6= ua0, Input, ua0_rxd- (UART receiver se
	* rial input) 7= trace, Output, tracedq[0]- (Trace Port Databus)
	*  PSU_IOU_SLCR_MIO_PIN_2_L3_SEL                               0

	* Configures MIO Pin 2 peripheral interface mapping
	* (OFFSET, MASK, VALUE)      (0XFF180008, 0x000000FEU ,0x00000002U)
	*/
	PSU_Mask_Write( IOU_SLCR_MIO_PIN_2_OFFSET, 0x000000FEU, 0x00000002U );
	/*##################################################################### */

	/*
	* Register : MIO_PIN_3 @ 0XFF18000C

	* Level 0 Mux Select 0= Level 1 Mux Output 1= qspi, Input, qspi_mi3- (QSPI
	*  Databus) 1= qspi, Output, qspi_mo3- (QSPI Databus)
	*  PSU_IOU_SLCR_MIO_PIN_3_L0_SEL                               1

	* Level 1 Mux Select 0= Level 2 Mux Output 1= Not Used
	*  PSU_IOU_SLCR_MIO_PIN_3_L1_SEL                               0

	* Level 2 Mux Select 0= Level 3 Mux Output 1= Not Used 2= test_scan, Input
	* , test_scan_in[3]- (Test Scan Port) = test_scan, Output, test_scan_out[3
	* ]- (Test Scan Port) 3= Not Used
	*  PSU_IOU_SLCR_MIO_PIN_3_L2_SEL                               0

	* Level 3 Mux Select 0= gpio0, Input, gpio_0_pin_in[3]- (GPIO bank 0) 0= g
	* pio0, Output, gpio_0_pin_out[3]- (GPIO bank 0) 1= can0, Output, can0_phy
	* _tx- (Can TX signal) 2= i2c0, Input, i2c0_sda_input- (SDA signal) 2= i2c
	* 0, Output, i2c0_sda_out- (SDA signal) 3= pjtag, Input, pjtag_tms- (PJTAG
	*  TMS) 4= spi0, Input, spi0_n_ss_in- (SPI Master Selects) 4= spi0, Output
	* , spi0_n_ss_out[0]- (SPI Master Selects) 5= ttc2, Output, ttc2_wave_out-
	*  (TTC Waveform Clock) 6= ua0, Output, ua0_txd- (UART transmitter serial
	* output) 7= trace, Output, tracedq[1]- (Trace Port Databus)
	*  PSU_IOU_SLCR_MIO_PIN_3_L3_SEL                               0

	* Configures MIO Pin 3 peripheral interface mapping
	* (OFFSET, MASK, VALUE)      (0XFF18000C, 0x000000FEU ,0x00000002U)
	*/
	PSU_Mask_Write( IOU_SLCR_MIO_PIN_3_OFFSET, 0x000000FEU, 0x00000002U );
	/*##################################################################### */

	/*
	* Register : MIO_PIN_4 @ 0XFF180010

	* Level 0 Mux Select 0= Level 1 Mux Output 1= qspi, Output, qspi_mo_mo0- (
	* QSPI Databus) 1= qspi, Input, qspi_si_mi0- (QSPI Databus)
	*  PSU_IOU_SLCR_MIO_PIN_4_L0_SEL                               1

	* Level 1 Mux Select 0= Level 2 Mux Output 1= Not Used
	*  PSU_IOU_SLCR_MIO_PIN_4_L1_SEL                               0

	* Level 2 Mux Select 0= Level 3 Mux Output 1= Not Used 2= test_scan, Input
	* , test_scan_in[4]- (Test Scan Port) = test_scan, Output, test_scan_out[4
	* ]- (Test Scan Port) 3= Not Used
	*  PSU_IOU_SLCR_MIO_PIN_4_L2_SEL                               0

	* Level 3 Mux Select 0= gpio0, Input, gpio_0_pin_in[4]- (GPIO bank 0) 0= g
	* pio0, Output, gpio_0_pin_out[4]- (GPIO bank 0) 1= can1, Output, can1_phy
	* _tx- (Can TX signal) 2= i2c1, Input, i2c1_scl_input- (SCL signal) 2= i2c
	* 1, Output, i2c1_scl_out- (SCL signal) 3= swdt1, Input, swdt1_clk_in- (Wa
	* tch Dog Timer Input clock) 4= spi0, Input, spi0_mi- (MISO signal) 4= spi
	* 0, Output, spi0_so- (MISO signal) 5= ttc1, Input, ttc1_clk_in- (TTC Cloc
	* k) 6= ua1, Output, ua1_txd- (UART transmitter serial output) 7= trace, O
	* utput, tracedq[2]- (Trace Port Databus)
	*  PSU_IOU_SLCR_MIO_PIN_4_L3_SEL                               0

	* Configures MIO Pin 4 peripheral interface mapping
	* (OFFSET, MASK, VALUE)      (0XFF180010, 0x000000FEU ,0x00000002U)
	*/
	PSU_Mask_Write( IOU_SLCR_MIO_PIN_4_OFFSET, 0x000000FEU, 0x00000002U );
	/*##################################################################### */

	/*
	* Register : MIO_PIN_5 @ 0XFF180014

	* Level 0 Mux Select 0= Level 1 Mux Output 1= qspi, Output, qspi_n_ss_out-
	*  (QSPI Slave Select)
	*  PSU_IOU_SLCR_MIO_PIN_5_L0_SEL                               1

	* Level 1 Mux Select 0= Level 2 Mux Output 1= Not Used
	*  PSU_IOU_SLCR_MIO_PIN_5_L1_SEL                               0

	* Level 2 Mux Select 0= Level 3 Mux Output 1= Not Used 2= test_scan, Input
	* , test_scan_in[5]- (Test Scan Port) = test_scan, Output, test_scan_out[5
	* ]- (Test Scan Port) 3= Not Used
	*  PSU_IOU_SLCR_MIO_PIN_5_L2_SEL                               0

	* Level 3 Mux Select 0= gpio0, Input, gpio_0_pin_in[5]- (GPIO bank 0) 0= g
	* pio0, Output, gpio_0_pin_out[5]- (GPIO bank 0) 1= can1, Input, can1_phy_
	* rx- (Can RX signal) 2= i2c1, Input, i2c1_sda_input- (SDA signal) 2= i2c1
	* , Output, i2c1_sda_out- (SDA signal) 3= swdt1, Output, swdt1_rst_out- (W
	* atch Dog Timer Output clock) 4= spi0, Output, spi0_mo- (MOSI signal) 4=
	* spi0, Input, spi0_si- (MOSI signal) 5= ttc1, Output, ttc1_wave_out- (TTC
	*  Waveform Clock) 6= ua1, Input, ua1_rxd- (UART receiver serial input) 7=
	*  trace, Output, tracedq[3]- (Trace Port Databus)
	*  PSU_IOU_SLCR_MIO_PIN_5_L3_SEL                               0

	* Configures MIO Pin 5 peripheral interface mapping
	* (OFFSET, MASK, VALUE)      (0XFF180014, 0x000000FEU ,0x00000002U)
	*/
	PSU_Mask_Write( IOU_SLCR_MIO_PIN_5_OFFSET, 0x000000FEU, 0x00000002U );
	/*##################################################################### */

	/*
	* Register : MIO_PIN_6 @ 0XFF180018

	* Level 0 Mux Select 0= Level 1 Mux Output 1= qspi, Output, qspi_clk_for_l
	* pbk- (QSPI Clock to be fed-back)
	*  PSU_IOU_SLCR_MIO_PIN_6_L0_SEL                               0

	* Level 1 Mux Select 0= Level 2 Mux Output 1= Not Used
	*  PSU_IOU_SLCR_MIO_PIN_6_L1_SEL                               0

	* Level 2 Mux Select 0= Level 3 Mux Output 1= Not Used 2= test_scan, Input
	* , test_scan_in[6]- (Test Scan Port) = test_scan, Output, test_scan_out[6
	* ]- (Test Scan Port) 3= Not Used
	*  PSU_IOU_SLCR_MIO_PIN_6_L2_SEL                               0

	* Level 3 Mux Select 0= gpio0, Input, gpio_0_pin_in[6]- (GPIO bank 0) 0= g
	* pio0, Output, gpio_0_pin_out[6]- (GPIO bank 0) 1= can0, Input, can0_phy_
	* rx- (Can RX signal) 2= i2c0, Input, i2c0_scl_input- (SCL signal) 2= i2c0
	* , Output, i2c0_scl_out- (SCL signal) 3= swdt0, Input, swdt0_clk_in- (Wat
	* ch Dog Timer Input clock) 4= spi1, Input, spi1_sclk_in- (SPI Clock) 4= s
	* pi1, Output, spi1_sclk_out- (SPI Clock) 5= ttc0, Input, ttc0_clk_in- (TT
	* C Clock) 6= ua0, Input, ua0_rxd- (UART receiver serial input) 7= trace,
	* Output, tracedq[4]- (Trace Port Databus)
	*  PSU_IOU_SLCR_MIO_PIN_6_L3_SEL                               4

	* Configures MIO Pin 6 peripheral interface mapping
	* (OFFSET, MASK, VALUE)      (0XFF180018, 0x000000FEU ,0x00000080U)
	*/
	PSU_Mask_Write( IOU_SLCR_MIO_PIN_6_OFFSET, 0x000000FEU, 0x00000080U );
	/*##################################################################### */

	/*
	* Register : MIO_PIN_7 @ 0XFF18001C

	* Level 0 Mux Select 0= Level 1 Mux Output 1= qspi, Output, qspi_n_ss_out_
	* upper- (QSPI Slave Select upper)
	*  PSU_IOU_SLCR_MIO_PIN_7_L0_SEL                               0

	* Level 1 Mux Select 0= Level 2 Mux Output 1= Not Used
	*  PSU_IOU_SLCR_MIO_PIN_7_L1_SEL                               0

	* Level 2 Mux Select 0= Level 3 Mux Output 1= Not Used 2= test_scan, Input
	* , test_scan_in[7]- (Test Scan Port) = test_scan, Output, test_scan_out[7
	* ]- (Test Scan Port) 3= Not Used
	*  PSU_IOU_SLCR_MIO_PIN_7_L2_SEL                               0

	* Level 3 Mux Select 0= gpio0, Input, gpio_0_pin_in[7]- (GPIO bank 0) 0= g
	* pio0, Output, gpio_0_pin_out[7]- (GPIO bank 0) 1= can0, Output, can0_phy
	* _tx- (Can TX signal) 2= i2c0, Input, i2c0_sda_input- (SDA signal) 2= i2c
	* 0, Output, i2c0_sda_out- (SDA signal) 3= swdt0, Output, swdt0_rst_out- (
	* Watch Dog Timer Output clock) 4= spi1, Output, spi1_n_ss_out[2]- (SPI Ma
	* ster Selects) 5= ttc0, Output, ttc0_wave_out- (TTC Waveform Clock) 6= ua
	* 0, Output, ua0_txd- (UART transmitter serial output) 7= trace, Output, t
	* racedq[5]- (Trace Port Databus)
	*  PSU_IOU_SLCR_MIO_PIN_7_L3_SEL                               0

	* Configures MIO Pin 7 peripheral interface mapping
	* (OFFSET, MASK, VALUE)      (0XFF18001C, 0x000000FEU ,0x00000000U)
	*/
	PSU_Mask_Write( IOU_SLCR_MIO_PIN_7_OFFSET, 0x000000FEU, 0x00000000U );
	/*##################################################################### */

	/*
	* Register : MIO_PIN_8 @ 0XFF180020

	* Level 0 Mux Select 0= Level 1 Mux Output 1= qspi, Input, qspi_mi_upper[0
	* ]- (QSPI Upper Databus) 1= qspi, Output, qspi_mo_upper[0]- (QSPI Upper D
	* atabus)
	*  PSU_IOU_SLCR_MIO_PIN_8_L0_SEL                               0

	* Level 1 Mux Select 0= Level 2 Mux Output 1= Not Used
	*  PSU_IOU_SLCR_MIO_PIN_8_L1_SEL                               0

	* Level 2 Mux Select 0= Level 3 Mux Output 1= Not Used 2= test_scan, Input
	* , test_scan_in[8]- (Test Scan Port) = test_scan, Output, test_scan_out[8
	* ]- (Test Scan Port) 3= Not Used
	*  PSU_IOU_SLCR_MIO_PIN_8_L2_SEL                               0

	* Level 3 Mux Select 0= gpio0, Input, gpio_0_pin_in[8]- (GPIO bank 0) 0= g
	* pio0, Output, gpio_0_pin_out[8]- (GPIO bank 0) 1= can1, Output, can1_phy
	* _tx- (Can TX signal) 2= i2c1, Input, i2c1_scl_input- (SCL signal) 2= i2c
	* 1, Output, i2c1_scl_out- (SCL signal) 3= swdt1, Input, swdt1_clk_in- (Wa
	* tch Dog Timer Input clock) 4= spi1, Output, spi1_n_ss_out[1]- (SPI Maste
	* r Selects) 5= ttc3, Input, ttc3_clk_in- (TTC Clock) 6= ua1, Output, ua1_
	* txd- (UART transmitter serial output) 7= trace, Output, tracedq[6]- (Tra
	* ce Port Databus)
	*  PSU_IOU_SLCR_MIO_PIN_8_L3_SEL                               0

	* Configures MIO Pin 8 peripheral interface mapping
	* (OFFSET, MASK, VALUE)      (0XFF180020, 0x000000FEU ,0x00000000U)
	*/
	PSU_Mask_Write( IOU_SLCR_MIO_PIN_8_OFFSET, 0x000000FEU, 0x00000000U );
	/*##################################################################### */

	/*
	* Register : MIO_PIN_9 @ 0XFF180024

	* Level 0 Mux Select 0= Level 1 Mux Output 1= qspi, Input, qspi_mi_upper[1
	* ]- (QSPI Upper Databus) 1= qspi, Output, qspi_mo_upper[1]- (QSPI Upper D
	* atabus)
	*  PSU_IOU_SLCR_MIO_PIN_9_L0_SEL                               0

	* Level 1 Mux Select 0= Level 2 Mux Output 1= nand, Output, nfc_ce[1]- (NA
	* ND chip enable)
	*  PSU_IOU_SLCR_MIO_PIN_9_L1_SEL                               0

	* Level 2 Mux Select 0= Level 3 Mux Output 1= Not Used 2= test_scan, Input
	* , test_scan_in[9]- (Test Scan Port) = test_scan, Output, test_scan_out[9
	* ]- (Test Scan Port) 3= Not Used
	*  PSU_IOU_SLCR_MIO_PIN_9_L2_SEL                               0

	* Level 3 Mux Select 0= gpio0, Input, gpio_0_pin_in[9]- (GPIO bank 0) 0= g
	* pio0, Output, gpio_0_pin_out[9]- (GPIO bank 0) 1= can1, Input, can1_phy_
	* rx- (Can RX signal) 2= i2c1, Input, i2c1_sda_input- (SDA signal) 2= i2c1
	* , Output, i2c1_sda_out- (SDA signal) 3= swdt1, Output, swdt1_rst_out- (W
	* atch Dog Timer Output clock) 4= spi1, Input, spi1_n_ss_in- (SPI Master S
	* elects) 4= spi1, Output, spi1_n_ss_out[0]- (SPI Master Selects) 5= ttc3,
	*  Output, ttc3_wave_out- (TTC Waveform Clock) 6= ua1, Input, ua1_rxd- (UA
	* RT receiver serial input) 7= trace, Output, tracedq[7]- (Trace Port Data
	* bus)
	*  PSU_IOU_SLCR_MIO_PIN_9_L3_SEL                               4

	* Configures MIO Pin 9 peripheral interface mapping
	* (OFFSET, MASK, VALUE)      (0XFF180024, 0x000000FEU ,0x00000080U)
	*/
	PSU_Mask_Write( IOU_SLCR_MIO_PIN_9_OFFSET, 0x000000FEU, 0x00000080U );
	/*##################################################################### */

	/*
	* Register : MIO_PIN_10 @ 0XFF180028

	* Level 0 Mux Select 0= Level 1 Mux Output 1= qspi, Input, qspi_mi_upper[2
	* ]- (QSPI Upper Databus) 1= qspi, Output, qspi_mo_upper[2]- (QSPI Upper D
	* atabus)
	*  PSU_IOU_SLCR_MIO_PIN_10_L0_SEL                              0

	* Level 1 Mux Select 0= Level 2 Mux Output 1= nand, Input, nfc_rb_n[0]- (N
	* AND Ready/Busy)
	*  PSU_IOU_SLCR_MIO_PIN_10_L1_SEL                              0

	* Level 2 Mux Select 0= Level 3 Mux Output 1= Not Used 2= test_scan, Input
	* , test_scan_in[10]- (Test Scan Port) = test_scan, Output, test_scan_out[
	* 10]- (Test Scan Port) 3= Not Used
	*  PSU_IOU_SLCR_MIO_PIN_10_L2_SEL                              0

	* Level 3 Mux Select 0= gpio0, Input, gpio_0_pin_in[10]- (GPIO bank 0) 0=
	* gpio0, Output, gpio_0_pin_out[10]- (GPIO bank 0) 1= can0, Input, can0_ph
	* y_rx- (Can RX signal) 2= i2c0, Input, i2c0_scl_input- (SCL signal) 2= i2
	* c0, Output, i2c0_scl_out- (SCL signal) 3= swdt0, Input, swdt0_clk_in- (W
	* atch Dog Timer Input clock) 4= spi1, Input, spi1_mi- (MISO signal) 4= sp
	* i1, Output, spi1_so- (MISO signal) 5= ttc2, Input, ttc2_clk_in- (TTC Clo
	* ck) 6= ua0, Input, ua0_rxd- (UART receiver serial input) 7= trace, Outpu
	* t, tracedq[8]- (Trace Port Databus)
	*  PSU_IOU_SLCR_MIO_PIN_10_L3_SEL                              4

	* Configures MIO Pin 10 peripheral interface mapping
	* (OFFSET, MASK, VALUE)      (0XFF180028, 0x000000FEU ,0x00000080U)
	*/
	PSU_Mask_Write( IOU_SLCR_MIO_PIN_10_OFFSET, 0x000000FEU, 0x00000080U );
	/*##################################################################### */

	/*
	* Register : MIO_PIN_11 @ 0XFF18002C

	* Level 0 Mux Select 0= Level 1 Mux Output 1= qspi, Input, qspi_mi_upper[3
	* ]- (QSPI Upper Databus) 1= qspi, Output, qspi_mo_upper[3]- (QSPI Upper D
	* atabus)
	*  PSU_IOU_SLCR_MIO_PIN_11_L0_SEL                              0

	* Level 1 Mux Select 0= Level 2 Mux Output 1= nand, Input, nfc_rb_n[1]- (N
	* AND Ready/Busy)
	*  PSU_IOU_SLCR_MIO_PIN_11_L1_SEL                              0

	* Level 2 Mux Select 0= Level 3 Mux Output 1= Not Used 2= test_scan, Input
	* , test_scan_in[11]- (Test Scan Port) = test_scan, Output, test_scan_out[
	* 11]- (Test Scan Port) 3= Not Used
	*  PSU_IOU_SLCR_MIO_PIN_11_L2_SEL                              0

	* Level 3 Mux Select 0= gpio0, Input, gpio_0_pin_in[11]- (GPIO bank 0) 0=
	* gpio0, Output, gpio_0_pin_out[11]- (GPIO bank 0) 1= can0, Output, can0_p
	* hy_tx- (Can TX signal) 2= i2c0, Input, i2c0_sda_input- (SDA signal) 2= i
	* 2c0, Output, i2c0_sda_out- (SDA signal) 3= swdt0, Output, swdt0_rst_out-
	*  (Watch Dog Timer Output clock) 4= spi1, Output, spi1_mo- (MOSI signal)
	* 4= spi1, Input, spi1_si- (MOSI signal) 5= ttc2, Output, ttc2_wave_out- (
	* TTC Waveform Clock) 6= ua0, Output, ua0_txd- (UART transmitter serial ou
	* tput) 7= trace, Output, tracedq[9]- (Trace Port Databus)
	*  PSU_IOU_SLCR_MIO_PIN_11_L3_SEL                              4

	* Configures MIO Pin 11 peripheral interface mapping
	* (OFFSET, MASK, VALUE)      (0XFF18002C, 0x000000FEU ,0x00000080U)
	*/
	PSU_Mask_Write( IOU_SLCR_MIO_PIN_11_OFFSET, 0x000000FEU, 0x00000080U );
	/*##################################################################### */

	/*
	* Register : MIO_PIN_12 @ 0XFF180030

	* Level 0 Mux Select 0= Level 1 Mux Output 1= qspi, Output, qspi_sclk_out_
	* upper- (QSPI Upper Clock)
	*  PSU_IOU_SLCR_MIO_PIN_12_L0_SEL                              0

	* Level 1 Mux Select 0= Level 2 Mux Output 1= nand, Input, nfc_dqs_in- (NA
	* ND Strobe) 1= nand, Output, nfc_dqs_out- (NAND Strobe)
	*  PSU_IOU_SLCR_MIO_PIN_12_L1_SEL                              0

	* Level 2 Mux Select 0= Level 3 Mux Output 1= Not Used 2= test_scan, Input
	* , test_scan_in[12]- (Test Scan Port) = test_scan, Output, test_scan_out[
	* 12]- (Test Scan Port) 3= Not Used
	*  PSU_IOU_SLCR_MIO_PIN_12_L2_SEL                              0

	* Level 3 Mux Select 0= gpio0, Input, gpio_0_pin_in[12]- (GPIO bank 0) 0=
	* gpio0, Output, gpio_0_pin_out[12]- (GPIO bank 0) 1= can1, Output, can1_p
	* hy_tx- (Can TX signal) 2= i2c1, Input, i2c1_scl_input- (SCL signal) 2= i
	* 2c1, Output, i2c1_scl_out- (SCL signal) 3= pjtag, Input, pjtag_tck- (PJT
	* AG TCK) 4= spi0, Input, spi0_sclk_in- (SPI Clock) 4= spi0, Output, spi0_
	* sclk_out- (SPI Clock) 5= ttc1, Input, ttc1_clk_in- (TTC Clock) 6= ua1, O
	* utput, ua1_txd- (UART transmitter serial output) 7= trace, Output, trace
	* dq[10]- (Trace Port Databus)
	*  PSU_IOU_SLCR_MIO_PIN_12_L3_SEL                              0

	* Configures MIO Pin 12 peripheral interface mapping
	* (OFFSET, MASK, VALUE)      (0XFF180030, 0x000000FEU ,0x00000000U)
	*/
	PSU_Mask_Write( IOU_SLCR_MIO_PIN_12_OFFSET, 0x000000FEU, 0x00000000U );
	/*##################################################################### */

	/*
	* Register : MIO_PIN_13 @ 0XFF180034

	* Level 0 Mux Select 0= Level 1 Mux Output 1= Not Used
	*  PSU_IOU_SLCR_MIO_PIN_13_L0_SEL                              0

	* Level 1 Mux Select 0= Level 2 Mux Output 1= nand, Output, nfc_ce[0]- (NA
	* ND chip enable)
	*  PSU_IOU_SLCR_MIO_PIN_13_L1_SEL                              0

	* Level 2 Mux Select 0= Level 3 Mux Output 1= sd0, Input, sd0_data_in[0]-
	* (8-bit Data bus) = sd0, Output, sdio0_data_out[0]- (8-bit Data bus) 2= t
	* est_scan, Input, test_scan_in[13]- (Test Scan Port) = test_scan, Output,
	*  test_scan_out[13]- (Test Scan Port) 3= Not Used
	*  PSU_IOU_SLCR_MIO_PIN_13_L2_SEL                              0

	* Level 3 Mux Select 0= gpio0, Input, gpio_0_pin_in[13]- (GPIO bank 0) 0=
	* gpio0, Output, gpio_0_pin_out[13]- (GPIO bank 0) 1= can1, Input, can1_ph
	* y_rx- (Can RX signal) 2= i2c1, Input, i2c1_sda_input- (SDA signal) 2= i2
	* c1, Output, i2c1_sda_out- (SDA signal) 3= pjtag, Input, pjtag_tdi- (PJTA
	* G TDI) 4= spi0, Output, spi0_n_ss_out[2]- (SPI Master Selects) 5= ttc1,
	* Output, ttc1_wave_out- (TTC Waveform Clock) 6= ua1, Input, ua1_rxd- (UAR
	* T receiver serial input) 7= trace, Output, tracedq[11]- (Trace Port Data
	* bus)
	*  PSU_IOU_SLCR_MIO_PIN_13_L3_SEL                              0

	* Configures MIO Pin 13 peripheral interface mapping
	* (OFFSET, MASK, VALUE)      (0XFF180034, 0x000000FEU ,0x00000000U)
	*/
	PSU_Mask_Write( IOU_SLCR_MIO_PIN_13_OFFSET, 0x000000FEU, 0x00000000U );
	/*##################################################################### */

	/*
	* Register : MIO_PIN_14 @ 0XFF180038

	* Level 0 Mux Select 0= Level 1 Mux Output 1= Not Used
	*  PSU_IOU_SLCR_MIO_PIN_14_L0_SEL                              0

	* Level 1 Mux Select 0= Level 2 Mux Output 1= nand, Output, nfc_cle- (NAND
	*  Command Latch Enable)
	*  PSU_IOU_SLCR_MIO_PIN_14_L1_SEL                              0

	* Level 2 Mux Select 0= Level 3 Mux Output 1= sd0, Input, sd0_data_in[1]-
	* (8-bit Data bus) = sd0, Output, sdio0_data_out[1]- (8-bit Data bus) 2= t
	* est_scan, Input, test_scan_in[14]- (Test Scan Port) = test_scan, Output,
	*  test_scan_out[14]- (Test Scan Port) 3= Not Used
	*  PSU_IOU_SLCR_MIO_PIN_14_L2_SEL                              0

	* Level 3 Mux Select 0= gpio0, Input, gpio_0_pin_in[14]- (GPIO bank 0) 0=
	* gpio0, Output, gpio_0_pin_out[14]- (GPIO bank 0) 1= can0, Input, can0_ph
	* y_rx- (Can RX signal) 2= i2c0, Input, i2c0_scl_input- (SCL signal) 2= i2
	* c0, Output, i2c0_scl_out- (SCL signal) 3= pjtag, Output, pjtag_tdo- (PJT
	* AG TDO) 4= spi0, Output, spi0_n_ss_out[1]- (SPI Master Selects) 5= ttc0,
	*  Input, ttc0_clk_in- (TTC Clock) 6= ua0, Input, ua0_rxd- (UART receiver
	* serial input) 7= trace, Output, tracedq[12]- (Trace Port Databus)
	*  PSU_IOU_SLCR_MIO_PIN_14_L3_SEL                              0

	* Configures MIO Pin 14 peripheral interface mapping
	* (OFFSET, MASK, VALUE)      (0XFF180038, 0x000000FEU ,0x00000000U)
	*/
	PSU_Mask_Write( IOU_SLCR_MIO_PIN_14_OFFSET, 0x000000FEU, 0x00000000U );
	/*##################################################################### */

	/*
	* Register : MIO_PIN_15 @ 0XFF18003C

	* Level 0 Mux Select 0= Level 1 Mux Output 1= Not Used
	*  PSU_IOU_SLCR_MIO_PIN_15_L0_SEL                              0

	* Level 1 Mux Select 0= Level 2 Mux Output 1= nand, Output, nfc_ale- (NAND
	*  Address Latch Enable)
	*  PSU_IOU_SLCR_MIO_PIN_15_L1_SEL                              0

	* Level 2 Mux Select 0= Level 3 Mux Output 1= sd0, Input, sd0_data_in[2]-
	* (8-bit Data bus) = sd0, Output, sdio0_data_out[2]- (8-bit Data bus) 2= t
	* est_scan, Input, test_scan_in[15]- (Test Scan Port) = test_scan, Output,
	*  test_scan_out[15]- (Test Scan Port) 3= Not Used
	*  PSU_IOU_SLCR_MIO_PIN_15_L2_SEL                              0

	* Level 3 Mux Select 0= gpio0, Input, gpio_0_pin_in[15]- (GPIO bank 0) 0=
	* gpio0, Output, gpio_0_pin_out[15]- (GPIO bank 0) 1= can0, Output, can0_p
	* hy_tx- (Can TX signal) 2= i2c0, Input, i2c0_sda_input- (SDA signal) 2= i
	* 2c0, Output, i2c0_sda_out- (SDA signal) 3= pjtag, Input, pjtag_tms- (PJT
	* AG TMS) 4= spi0, Input, spi0_n_ss_in- (SPI Master Selects) 4= spi0, Outp
	* ut, spi0_n_ss_out[0]- (SPI Master Selects) 5= ttc0, Output, ttc0_wave_ou
	* t- (TTC Waveform Clock) 6= ua0, Output, ua0_txd- (UART transmitter seria
	* l output) 7= trace, Output, tracedq[13]- (Trace Port Databus)
	*  PSU_IOU_SLCR_MIO_PIN_15_L3_SEL                              0

	* Configures MIO Pin 15 peripheral interface mapping
	* (OFFSET, MASK, VALUE)      (0XFF18003C, 0x000000FEU ,0x00000000U)
	*/
	PSU_Mask_Write( IOU_SLCR_MIO_PIN_15_OFFSET, 0x000000FEU, 0x00000000U );
	/*##################################################################### */

	/*
	* Register : MIO_PIN_16 @ 0XFF180040

	* Level 0 Mux Select 0= Level 1 Mux Output 1= Not Used
	*  PSU_IOU_SLCR_MIO_PIN_16_L0_SEL                              0

	* Level 1 Mux Select 0= Level 2 Mux Output 1= nand, Input, nfc_dq_in[0]- (
	* NAND Data Bus) 1= nand, Output, nfc_dq_out[0]- (NAND Data Bus)
	*  PSU_IOU_SLCR_MIO_PIN_16_L1_SEL                              0

	* Level 2 Mux Select 0= Level 3 Mux Output 1= sd0, Input, sd0_data_in[3]-
	* (8-bit Data bus) = sd0, Output, sdio0_data_out[3]- (8-bit Data bus) 2= t
	* est_scan, Input, test_scan_in[16]- (Test Scan Port) = test_scan, Output,
	*  test_scan_out[16]- (Test Scan Port) 3= Not Used
	*  PSU_IOU_SLCR_MIO_PIN_16_L2_SEL                              0

	* Level 3 Mux Select 0= gpio0, Input, gpio_0_pin_in[16]- (GPIO bank 0) 0=
	* gpio0, Output, gpio_0_pin_out[16]- (GPIO bank 0) 1= can1, Output, can1_p
	* hy_tx- (Can TX signal) 2= i2c1, Input, i2c1_scl_input- (SCL signal) 2= i
	* 2c1, Output, i2c1_scl_out- (SCL signal) 3= swdt1, Input, swdt1_clk_in- (
	* Watch Dog Timer Input clock) 4= spi0, Input, spi0_mi- (MISO signal) 4= s
	* pi0, Output, spi0_so- (MISO signal) 5= ttc3, Input, ttc3_clk_in- (TTC Cl
	* ock) 6= ua1, Output, ua1_txd- (UART transmitter serial output) 7= trace,
	*  Output, tracedq[14]- (Trace Port Databus)
	*  PSU_IOU_SLCR_MIO_PIN_16_L3_SEL                              0

	* Configures MIO Pin 16 peripheral interface mapping
	* (OFFSET, MASK, VALUE)      (0XFF180040, 0x000000FEU ,0x00000000U)
	*/
	PSU_Mask_Write( IOU_SLCR_MIO_PIN_16_OFFSET, 0x000000FEU, 0x00000000U );
	/*##################################################################### */

	/*
	* Register : MIO_PIN_17 @ 0XFF180044

	* Level 0 Mux Select 0= Level 1 Mux Output 1= Not Used
	*  PSU_IOU_SLCR_MIO_PIN_17_L0_SEL                              0

	* Level 1 Mux Select 0= Level 2 Mux Output 1= nand, Input, nfc_dq_in[1]- (
	* NAND Data Bus) 1= nand, Output, nfc_dq_out[1]- (NAND Data Bus)
	*  PSU_IOU_SLCR_MIO_PIN_17_L1_SEL                              0

	* Level 2 Mux Select 0= Level 3 Mux Output 1= sd0, Input, sd0_data_in[4]-
	* (8-bit Data bus) = sd0, Output, sdio0_data_out[4]- (8-bit Data bus) 2= t
	* est_scan, Input, test_scan_in[17]- (Test Scan Port) = test_scan, Output,
	*  test_scan_out[17]- (Test Scan Port) 3= Not Used
	*  PSU_IOU_SLCR_MIO_PIN_17_L2_SEL                              0

	* Level 3 Mux Select 0= gpio0, Input, gpio_0_pin_in[17]- (GPIO bank 0) 0=
	* gpio0, Output, gpio_0_pin_out[17]- (GPIO bank 0) 1= can1, Input, can1_ph
	* y_rx- (Can RX signal) 2= i2c1, Input, i2c1_sda_input- (SDA signal) 2= i2
	* c1, Output, i2c1_sda_out- (SDA signal) 3= swdt1, Output, swdt1_rst_out-
	* (Watch Dog Timer Output clock) 4= spi0, Output, spi0_mo- (MOSI signal) 4
	* = spi0, Input, spi0_si- (MOSI signal) 5= ttc3, Output, ttc3_wave_out- (T
	* TC Waveform Clock) 6= ua1, Input, ua1_rxd- (UART receiver serial input)
	* 7= trace, Output, tracedq[15]- (Trace Port Databus)
	*  PSU_IOU_SLCR_MIO_PIN_17_L3_SEL                              0

	* Configures MIO Pin 17 peripheral interface mapping
	* (OFFSET, MASK, VALUE)      (0XFF180044, 0x000000FEU ,0x00000000U)
	*/
	PSU_Mask_Write( IOU_SLCR_MIO_PIN_17_OFFSET, 0x000000FEU, 0x00000000U );
	/*##################################################################### */

	/*
	* Register : MIO_PIN_18 @ 0XFF180048

	* Level 0 Mux Select 0= Level 1 Mux Output 1= Not Used
	*  PSU_IOU_SLCR_MIO_PIN_18_L0_SEL                              0

	* Level 1 Mux Select 0= Level 2 Mux Output 1= nand, Input, nfc_dq_in[2]- (
	* NAND Data Bus) 1= nand, Output, nfc_dq_out[2]- (NAND Data Bus)
	*  PSU_IOU_SLCR_MIO_PIN_18_L1_SEL                              0

	* Level 2 Mux Select 0= Level 3 Mux Output 1= sd0, Input, sd0_data_in[5]-
	* (8-bit Data bus) = sd0, Output, sdio0_data_out[5]- (8-bit Data bus) 2= t
	* est_scan, Input, test_scan_in[18]- (Test Scan Port) = test_scan, Output,
	*  test_scan_out[18]- (Test Scan Port) 3= csu, Input, csu_ext_tamper- (CSU
	*  Ext Tamper)
	*  PSU_IOU_SLCR_MIO_PIN_18_L2_SEL                              0

	* Level 3 Mux Select 0= gpio0, Input, gpio_0_pin_in[18]- (GPIO bank 0) 0=
	* gpio0, Output, gpio_0_pin_out[18]- (GPIO bank 0) 1= can0, Input, can0_ph
	* y_rx- (Can RX signal) 2= i2c0, Input, i2c0_scl_input- (SCL signal) 2= i2
	* c0, Output, i2c0_scl_out- (SCL signal) 3= swdt0, Input, swdt0_clk_in- (W
	* atch Dog Timer Input clock) 4= spi1, Input, spi1_mi- (MISO signal) 4= sp
	* i1, Output, spi1_so- (MISO signal) 5= ttc2, Input, ttc2_clk_in- (TTC Clo
	* ck) 6= ua0, Input, ua0_rxd- (UART receiver serial input) 7= Not Used
	*  PSU_IOU_SLCR_MIO_PIN_18_L3_SEL                              0

	* Configures MIO Pin 18 peripheral interface mapping
	* (OFFSET, MASK, VALUE)      (0XFF180048, 0x000000FEU ,0x00000000U)
	*/
	PSU_Mask_Write( IOU_SLCR_MIO_PIN_18_OFFSET, 0x000000FEU, 0x00000000U );
	/*##################################################################### */

	/*
	* Register : MIO_PIN_19 @ 0XFF18004C

	* Level 0 Mux Select 0= Level 1 Mux Output 1= Not Used
	*  PSU_IOU_SLCR_MIO_PIN_19_L0_SEL                              0

	* Level 1 Mux Select 0= Level 2 Mux Output 1= nand, Input, nfc_dq_in[3]- (
	* NAND Data Bus) 1= nand, Output, nfc_dq_out[3]- (NAND Data Bus)
	*  PSU_IOU_SLCR_MIO_PIN_19_L1_SEL                              0

	* Level 2 Mux Select 0= Level 3 Mux Output 1= sd0, Input, sd0_data_in[6]-
	* (8-bit Data bus) = sd0, Output, sdio0_data_out[6]- (8-bit Data bus) 2= t
	* est_scan, Input, test_scan_in[19]- (Test Scan Port) = test_scan, Output,
	*  test_scan_out[19]- (Test Scan Port) 3= csu, Input, csu_ext_tamper- (CSU
	*  Ext Tamper)
	*  PSU_IOU_SLCR_MIO_PIN_19_L2_SEL                              0

	* Level 3 Mux Select 0= gpio0, Input, gpio_0_pin_in[19]- (GPIO bank 0) 0=
	* gpio0, Output, gpio_0_pin_out[19]- (GPIO bank 0) 1= can0, Output, can0_p
	* hy_tx- (Can TX signal) 2= i2c0, Input, i2c0_sda_input- (SDA signal) 2= i
	* 2c0, Output, i2c0_sda_out- (SDA signal) 3= swdt0, Output, swdt0_rst_out-
	*  (Watch Dog Timer Output clock) 4= spi1, Output, spi1_n_ss_out[2]- (SPI
	* Master Selects) 5= ttc2, Output, ttc2_wave_out- (TTC Waveform Clock) 6=
	* ua0, Output, ua0_txd- (UART transmitter serial output) 7= Not Used
	*  PSU_IOU_SLCR_MIO_PIN_19_L3_SEL                              0

	* Configures MIO Pin 19 peripheral interface mapping
	* (OFFSET, MASK, VALUE)      (0XFF18004C, 0x000000FEU ,0x00000000U)
	*/
	PSU_Mask_Write( IOU_SLCR_MIO_PIN_19_OFFSET, 0x000000FEU, 0x00000000U );
	/*##################################################################### */

	/*
	* Register : MIO_PIN_20 @ 0XFF180050

	* Level 0 Mux Select 0= Level 1 Mux Output 1= Not Used
	*  PSU_IOU_SLCR_MIO_PIN_20_L0_SEL                              0

	* Level 1 Mux Select 0= Level 2 Mux Output 1= nand, Input, nfc_dq_in[4]- (
	* NAND Data Bus) 1= nand, Output, nfc_dq_out[4]- (NAND Data Bus)
	*  PSU_IOU_SLCR_MIO_PIN_20_L1_SEL                              0

	* Level 2 Mux Select 0= Level 3 Mux Output 1= sd0, Input, sd0_data_in[7]-
	* (8-bit Data bus) = sd0, Output, sdio0_data_out[7]- (8-bit Data bus) 2= t
	* est_scan, Input, test_scan_in[20]- (Test Scan Port) = test_scan, Output,
	*  test_scan_out[20]- (Test Scan Port) 3= csu, Input, csu_ext_tamper- (CSU
	*  Ext Tamper)
	*  PSU_IOU_SLCR_MIO_PIN_20_L2_SEL                              0

	* Level 3 Mux Select 0= gpio0, Input, gpio_0_pin_in[20]- (GPIO bank 0) 0=
	* gpio0, Output, gpio_0_pin_out[20]- (GPIO bank 0) 1= can1, Output, can1_p
	* hy_tx- (Can TX signal) 2= i2c1, Input, i2c1_scl_input- (SCL signal) 2= i
	* 2c1, Output, i2c1_scl_out- (SCL signal) 3= swdt1, Input, swdt1_clk_in- (
	* Watch Dog Timer Input clock) 4= spi1, Output, spi1_n_ss_out[1]- (SPI Mas
	* ter Selects) 5= ttc1, Input, ttc1_clk_in- (TTC Clock) 6= ua1, Output, ua
	* 1_txd- (UART transmitter serial output) 7= Not Used
	*  PSU_IOU_SLCR_MIO_PIN_20_L3_SEL                              0

	* Configures MIO Pin 20 peripheral interface mapping
	* (OFFSET, MASK, VALUE)      (0XFF180050, 0x000000FEU ,0x00000000U)
	*/
	PSU_Mask_Write( IOU_SLCR_MIO_PIN_20_OFFSET, 0x000000FEU, 0x00000000U );
	/*##################################################################### */

	/*
	* Register : MIO_PIN_21 @ 0XFF180054

	* Level 0 Mux Select 0= Level 1 Mux Output 1= Not Used
	*  PSU_IOU_SLCR_MIO_PIN_21_L0_SEL                              0

	* Level 1 Mux Select 0= Level 2 Mux Output 1= nand, Input, nfc_dq_in[5]- (
	* NAND Data Bus) 1= nand, Output, nfc_dq_out[5]- (NAND Data Bus)
	*  PSU_IOU_SLCR_MIO_PIN_21_L1_SEL                              0

	* Level 2 Mux Select 0= Level 3 Mux Output 1= sd0, Input, sd0_cmd_in- (Com
	* mand Indicator) = sd0, Output, sdio0_cmd_out- (Command Indicator) 2= tes
	* t_scan, Input, test_scan_in[21]- (Test Scan Port) = test_scan, Output, t
	* est_scan_out[21]- (Test Scan Port) 3= csu, Input, csu_ext_tamper- (CSU E
	* xt Tamper)
	*  PSU_IOU_SLCR_MIO_PIN_21_L2_SEL                              0

	* Level 3 Mux Select 0= gpio0, Input, gpio_0_pin_in[21]- (GPIO bank 0) 0=
	* gpio0, Output, gpio_0_pin_out[21]- (GPIO bank 0) 1= can1, Input, can1_ph
	* y_rx- (Can RX signal) 2= i2c1, Input, i2c1_sda_input- (SDA signal) 2= i2
	* c1, Output, i2c1_sda_out- (SDA signal) 3= swdt1, Output, swdt1_rst_out-
	* (Watch Dog Timer Output clock) 4= spi1, Input, spi1_n_ss_in- (SPI Master
	*  Selects) 4= spi1, Output, spi1_n_ss_out[0]- (SPI Master Selects) 5= ttc
	* 1, Output, ttc1_wave_out- (TTC Waveform Clock) 6= ua1, Input, ua1_rxd- (
	* UART receiver serial input) 7= Not Used
	*  PSU_IOU_SLCR_MIO_PIN_21_L3_SEL                              0

	* Configures MIO Pin 21 peripheral interface mapping
	* (OFFSET, MASK, VALUE)      (0XFF180054, 0x000000FEU ,0x00000000U)
	*/
	PSU_Mask_Write( IOU_SLCR_MIO_PIN_21_OFFSET, 0x000000FEU, 0x00000000U );
	/*##################################################################### */

	/*
	* Register : MIO_PIN_22 @ 0XFF180058

	* Level 0 Mux Select 0= Level 1 Mux Output 1= Not Used
	*  PSU_IOU_SLCR_MIO_PIN_22_L0_SEL                              0

	* Level 1 Mux Select 0= Level 2 Mux Output 1= nand, Output, nfc_we_b- (NAN
	* D Write Enable)
	*  PSU_IOU_SLCR_MIO_PIN_22_L1_SEL                              0

	* Level 2 Mux Select 0= Level 3 Mux Output 1= sd0, Output, sdio0_clk_out-
	* (SDSDIO clock) 2= test_scan, Input, test_scan_in[22]- (Test Scan Port) =
	*  test_scan, Output, test_scan_out[22]- (Test Scan Port) 3= csu, Input, c
	* su_ext_tamper- (CSU Ext Tamper)
	*  PSU_IOU_SLCR_MIO_PIN_22_L2_SEL                              0

	* Level 3 Mux Select 0= gpio0, Input, gpio_0_pin_in[22]- (GPIO bank 0) 0=
	* gpio0, Output, gpio_0_pin_out[22]- (GPIO bank 0) 1= can0, Input, can0_ph
	* y_rx- (Can RX signal) 2= i2c0, Input, i2c0_scl_input- (SCL signal) 2= i2
	* c0, Output, i2c0_scl_out- (SCL signal) 3= swdt0, Input, swdt0_clk_in- (W
	* atch Dog Timer Input clock) 4= spi1, Input, spi1_sclk_in- (SPI Clock) 4=
	*  spi1, Output, spi1_sclk_out- (SPI Clock) 5= ttc0, Input, ttc0_clk_in- (
	* TTC Clock) 6= ua0, Input, ua0_rxd- (UART receiver serial input) 7= Not U
	* sed
	*  PSU_IOU_SLCR_MIO_PIN_22_L3_SEL                              0

	* Configures MIO Pin 22 peripheral interface mapping
	* (OFFSET, MASK, VALUE)      (0XFF180058, 0x000000FEU ,0x00000000U)
	*/
	PSU_Mask_Write( IOU_SLCR_MIO_PIN_22_OFFSET, 0x000000FEU, 0x00000000U );
	/*##################################################################### */

	/*
	* Register : MIO_PIN_23 @ 0XFF18005C

	* Level 0 Mux Select 0= Level 1 Mux Output 1= Not Used
	*  PSU_IOU_SLCR_MIO_PIN_23_L0_SEL                              0

	* Level 1 Mux Select 0= Level 2 Mux Output 1= nand, Input, nfc_dq_in[6]- (
	* NAND Data Bus) 1= nand, Output, nfc_dq_out[6]- (NAND Data Bus)
	*  PSU_IOU_SLCR_MIO_PIN_23_L1_SEL                              0

	* Level 2 Mux Select 0= Level 3 Mux Output 1= sd0, Output, sdio0_bus_pow-
	* (SD card bus power) 2= test_scan, Input, test_scan_in[23]- (Test Scan Po
	* rt) = test_scan, Output, test_scan_out[23]- (Test Scan Port) 3= csu, Inp
	* ut, csu_ext_tamper- (CSU Ext Tamper)
	*  PSU_IOU_SLCR_MIO_PIN_23_L2_SEL                              0

	* Level 3 Mux Select 0= gpio0, Input, gpio_0_pin_in[23]- (GPIO bank 0) 0=
	* gpio0, Output, gpio_0_pin_out[23]- (GPIO bank 0) 1= can0, Output, can0_p
	* hy_tx- (Can TX signal) 2= i2c0, Input, i2c0_sda_input- (SDA signal) 2= i
	* 2c0, Output, i2c0_sda_out- (SDA signal) 3= swdt0, Output, swdt0_rst_out-
	*  (Watch Dog Timer Output clock) 4= spi1, Output, spi1_mo- (MOSI signal)
	* 4= spi1, Input, spi1_si- (MOSI signal) 5= ttc0, Output, ttc0_wave_out- (
	* TTC Waveform Clock) 6= ua0, Output, ua0_txd- (UART transmitter serial ou
	* tput) 7= Not Used
	*  PSU_IOU_SLCR_MIO_PIN_23_L3_SEL                              0

	* Configures MIO Pin 23 peripheral interface mapping
	* (OFFSET, MASK, VALUE)      (0XFF18005C, 0x000000FEU ,0x00000000U)
	*/
	PSU_Mask_Write( IOU_SLCR_MIO_PIN_23_OFFSET, 0x000000FEU, 0x00000000U );
	/*##################################################################### */

	/*
	* Register : MIO_PIN_24 @ 0XFF180060

	* Level 0 Mux Select 0= Level 1 Mux Output 1= Not Used
	*  PSU_IOU_SLCR_MIO_PIN_24_L0_SEL                              0

	* Level 1 Mux Select 0= Level 2 Mux Output 1= nand, Input, nfc_dq_in[7]- (
	* NAND Data Bus) 1= nand, Output, nfc_dq_out[7]- (NAND Data Bus)
	*  PSU_IOU_SLCR_MIO_PIN_24_L1_SEL                              0

	* Level 2 Mux Select 0= Level 3 Mux Output 1= sd0, Input, sdio0_cd_n- (SD
	* card detect from connector) 2= test_scan, Input, test_scan_in[24]- (Test
	*  Scan Port) = test_scan, Output, test_scan_out[24]- (Test Scan Port) 3=
	* csu, Input, csu_ext_tamper- (CSU Ext Tamper)
	*  PSU_IOU_SLCR_MIO_PIN_24_L2_SEL                              0

	* Level 3 Mux Select 0= gpio0, Input, gpio_0_pin_in[24]- (GPIO bank 0) 0=
	* gpio0, Output, gpio_0_pin_out[24]- (GPIO bank 0) 1= can1, Output, can1_p
	* hy_tx- (Can TX signal) 2= i2c1, Input, i2c1_scl_input- (SCL signal) 2= i
	* 2c1, Output, i2c1_scl_out- (SCL signal) 3= swdt1, Input, swdt1_clk_in- (
	* Watch Dog Timer Input clock) 4= Not Used 5= ttc3, Input, ttc3_clk_in- (T
	* TC Clock) 6= ua1, Output, ua1_txd- (UART transmitter serial output) 7= N
	* ot Used
	*  PSU_IOU_SLCR_MIO_PIN_24_L3_SEL                              2

	* Configures MIO Pin 24 peripheral interface mapping
	* (OFFSET, MASK, VALUE)      (0XFF180060, 0x000000FEU ,0x00000040U)
	*/
	PSU_Mask_Write( IOU_SLCR_MIO_PIN_24_OFFSET, 0x000000FEU, 0x00000040U );
	/*##################################################################### */

	/*
	* Register : MIO_PIN_25 @ 0XFF180064

	* Level 0 Mux Select 0= Level 1 Mux Output 1= Not Used
	*  PSU_IOU_SLCR_MIO_PIN_25_L0_SEL                              0

	* Level 1 Mux Select 0= Level 2 Mux Output 1= nand, Output, nfc_re_n- (NAN
	* D Read Enable)
	*  PSU_IOU_SLCR_MIO_PIN_25_L1_SEL                              0

	* Level 2 Mux Select 0= Level 3 Mux Output 1= sd0, Input, sdio0_wp- (SD ca
	* rd write protect from connector) 2= test_scan, Input, test_scan_in[25]-
	* (Test Scan Port) = test_scan, Output, test_scan_out[25]- (Test Scan Port
	* ) 3= csu, Input, csu_ext_tamper- (CSU Ext Tamper)
	*  PSU_IOU_SLCR_MIO_PIN_25_L2_SEL                              0

	* Level 3 Mux Select 0= gpio0, Input, gpio_0_pin_in[25]- (GPIO bank 0) 0=
	* gpio0, Output, gpio_0_pin_out[25]- (GPIO bank 0) 1= can1, Input, can1_ph
	* y_rx- (Can RX signal) 2= i2c1, Input, i2c1_sda_input- (SDA signal) 2= i2
	* c1, Output, i2c1_sda_out- (SDA signal) 3= swdt1, Output, swdt1_rst_out-
	* (Watch Dog Timer Output clock) 4= Not Used 5= ttc3, Output, ttc3_wave_ou
	* t- (TTC Waveform Clock) 6= ua1, Input, ua1_rxd- (UART receiver serial in
	* put) 7= Not Used
	*  PSU_IOU_SLCR_MIO_PIN_25_L3_SEL                              2

	* Configures MIO Pin 25 peripheral interface mapping
	* (OFFSET, MASK, VALUE)      (0XFF180064, 0x000000FEU ,0x00000040U)
	*/
	PSU_Mask_Write( IOU_SLCR_MIO_PIN_25_OFFSET, 0x000000FEU, 0x00000040U );
	/*##################################################################### */

	/*
	* Register : MIO_PIN_26 @ 0XFF180068

	* Level 0 Mux Select 0= Level 1 Mux Output 1= gem0, Output, gem0_rgmii_tx_
	* clk- (TX RGMII clock)
	*  PSU_IOU_SLCR_MIO_PIN_26_L0_SEL                              0

	* Level 1 Mux Select 0= Level 2 Mux Output 1= nand, Output, nfc_ce[1]- (NA
	* ND chip enable)
	*  PSU_IOU_SLCR_MIO_PIN_26_L1_SEL                              0

	* Level 2 Mux Select 0= Level 3 Mux Output 1= pmu, Input, pmu_gpi[0]- (PMU
	*  GPI) 2= test_scan, Input, test_scan_in[26]- (Test Scan Port) = test_sca
	* n, Output, test_scan_out[26]- (Test Scan Port) 3= csu, Input, csu_ext_ta
	* mper- (CSU Ext Tamper)
	*  PSU_IOU_SLCR_MIO_PIN_26_L2_SEL                              1

	* Level 3 Mux Select 0= gpio1, Input, gpio_1_pin_in[0]- (GPIO bank 1) 0= g
	* pio1, Output, gpio_1_pin_out[0]- (GPIO bank 1) 1= can0, Input, can0_phy_
	* rx- (Can RX signal) 2= i2c0, Input, i2c0_scl_input- (SCL signal) 2= i2c0
	* , Output, i2c0_scl_out- (SCL signal) 3= pjtag, Input, pjtag_tck- (PJTAG
	* TCK) 4= spi0, Input, spi0_sclk_in- (SPI Clock) 4= spi0, Output, spi0_scl
	* k_out- (SPI Clock) 5= ttc2, Input, ttc2_clk_in- (TTC Clock) 6= ua0, Inpu
	* t, ua0_rxd- (UART receiver serial input) 7= trace, Output, tracedq[4]- (
	* Trace Port Databus)
	*  PSU_IOU_SLCR_MIO_PIN_26_L3_SEL                              0

	* Configures MIO Pin 26 peripheral interface mapping
	* (OFFSET, MASK, VALUE)      (0XFF180068, 0x000000FEU ,0x00000008U)
	*/
	PSU_Mask_Write( IOU_SLCR_MIO_PIN_26_OFFSET, 0x000000FEU, 0x00000008U );
	/*##################################################################### */

	/*
	* Register : MIO_PIN_27 @ 0XFF18006C

	* Level 0 Mux Select 0= Level 1 Mux Output 1= gem0, Output, gem0_rgmii_txd
	* [0]- (TX RGMII data)
	*  PSU_IOU_SLCR_MIO_PIN_27_L0_SEL                              0

	* Level 1 Mux Select 0= Level 2 Mux Output 1= nand, Input, nfc_rb_n[0]- (N
	* AND Ready/Busy)
	*  PSU_IOU_SLCR_MIO_PIN_27_L1_SEL                              0

	* Level 2 Mux Select 0= Level 3 Mux Output 1= pmu, Input, pmu_gpi[1]- (PMU
	*  GPI) 2= test_scan, Input, test_scan_in[27]- (Test Scan Port) = test_sca
	* n, Output, test_scan_out[27]- (Test Scan Port) 3= dpaux, Input, dp_aux_d
	* ata_in- (Dp Aux Data) = dpaux, Output, dp_aux_data_out- (Dp Aux Data)
	*  PSU_IOU_SLCR_MIO_PIN_27_L2_SEL                              3

	* Level 3 Mux Select 0= gpio1, Input, gpio_1_pin_in[1]- (GPIO bank 1) 0= g
	* pio1, Output, gpio_1_pin_out[1]- (GPIO bank 1) 1= can0, Output, can0_phy
	* _tx- (Can TX signal) 2= i2c0, Input, i2c0_sda_input- (SDA signal) 2= i2c
	* 0, Output, i2c0_sda_out- (SDA signal) 3= pjtag, Input, pjtag_tdi- (PJTAG
	*  TDI) 4= spi0, Output, spi0_n_ss_out[2]- (SPI Master Selects) 5= ttc2, O
	* utput, ttc2_wave_out- (TTC Waveform Clock) 6= ua0, Output, ua0_txd- (UAR
	* T transmitter serial output) 7= trace, Output, tracedq[5]- (Trace Port D
	* atabus)
	*  PSU_IOU_SLCR_MIO_PIN_27_L3_SEL                              0

	* Configures MIO Pin 27 peripheral interface mapping
	* (OFFSET, MASK, VALUE)      (0XFF18006C, 0x000000FEU ,0x00000018U)
	*/
	PSU_Mask_Write( IOU_SLCR_MIO_PIN_27_OFFSET, 0x000000FEU, 0x00000018U );
	/*##################################################################### */

	/*
	* Register : MIO_PIN_28 @ 0XFF180070

	* Level 0 Mux Select 0= Level 1 Mux Output 1= gem0, Output, gem0_rgmii_txd
	* [1]- (TX RGMII data)
	*  PSU_IOU_SLCR_MIO_PIN_28_L0_SEL                              0

	* Level 1 Mux Select 0= Level 2 Mux Output 1= nand, Input, nfc_rb_n[1]- (N
	* AND Ready/Busy)
	*  PSU_IOU_SLCR_MIO_PIN_28_L1_SEL                              0

	* Level 2 Mux Select 0= Level 3 Mux Output 1= pmu, Input, pmu_gpi[2]- (PMU
	*  GPI) 2= test_scan, Input, test_scan_in[28]- (Test Scan Port) = test_sca
	* n, Output, test_scan_out[28]- (Test Scan Port) 3= dpaux, Input, dp_hot_p
	* lug_detect- (Dp Aux Hot Plug)
	*  PSU_IOU_SLCR_MIO_PIN_28_L2_SEL                              3

	* Level 3 Mux Select 0= gpio1, Input, gpio_1_pin_in[2]- (GPIO bank 1) 0= g
	* pio1, Output, gpio_1_pin_out[2]- (GPIO bank 1) 1= can1, Output, can1_phy
	* _tx- (Can TX signal) 2= i2c1, Input, i2c1_scl_input- (SCL signal) 2= i2c
	* 1, Output, i2c1_scl_out- (SCL signal) 3= pjtag, Output, pjtag_tdo- (PJTA
	* G TDO) 4= spi0, Output, spi0_n_ss_out[1]- (SPI Master Selects) 5= ttc1,
	* Input, ttc1_clk_in- (TTC Clock) 6= ua1, Output, ua1_txd- (UART transmitt
	* er serial output) 7= trace, Output, tracedq[6]- (Trace Port Databus)
	*  PSU_IOU_SLCR_MIO_PIN_28_L3_SEL                              0

	* Configures MIO Pin 28 peripheral interface mapping
	* (OFFSET, MASK, VALUE)      (0XFF180070, 0x000000FEU ,0x00000018U)
	*/
	PSU_Mask_Write( IOU_SLCR_MIO_PIN_28_OFFSET, 0x000000FEU, 0x00000018U );
	/*##################################################################### */

	/*
	* Register : MIO_PIN_29 @ 0XFF180074

	* Level 0 Mux Select 0= Level 1 Mux Output 1= gem0, Output, gem0_rgmii_txd
	* [2]- (TX RGMII data)
	*  PSU_IOU_SLCR_MIO_PIN_29_L0_SEL                              0

	* Level 1 Mux Select 0= Level 2 Mux Output 1= pcie, Input, pcie_reset_n- (
	* PCIE Reset signal)
	*  PSU_IOU_SLCR_MIO_PIN_29_L1_SEL                              0

	* Level 2 Mux Select 0= Level 3 Mux Output 1= pmu, Input, pmu_gpi[3]- (PMU
	*  GPI) 2= test_scan, Input, test_scan_in[29]- (Test Scan Port) = test_sca
	* n, Output, test_scan_out[29]- (Test Scan Port) 3= dpaux, Input, dp_aux_d
	* ata_in- (Dp Aux Data) = dpaux, Output, dp_aux_data_out- (Dp Aux Data)
	*  PSU_IOU_SLCR_MIO_PIN_29_L2_SEL                              3

	* Level 3 Mux Select 0= gpio1, Input, gpio_1_pin_in[3]- (GPIO bank 1) 0= g
	* pio1, Output, gpio_1_pin_out[3]- (GPIO bank 1) 1= can1, Input, can1_phy_
	* rx- (Can RX signal) 2= i2c1, Input, i2c1_sda_input- (SDA signal) 2= i2c1
	* , Output, i2c1_sda_out- (SDA signal) 3= pjtag, Input, pjtag_tms- (PJTAG
	* TMS) 4= spi0, Input, spi0_n_ss_in- (SPI Master Selects) 4= spi0, Output,
	*  spi0_n_ss_out[0]- (SPI Master Selects) 5= ttc1, Output, ttc1_wave_out-
	* (TTC Waveform Clock) 6= ua1, Input, ua1_rxd- (UART receiver serial input
	* ) 7= trace, Output, tracedq[7]- (Trace Port Databus)
	*  PSU_IOU_SLCR_MIO_PIN_29_L3_SEL                              0

	* Configures MIO Pin 29 peripheral interface mapping
	* (OFFSET, MASK, VALUE)      (0XFF180074, 0x000000FEU ,0x00000018U)
	*/
	PSU_Mask_Write( IOU_SLCR_MIO_PIN_29_OFFSET, 0x000000FEU, 0x00000018U );
	/*##################################################################### */

	/*
	* Register : MIO_PIN_30 @ 0XFF180078

	* Level 0 Mux Select 0= Level 1 Mux Output 1= gem0, Output, gem0_rgmii_txd
	* [3]- (TX RGMII data)
	*  PSU_IOU_SLCR_MIO_PIN_30_L0_SEL                              0

	* Level 1 Mux Select 0= Level 2 Mux Output 1= pcie, Input, pcie_reset_n- (
	* PCIE Reset signal)
	*  PSU_IOU_SLCR_MIO_PIN_30_L1_SEL                              0

	* Level 2 Mux Select 0= Level 3 Mux Output 1= pmu, Input, pmu_gpi[4]- (PMU
	*  GPI) 2= test_scan, Input, test_scan_in[30]- (Test Scan Port) = test_sca
	* n, Output, test_scan_out[30]- (Test Scan Port) 3= dpaux, Input, dp_hot_p
	* lug_detect- (Dp Aux Hot Plug)
	*  PSU_IOU_SLCR_MIO_PIN_30_L2_SEL                              3

	* Level 3 Mux Select 0= gpio1, Input, gpio_1_pin_in[4]- (GPIO bank 1) 0= g
	* pio1, Output, gpio_1_pin_out[4]- (GPIO bank 1) 1= can0, Input, can0_phy_
	* rx- (Can RX signal) 2= i2c0, Input, i2c0_scl_input- (SCL signal) 2= i2c0
	* , Output, i2c0_scl_out- (SCL signal) 3= swdt0, Input, swdt0_clk_in- (Wat
	* ch Dog Timer Input clock) 4= spi0, Input, spi0_mi- (MISO signal) 4= spi0
	* , Output, spi0_so- (MISO signal) 5= ttc0, Input, ttc0_clk_in- (TTC Clock
	* ) 6= ua0, Input, ua0_rxd- (UART receiver serial input) 7= trace, Output,
	*  tracedq[8]- (Trace Port Databus)
	*  PSU_IOU_SLCR_MIO_PIN_30_L3_SEL                              0

	* Configures MIO Pin 30 peripheral interface mapping
	* (OFFSET, MASK, VALUE)      (0XFF180078, 0x000000FEU ,0x00000018U)
	*/
	PSU_Mask_Write( IOU_SLCR_MIO_PIN_30_OFFSET, 0x000000FEU, 0x00000018U );
	/*##################################################################### */

	/*
	* Register : MIO_PIN_31 @ 0XFF18007C

	* Level 0 Mux Select 0= Level 1 Mux Output 1= gem0, Output, gem0_rgmii_tx_
	* ctl- (TX RGMII control)
	*  PSU_IOU_SLCR_MIO_PIN_31_L0_SEL                              0

	* Level 1 Mux Select 0= Level 2 Mux Output 1= pcie, Input, pcie_reset_n- (
	* PCIE Reset signal)
	*  PSU_IOU_SLCR_MIO_PIN_31_L1_SEL                              0

	* Level 2 Mux Select 0= Level 3 Mux Output 1= pmu, Input, pmu_gpi[5]- (PMU
	*  GPI) 2= test_scan, Input, test_scan_in[31]- (Test Scan Port) = test_sca
	* n, Output, test_scan_out[31]- (Test Scan Port) 3= csu, Input, csu_ext_ta
	* mper- (CSU Ext Tamper)
	*  PSU_IOU_SLCR_MIO_PIN_31_L2_SEL                              1

	* Level 3 Mux Select 0= gpio1, Input, gpio_1_pin_in[5]- (GPIO bank 1) 0= g
	* pio1, Output, gpio_1_pin_out[5]- (GPIO bank 1) 1= can0, Output, can0_phy
	* _tx- (Can TX signal) 2= i2c0, Input, i2c0_sda_input- (SDA signal) 2= i2c
	* 0, Output, i2c0_sda_out- (SDA signal) 3= swdt0, Output, swdt0_rst_out- (
	* Watch Dog Timer Output clock) 4= spi0, Output, spi0_mo- (MOSI signal) 4=
	*  spi0, Input, spi0_si- (MOSI signal) 5= ttc0, Output, ttc0_wave_out- (TT
	* C Waveform Clock) 6= ua0, Output, ua0_txd- (UART transmitter serial outp
	* ut) 7= trace, Output, tracedq[9]- (Trace Port Databus)
	*  PSU_IOU_SLCR_MIO_PIN_31_L3_SEL                              0

	* Configures MIO Pin 31 peripheral interface mapping
	* (OFFSET, MASK, VALUE)      (0XFF18007C, 0x000000FEU ,0x00000008U)
	*/
	PSU_Mask_Write( IOU_SLCR_MIO_PIN_31_OFFSET, 0x000000FEU, 0x00000008U );
	/*##################################################################### */

	/*
	* Register : MIO_PIN_32 @ 0XFF180080

	* Level 0 Mux Select 0= Level 1 Mux Output 1= gem0, Input, gem0_rgmii_rx_c
	* lk- (RX RGMII clock)
	*  PSU_IOU_SLCR_MIO_PIN_32_L0_SEL                              0

	* Level 1 Mux Select 0= Level 2 Mux Output 1= nand, Input, nfc_dqs_in- (NA
	* ND Strobe) 1= nand, Output, nfc_dqs_out- (NAND Strobe)
	*  PSU_IOU_SLCR_MIO_PIN_32_L1_SEL                              0

	* Level 2 Mux Select 0= Level 3 Mux Output 1= pmu, Output, pmu_gpo[0]- (PM
	* U GPI) 2= test_scan, Input, test_scan_in[32]- (Test Scan Port) = test_sc
	* an, Output, test_scan_out[32]- (Test Scan Port) 3= csu, Input, csu_ext_t
	* amper- (CSU Ext Tamper)
	*  PSU_IOU_SLCR_MIO_PIN_32_L2_SEL                              1

	* Level 3 Mux Select 0= gpio1, Input, gpio_1_pin_in[6]- (GPIO bank 1) 0= g
	* pio1, Output, gpio_1_pin_out[6]- (GPIO bank 1) 1= can1, Output, can1_phy
	* _tx- (Can TX signal) 2= i2c1, Input, i2c1_scl_input- (SCL signal) 2= i2c
	* 1, Output, i2c1_scl_out- (SCL signal) 3= swdt1, Input, swdt1_clk_in- (Wa
	* tch Dog Timer Input clock) 4= spi1, Input, spi1_sclk_in- (SPI Clock) 4=
	* spi1, Output, spi1_sclk_out- (SPI Clock) 5= ttc3, Input, ttc3_clk_in- (T
	* TC Clock) 6= ua1, Output, ua1_txd- (UART transmitter serial output) 7= t
	* race, Output, tracedq[10]- (Trace Port Databus)
	*  PSU_IOU_SLCR_MIO_PIN_32_L3_SEL                              0

	* Configures MIO Pin 32 peripheral interface mapping
	* (OFFSET, MASK, VALUE)      (0XFF180080, 0x000000FEU ,0x00000008U)
	*/
	PSU_Mask_Write( IOU_SLCR_MIO_PIN_32_OFFSET, 0x000000FEU, 0x00000008U );
	/*##################################################################### */

	/*
	* Register : MIO_PIN_33 @ 0XFF180084

	* Level 0 Mux Select 0= Level 1 Mux Output 1= gem0, Input, gem0_rgmii_rxd[
	* 0]- (RX RGMII data)
	*  PSU_IOU_SLCR_MIO_PIN_33_L0_SEL                              0

	* Level 1 Mux Select 0= Level 2 Mux Output 1= pcie, Input, pcie_reset_n- (
	* PCIE Reset signal)
	*  PSU_IOU_SLCR_MIO_PIN_33_L1_SEL                              0

	* Level 2 Mux Select 0= Level 3 Mux Output 1= pmu, Output, pmu_gpo[1]- (PM
	* U GPI) 2= test_scan, Input, test_scan_in[33]- (Test Scan Port) = test_sc
	* an, Output, test_scan_out[33]- (Test Scan Port) 3= csu, Input, csu_ext_t
	* amper- (CSU Ext Tamper)
	*  PSU_IOU_SLCR_MIO_PIN_33_L2_SEL                              1

	* Level 3 Mux Select 0= gpio1, Input, gpio_1_pin_in[7]- (GPIO bank 1) 0= g
	* pio1, Output, gpio_1_pin_out[7]- (GPIO bank 1) 1= can1, Input, can1_phy_
	* rx- (Can RX signal) 2= i2c1, Input, i2c1_sda_input- (SDA signal) 2= i2c1
	* , Output, i2c1_sda_out- (SDA signal) 3= swdt1, Output, swdt1_rst_out- (W
	* atch Dog Timer Output clock) 4= spi1, Output, spi1_n_ss_out[2]- (SPI Mas
	* ter Selects) 5= ttc3, Output, ttc3_wave_out- (TTC Waveform Clock) 6= ua1
	* , Input, ua1_rxd- (UART receiver serial input) 7= trace, Output, tracedq
	* [11]- (Trace Port Databus)
	*  PSU_IOU_SLCR_MIO_PIN_33_L3_SEL                              0

	* Configures MIO Pin 33 peripheral interface mapping
	* (OFFSET, MASK, VALUE)      (0XFF180084, 0x000000FEU ,0x00000008U)
	*/
	PSU_Mask_Write( IOU_SLCR_MIO_PIN_33_OFFSET, 0x000000FEU, 0x00000008U );
	/*##################################################################### */

	/*
	* Register : MIO_PIN_36 @ 0XFF180090

	* Level 0 Mux Select 0= Level 1 Mux Output 1= gem0, Input, gem0_rgmii_rxd[
	* 3]- (RX RGMII data)
	*  PSU_IOU_SLCR_MIO_PIN_36_L0_SEL                              0

	* Level 1 Mux Select 0= Level 2 Mux Output 1= pcie, Input, pcie_reset_n- (
	* PCIE Reset signal)
	*  PSU_IOU_SLCR_MIO_PIN_36_L1_SEL                              0

	* Level 2 Mux Select 0= Level 3 Mux Output 1= pmu, Output, pmu_gpo[4]- (PM
	* U GPI) 2= test_scan, Input, test_scan_in[36]- (Test Scan Port) = test_sc
	* an, Output, test_scan_out[36]- (Test Scan Port) 3= dpaux, Input, dp_aux_
	* data_in- (Dp Aux Data) = dpaux, Output, dp_aux_data_out- (Dp Aux Data)
	*  PSU_IOU_SLCR_MIO_PIN_36_L2_SEL                              0

	* Level 3 Mux Select 0= gpio1, Input, gpio_1_pin_in[10]- (GPIO bank 1) 0=
	* gpio1, Output, gpio_1_pin_out[10]- (GPIO bank 1) 1= can1, Output, can1_p
	* hy_tx- (Can TX signal) 2= i2c1, Input, i2c1_scl_input- (SCL signal) 2= i
	* 2c1, Output, i2c1_scl_out- (SCL signal) 3= swdt1, Input, swdt1_clk_in- (
	* Watch Dog Timer Input clock) 4= spi1, Input, spi1_mi- (MISO signal) 4= s
	* pi1, Output, spi1_so- (MISO signal) 5= ttc1, Input, ttc1_clk_in- (TTC Cl
	* ock) 6= ua1, Output, ua1_txd- (UART transmitter serial output) 7= trace,
	*  Output, tracedq[14]- (Trace Port Databus)
	*  PSU_IOU_SLCR_MIO_PIN_36_L3_SEL                              6

	* Configures MIO Pin 36 peripheral interface mapping
	* (OFFSET, MASK, VALUE)      (0XFF180090, 0x000000FEU ,0x000000C0U)
	*/
	PSU_Mask_Write( IOU_SLCR_MIO_PIN_36_OFFSET, 0x000000FEU, 0x000000C0U );
	/*##################################################################### */

	/*
	* Register : MIO_PIN_37 @ 0XFF180094

	* Level 0 Mux Select 0= Level 1 Mux Output 1= gem0, Input, gem0_rgmii_rx_c
	* tl- (RX RGMII control )
	*  PSU_IOU_SLCR_MIO_PIN_37_L0_SEL                              0

	* Level 1 Mux Select 0= Level 2 Mux Output 1= pcie, Input, pcie_reset_n- (
	* PCIE Reset signal)
	*  PSU_IOU_SLCR_MIO_PIN_37_L1_SEL                              0

	* Level 2 Mux Select 0= Level 3 Mux Output 1= pmu, Output, pmu_gpo[5]- (PM
	* U GPI) 2= test_scan, Input, test_scan_in[37]- (Test Scan Port) = test_sc
	* an, Output, test_scan_out[37]- (Test Scan Port) 3= dpaux, Input, dp_hot_
	* plug_detect- (Dp Aux Hot Plug)
	*  PSU_IOU_SLCR_MIO_PIN_37_L2_SEL                              0

	* Level 3 Mux Select 0= gpio1, Input, gpio_1_pin_in[11]- (GPIO bank 1) 0=
	* gpio1, Output, gpio_1_pin_out[11]- (GPIO bank 1) 1= can1, Input, can1_ph
	* y_rx- (Can RX signal) 2= i2c1, Input, i2c1_sda_input- (SDA signal) 2= i2
	* c1, Output, i2c1_sda_out- (SDA signal) 3= swdt1, Output, swdt1_rst_out-
	* (Watch Dog Timer Output clock) 4= spi1, Output, spi1_mo- (MOSI signal) 4
	* = spi1, Input, spi1_si- (MOSI signal) 5= ttc1, Output, ttc1_wave_out- (T
	* TC Waveform Clock) 6= ua1, Input, ua1_rxd- (UART receiver serial input)
	* 7= trace, Output, tracedq[15]- (Trace Port Databus)
	*  PSU_IOU_SLCR_MIO_PIN_37_L3_SEL                              6

	* Configures MIO Pin 37 peripheral interface mapping
	* (OFFSET, MASK, VALUE)      (0XFF180094, 0x000000FEU ,0x000000C0U)
	*/
	PSU_Mask_Write( IOU_SLCR_MIO_PIN_37_OFFSET, 0x000000FEU, 0x000000C0U );
	/*##################################################################### */

	/*
	* Register : MIO_PIN_38 @ 0XFF180098

	* Level 0 Mux Select 0= Level 1 Mux Output 1= gem1, Output, gem1_rgmii_tx_
	* clk- (TX RGMII clock)
	*  PSU_IOU_SLCR_MIO_PIN_38_L0_SEL                              0

	* Level 1 Mux Select 0= Level 2 Mux Output 1= Not Used
	*  PSU_IOU_SLCR_MIO_PIN_38_L1_SEL                              0

	* Level 2 Mux Select 0= Level 3 Mux Output 1= sd0, Output, sdio0_clk_out-
	* (SDSDIO clock) 2= Not Used 3= Not Used
	*  PSU_IOU_SLCR_MIO_PIN_38_L2_SEL                              0

	* Level 3 Mux Select 0= gpio1, Input, gpio_1_pin_in[12]- (GPIO bank 1) 0=
	* gpio1, Output, gpio_1_pin_out[12]- (GPIO bank 1) 1= can0, Input, can0_ph
	* y_rx- (Can RX signal) 2= i2c0, Input, i2c0_scl_input- (SCL signal) 2= i2
	* c0, Output, i2c0_scl_out- (SCL signal) 3= pjtag, Input, pjtag_tck- (PJTA
	* G TCK) 4= spi0, Input, spi0_sclk_in- (SPI Clock) 4= spi0, Output, spi0_s
	* clk_out- (SPI Clock) 5= ttc0, Input, ttc0_clk_in- (TTC Clock) 6= ua0, In
	* put, ua0_rxd- (UART receiver serial input) 7= trace, Output, trace_clk-
	* (Trace Port Clock)
	*  PSU_IOU_SLCR_MIO_PIN_38_L3_SEL                              0

	* Configures MIO Pin 38 peripheral interface mapping
	* (OFFSET, MASK, VALUE)      (0XFF180098, 0x000000FEU ,0x00000000U)
	*/
	PSU_Mask_Write( IOU_SLCR_MIO_PIN_38_OFFSET, 0x000000FEU, 0x00000000U );
	/*##################################################################### */

	/*
	* Register : MIO_PIN_39 @ 0XFF18009C

	* Level 0 Mux Select 0= Level 1 Mux Output 1= gem1, Output, gem1_rgmii_txd
	* [0]- (TX RGMII data)
	*  PSU_IOU_SLCR_MIO_PIN_39_L0_SEL                              0

	* Level 1 Mux Select 0= Level 2 Mux Output 1= Not Used
	*  PSU_IOU_SLCR_MIO_PIN_39_L1_SEL                              0

	* Level 2 Mux Select 0= Level 3 Mux Output 1= sd0, Input, sdio0_cd_n- (SD
	* card detect from connector) 2= sd1, Input, sd1_data_in[4]- (8-bit Data b
	* us) = sd1, Output, sdio1_data_out[4]- (8-bit Data bus) 3= Not Used
	*  PSU_IOU_SLCR_MIO_PIN_39_L2_SEL                              2

	* Level 3 Mux Select 0= gpio1, Input, gpio_1_pin_in[13]- (GPIO bank 1) 0=
	* gpio1, Output, gpio_1_pin_out[13]- (GPIO bank 1) 1= can0, Output, can0_p
	* hy_tx- (Can TX signal) 2= i2c0, Input, i2c0_sda_input- (SDA signal) 2= i
	* 2c0, Output, i2c0_sda_out- (SDA signal) 3= pjtag, Input, pjtag_tdi- (PJT
	* AG TDI) 4= spi0, Output, spi0_n_ss_out[2]- (SPI Master Selects) 5= ttc0,
	*  Output, ttc0_wave_out- (TTC Waveform Clock) 6= ua0, Output, ua0_txd- (U
	* ART transmitter serial output) 7= trace, Output, trace_ctl- (Trace Port
	* Control Signal)
	*  PSU_IOU_SLCR_MIO_PIN_39_L3_SEL                              0

	* Configures MIO Pin 39 peripheral interface mapping
	* (OFFSET, MASK, VALUE)      (0XFF18009C, 0x000000FEU ,0x00000010U)
	*/
	PSU_Mask_Write( IOU_SLCR_MIO_PIN_39_OFFSET, 0x000000FEU, 0x00000010U );
	/*##################################################################### */

	/*
	* Register : MIO_PIN_40 @ 0XFF1800A0

	* Level 0 Mux Select 0= Level 1 Mux Output 1= gem1, Output, gem1_rgmii_txd
	* [1]- (TX RGMII data)
	*  PSU_IOU_SLCR_MIO_PIN_40_L0_SEL                              0

	* Level 1 Mux Select 0= Level 2 Mux Output 1= Not Used
	*  PSU_IOU_SLCR_MIO_PIN_40_L1_SEL                              0

	* Level 2 Mux Select 0= Level 3 Mux Output 1= sd0, Input, sd0_cmd_in- (Com
	* mand Indicator) = sd0, Output, sdio0_cmd_out- (Command Indicator) 2= sd1
	* , Input, sd1_data_in[5]- (8-bit Data bus) = sd1, Output, sdio1_data_out[
	* 5]- (8-bit Data bus) 3= Not Used
	*  PSU_IOU_SLCR_MIO_PIN_40_L2_SEL                              2

	* Level 3 Mux Select 0= gpio1, Input, gpio_1_pin_in[14]- (GPIO bank 1) 0=
	* gpio1, Output, gpio_1_pin_out[14]- (GPIO bank 1) 1= can1, Output, can1_p
	* hy_tx- (Can TX signal) 2= i2c1, Input, i2c1_scl_input- (SCL signal) 2= i
	* 2c1, Output, i2c1_scl_out- (SCL signal) 3= pjtag, Output, pjtag_tdo- (PJ
	* TAG TDO) 4= spi0, Output, spi0_n_ss_out[1]- (SPI Master Selects) 5= ttc3
	* , Input, ttc3_clk_in- (TTC Clock) 6= ua1, Output, ua1_txd- (UART transmi
	* tter serial output) 7= trace, Output, tracedq[0]- (Trace Port Databus)
	*  PSU_IOU_SLCR_MIO_PIN_40_L3_SEL                              0

	* Configures MIO Pin 40 peripheral interface mapping
	* (OFFSET, MASK, VALUE)      (0XFF1800A0, 0x000000FEU ,0x00000010U)
	*/
	PSU_Mask_Write( IOU_SLCR_MIO_PIN_40_OFFSET, 0x000000FEU, 0x00000010U );
	/*##################################################################### */

	/*
	* Register : MIO_PIN_41 @ 0XFF1800A4

	* Level 0 Mux Select 0= Level 1 Mux Output 1= gem1, Output, gem1_rgmii_txd
	* [2]- (TX RGMII data)
	*  PSU_IOU_SLCR_MIO_PIN_41_L0_SEL                              0

	* Level 1 Mux Select 0= Level 2 Mux Output 1= Not Used
	*  PSU_IOU_SLCR_MIO_PIN_41_L1_SEL                              0

	* Level 2 Mux Select 0= Level 3 Mux Output 1= sd0, Input, sd0_data_in[0]-
	* (8-bit Data bus) = sd0, Output, sdio0_data_out[0]- (8-bit Data bus) 2= s
	* d1, Input, sd1_data_in[6]- (8-bit Data bus) = sd1, Output, sdio1_data_ou
	* t[6]- (8-bit Data bus) 3= Not Used
	*  PSU_IOU_SLCR_MIO_PIN_41_L2_SEL                              2

	* Level 3 Mux Select 0= gpio1, Input, gpio_1_pin_in[15]- (GPIO bank 1) 0=
	* gpio1, Output, gpio_1_pin_out[15]- (GPIO bank 1) 1= can1, Input, can1_ph
	* y_rx- (Can RX signal) 2= i2c1, Input, i2c1_sda_input- (SDA signal) 2= i2
	* c1, Output, i2c1_sda_out- (SDA signal) 3= pjtag, Input, pjtag_tms- (PJTA
	* G TMS) 4= spi0, Input, spi0_n_ss_in- (SPI Master Selects) 4= spi0, Outpu
	* t, spi0_n_ss_out[0]- (SPI Master Selects) 5= ttc3, Output, ttc3_wave_out
	* - (TTC Waveform Clock) 6= ua1, Input, ua1_rxd- (UART receiver serial inp
	* ut) 7= trace, Output, tracedq[1]- (Trace Port Databus)
	*  PSU_IOU_SLCR_MIO_PIN_41_L3_SEL                              0

	* Configures MIO Pin 41 peripheral interface mapping
	* (OFFSET, MASK, VALUE)      (0XFF1800A4, 0x000000FEU ,0x00000010U)
	*/
	PSU_Mask_Write( IOU_SLCR_MIO_PIN_41_OFFSET, 0x000000FEU, 0x00000010U );
	/*##################################################################### */

	/*
	* Register : MIO_PIN_42 @ 0XFF1800A8

	* Level 0 Mux Select 0= Level 1 Mux Output 1= gem1, Output, gem1_rgmii_txd
	* [3]- (TX RGMII data)
	*  PSU_IOU_SLCR_MIO_PIN_42_L0_SEL                              0

	* Level 1 Mux Select 0= Level 2 Mux Output 1= Not Used
	*  PSU_IOU_SLCR_MIO_PIN_42_L1_SEL                              0

	* Level 2 Mux Select 0= Level 3 Mux Output 1= sd0, Input, sd0_data_in[1]-
	* (8-bit Data bus) = sd0, Output, sdio0_data_out[1]- (8-bit Data bus) 2= s
	* d1, Input, sd1_data_in[7]- (8-bit Data bus) = sd1, Output, sdio1_data_ou
	* t[7]- (8-bit Data bus) 3= Not Used
	*  PSU_IOU_SLCR_MIO_PIN_42_L2_SEL                              2

	* Level 3 Mux Select 0= gpio1, Input, gpio_1_pin_in[16]- (GPIO bank 1) 0=
	* gpio1, Output, gpio_1_pin_out[16]- (GPIO bank 1) 1= can0, Input, can0_ph
	* y_rx- (Can RX signal) 2= i2c0, Input, i2c0_scl_input- (SCL signal) 2= i2
	* c0, Output, i2c0_scl_out- (SCL signal) 3= swdt0, Input, swdt0_clk_in- (W
	* atch Dog Timer Input clock) 4= spi0, Input, spi0_mi- (MISO signal) 4= sp
	* i0, Output, spi0_so- (MISO signal) 5= ttc2, Input, ttc2_clk_in- (TTC Clo
	* ck) 6= ua0, Input, ua0_rxd- (UART receiver serial input) 7= trace, Outpu
	* t, tracedq[2]- (Trace Port Databus)
	*  PSU_IOU_SLCR_MIO_PIN_42_L3_SEL                              0

	* Configures MIO Pin 42 peripheral interface mapping
	* (OFFSET, MASK, VALUE)      (0XFF1800A8, 0x000000FEU ,0x00000010U)
	*/
	PSU_Mask_Write( IOU_SLCR_MIO_PIN_42_OFFSET, 0x000000FEU, 0x00000010U );
	/*##################################################################### */

	/*
	* Register : MIO_PIN_43 @ 0XFF1800AC

	* Level 0 Mux Select 0= Level 1 Mux Output 1= gem1, Output, gem1_rgmii_tx_
	* ctl- (TX RGMII control)
	*  PSU_IOU_SLCR_MIO_PIN_43_L0_SEL                              0

	* Level 1 Mux Select 0= Level 2 Mux Output 1= Not Used
	*  PSU_IOU_SLCR_MIO_PIN_43_L1_SEL                              0

	* Level 2 Mux Select 0= Level 3 Mux Output 1= sd0, Input, sd0_data_in[2]-
	* (8-bit Data bus) = sd0, Output, sdio0_data_out[2]- (8-bit Data bus) 2= s
	* d1, Output, sdio1_bus_pow- (SD card bus power) 3= Not Used
	*  PSU_IOU_SLCR_MIO_PIN_43_L2_SEL                              2

	* Level 3 Mux Select 0= gpio1, Input, gpio_1_pin_in[17]- (GPIO bank 1) 0=
	* gpio1, Output, gpio_1_pin_out[17]- (GPIO bank 1) 1= can0, Output, can0_p
	* hy_tx- (Can TX signal) 2= i2c0, Input, i2c0_sda_input- (SDA signal) 2= i
	* 2c0, Output, i2c0_sda_out- (SDA signal) 3= swdt0, Output, swdt0_rst_out-
	*  (Watch Dog Timer Output clock) 4= spi0, Output, spi0_mo- (MOSI signal)
	* 4= spi0, Input, spi0_si- (MOSI signal) 5= ttc2, Output, ttc2_wave_out- (
	* TTC Waveform Clock) 6= ua0, Output, ua0_txd- (UART transmitter serial ou
	* tput) 7= trace, Output, tracedq[3]- (Trace Port Databus)
	*  PSU_IOU_SLCR_MIO_PIN_43_L3_SEL                              0

	* Configures MIO Pin 43 peripheral interface mapping
	* (OFFSET, MASK, VALUE)      (0XFF1800AC, 0x000000FEU ,0x00000010U)
	*/
	PSU_Mask_Write( IOU_SLCR_MIO_PIN_43_OFFSET, 0x000000FEU, 0x00000010U );
	/*##################################################################### */

	/*
	* Register : MIO_PIN_44 @ 0XFF1800B0

	* Level 0 Mux Select 0= Level 1 Mux Output 1= gem1, Input, gem1_rgmii_rx_c
	* lk- (RX RGMII clock)
	*  PSU_IOU_SLCR_MIO_PIN_44_L0_SEL                              0

	* Level 1 Mux Select 0= Level 2 Mux Output 1= Not Used
	*  PSU_IOU_SLCR_MIO_PIN_44_L1_SEL                              0

	* Level 2 Mux Select 0= Level 3 Mux Output 1= sd0, Input, sd0_data_in[3]-
	* (8-bit Data bus) = sd0, Output, sdio0_data_out[3]- (8-bit Data bus) 2= s
	* d1, Input, sdio1_wp- (SD card write protect from connector) 3= Not Used
	*  PSU_IOU_SLCR_MIO_PIN_44_L2_SEL                              0

	* Level 3 Mux Select 0= gpio1, Input, gpio_1_pin_in[18]- (GPIO bank 1) 0=
	* gpio1, Output, gpio_1_pin_out[18]- (GPIO bank 1) 1= can1, Output, can1_p
	* hy_tx- (Can TX signal) 2= i2c1, Input, i2c1_scl_input- (SCL signal) 2= i
	* 2c1, Output, i2c1_scl_out- (SCL signal) 3= swdt1, Input, swdt1_clk_in- (
	* Watch Dog Timer Input clock) 4= spi1, Input, spi1_sclk_in- (SPI Clock) 4
	* = spi1, Output, spi1_sclk_out- (SPI Clock) 5= ttc1, Input, ttc1_clk_in-
	* (TTC Clock) 6= ua1, Output, ua1_txd- (UART transmitter serial output) 7=
	*  Not Used
	*  PSU_IOU_SLCR_MIO_PIN_44_L3_SEL                              0

	* Configures MIO Pin 44 peripheral interface mapping
	* (OFFSET, MASK, VALUE)      (0XFF1800B0, 0x000000FEU ,0x00000000U)
	*/
	PSU_Mask_Write( IOU_SLCR_MIO_PIN_44_OFFSET, 0x000000FEU, 0x00000000U );
	/*##################################################################### */

	/*
	* Register : MIO_PIN_45 @ 0XFF1800B4

	* Level 0 Mux Select 0= Level 1 Mux Output 1= gem1, Input, gem1_rgmii_rxd[
	* 0]- (RX RGMII data)
	*  PSU_IOU_SLCR_MIO_PIN_45_L0_SEL                              0

	* Level 1 Mux Select 0= Level 2 Mux Output 1= Not Used
	*  PSU_IOU_SLCR_MIO_PIN_45_L1_SEL                              0

	* Level 2 Mux Select 0= Level 3 Mux Output 1= sd0, Input, sd0_data_in[4]-
	* (8-bit Data bus) = sd0, Output, sdio0_data_out[4]- (8-bit Data bus) 2= s
	* d1, Input, sdio1_cd_n- (SD card detect from connector) 3= Not Used
	*  PSU_IOU_SLCR_MIO_PIN_45_L2_SEL                              2

	* Level 3 Mux Select 0= gpio1, Input, gpio_1_pin_in[19]- (GPIO bank 1) 0=
	* gpio1, Output, gpio_1_pin_out[19]- (GPIO bank 1) 1= can1, Input, can1_ph
	* y_rx- (Can RX signal) 2= i2c1, Input, i2c1_sda_input- (SDA signal) 2= i2
	* c1, Output, i2c1_sda_out- (SDA signal) 3= swdt1, Output, swdt1_rst_out-
	* (Watch Dog Timer Output clock) 4= spi1, Output, spi1_n_ss_out[2]- (SPI M
	* aster Selects) 5= ttc1, Output, ttc1_wave_out- (TTC Waveform Clock) 6= u
	* a1, Input, ua1_rxd- (UART receiver serial input) 7= Not Used
	*  PSU_IOU_SLCR_MIO_PIN_45_L3_SEL                              0

	* Configures MIO Pin 45 peripheral interface mapping
	* (OFFSET, MASK, VALUE)      (0XFF1800B4, 0x000000FEU ,0x00000010U)
	*/
	PSU_Mask_Write( IOU_SLCR_MIO_PIN_45_OFFSET, 0x000000FEU, 0x00000010U );
	/*##################################################################### */

	/*
	* Register : MIO_PIN_46 @ 0XFF1800B8

	* Level 0 Mux Select 0= Level 1 Mux Output 1= gem1, Input, gem1_rgmii_rxd[
	* 1]- (RX RGMII data)
	*  PSU_IOU_SLCR_MIO_PIN_46_L0_SEL                              0

	* Level 1 Mux Select 0= Level 2 Mux Output 1= Not Used
	*  PSU_IOU_SLCR_MIO_PIN_46_L1_SEL                              0

	* Level 2 Mux Select 0= Level 3 Mux Output 1= sd0, Input, sd0_data_in[5]-
	* (8-bit Data bus) = sd0, Output, sdio0_data_out[5]- (8-bit Data bus) 2= s
	* d1, Input, sd1_data_in[0]- (8-bit Data bus) = sd1, Output, sdio1_data_ou
	* t[0]- (8-bit Data bus) 3= Not Used
	*  PSU_IOU_SLCR_MIO_PIN_46_L2_SEL                              2

	* Level 3 Mux Select 0= gpio1, Input, gpio_1_pin_in[20]- (GPIO bank 1) 0=
	* gpio1, Output, gpio_1_pin_out[20]- (GPIO bank 1) 1= can0, Input, can0_ph
	* y_rx- (Can RX signal) 2= i2c0, Input, i2c0_scl_input- (SCL signal) 2= i2
	* c0, Output, i2c0_scl_out- (SCL signal) 3= swdt0, Input, swdt0_clk_in- (W
	* atch Dog Timer Input clock) 4= spi1, Output, spi1_n_ss_out[1]- (SPI Mast
	* er Selects) 5= ttc0, Input, ttc0_clk_in- (TTC Clock) 6= ua0, Input, ua0_
	* rxd- (UART receiver serial input) 7= Not Used
	*  PSU_IOU_SLCR_MIO_PIN_46_L3_SEL                              0

	* Configures MIO Pin 46 peripheral interface mapping
	* (OFFSET, MASK, VALUE)      (0XFF1800B8, 0x000000FEU ,0x00000010U)
	*/
	PSU_Mask_Write( IOU_SLCR_MIO_PIN_46_OFFSET, 0x000000FEU, 0x00000010U );
	/*##################################################################### */

	/*
	* Register : MIO_PIN_47 @ 0XFF1800BC

	* Level 0 Mux Select 0= Level 1 Mux Output 1= gem1, Input, gem1_rgmii_rxd[
	* 2]- (RX RGMII data)
	*  PSU_IOU_SLCR_MIO_PIN_47_L0_SEL                              0

	* Level 1 Mux Select 0= Level 2 Mux Output 1= Not Used
	*  PSU_IOU_SLCR_MIO_PIN_47_L1_SEL                              0

	* Level 2 Mux Select 0= Level 3 Mux Output 1= sd0, Input, sd0_data_in[6]-
	* (8-bit Data bus) = sd0, Output, sdio0_data_out[6]- (8-bit Data bus) 2= s
	* d1, Input, sd1_data_in[1]- (8-bit Data bus) = sd1, Output, sdio1_data_ou
	* t[1]- (8-bit Data bus) 3= Not Used
	*  PSU_IOU_SLCR_MIO_PIN_47_L2_SEL                              2

	* Level 3 Mux Select 0= gpio1, Input, gpio_1_pin_in[21]- (GPIO bank 1) 0=
	* gpio1, Output, gpio_1_pin_out[21]- (GPIO bank 1) 1= can0, Output, can0_p
	* hy_tx- (Can TX signal) 2= i2c0, Input, i2c0_sda_input- (SDA signal) 2= i
	* 2c0, Output, i2c0_sda_out- (SDA signal) 3= swdt0, Output, swdt0_rst_out-
	*  (Watch Dog Timer Output clock) 4= spi1, Input, spi1_n_ss_in- (SPI Maste
	* r Selects) 4= spi1, Output, spi1_n_ss_out[0]- (SPI Master Selects) 5= tt
	* c0, Output, ttc0_wave_out- (TTC Waveform Clock) 6= ua0, Output, ua0_txd-
	*  (UART transmitter serial output) 7= Not Used
	*  PSU_IOU_SLCR_MIO_PIN_47_L3_SEL                              0

	* Configures MIO Pin 47 peripheral interface mapping
	* (OFFSET, MASK, VALUE)      (0XFF1800BC, 0x000000FEU ,0x00000010U)
	*/
	PSU_Mask_Write( IOU_SLCR_MIO_PIN_47_OFFSET, 0x000000FEU, 0x00000010U );
	/*##################################################################### */

	/*
	* Register : MIO_PIN_48 @ 0XFF1800C0

	* Level 0 Mux Select 0= Level 1 Mux Output 1= gem1, Input, gem1_rgmii_rxd[
	* 3]- (RX RGMII data)
	*  PSU_IOU_SLCR_MIO_PIN_48_L0_SEL                              0

	* Level 1 Mux Select 0= Level 2 Mux Output 1= Not Used
	*  PSU_IOU_SLCR_MIO_PIN_48_L1_SEL                              0

	* Level 2 Mux Select 0= Level 3 Mux Output 1= sd0, Input, sd0_data_in[7]-
	* (8-bit Data bus) = sd0, Output, sdio0_data_out[7]- (8-bit Data bus) 2= s
	* d1, Input, sd1_data_in[2]- (8-bit Data bus) = sd1, Output, sdio1_data_ou
	* t[2]- (8-bit Data bus) 3= Not Used
	*  PSU_IOU_SLCR_MIO_PIN_48_L2_SEL                              2

	* Level 3 Mux Select 0= gpio1, Input, gpio_1_pin_in[22]- (GPIO bank 1) 0=
	* gpio1, Output, gpio_1_pin_out[22]- (GPIO bank 1) 1= can1, Output, can1_p
	* hy_tx- (Can TX signal) 2= i2c1, Input, i2c1_scl_input- (SCL signal) 2= i
	* 2c1, Output, i2c1_scl_out- (SCL signal) 3= swdt1, Input, swdt1_clk_in- (
	* Watch Dog Timer Input clock) 4= spi1, Input, spi1_mi- (MISO signal) 4= s
	* pi1, Output, spi1_so- (MISO signal) 5= ttc3, Input, ttc3_clk_in- (TTC Cl
	* ock) 6= ua1, Output, ua1_txd- (UART transmitter serial output) 7= Not Us
	* ed
	*  PSU_IOU_SLCR_MIO_PIN_48_L3_SEL                              0

	* Configures MIO Pin 48 peripheral interface mapping
	* (OFFSET, MASK, VALUE)      (0XFF1800C0, 0x000000FEU ,0x00000010U)
	*/
	PSU_Mask_Write( IOU_SLCR_MIO_PIN_48_OFFSET, 0x000000FEU, 0x00000010U );
	/*##################################################################### */

	/*
	* Register : MIO_PIN_49 @ 0XFF1800C4

	* Level 0 Mux Select 0= Level 1 Mux Output 1= gem1, Input, gem1_rgmii_rx_c
	* tl- (RX RGMII control )
	*  PSU_IOU_SLCR_MIO_PIN_49_L0_SEL                              0

	* Level 1 Mux Select 0= Level 2 Mux Output 1= Not Used
	*  PSU_IOU_SLCR_MIO_PIN_49_L1_SEL                              0

	* Level 2 Mux Select 0= Level 3 Mux Output 1= sd0, Output, sdio0_bus_pow-
	* (SD card bus power) 2= sd1, Input, sd1_data_in[3]- (8-bit Data bus) = sd
	* 1, Output, sdio1_data_out[3]- (8-bit Data bus) 3= Not Used
	*  PSU_IOU_SLCR_MIO_PIN_49_L2_SEL                              2

	* Level 3 Mux Select 0= gpio1, Input, gpio_1_pin_in[23]- (GPIO bank 1) 0=
	* gpio1, Output, gpio_1_pin_out[23]- (GPIO bank 1) 1= can1, Input, can1_ph
	* y_rx- (Can RX signal) 2= i2c1, Input, i2c1_sda_input- (SDA signal) 2= i2
	* c1, Output, i2c1_sda_out- (SDA signal) 3= swdt1, Output, swdt1_rst_out-
	* (Watch Dog Timer Output clock) 4= spi1, Output, spi1_mo- (MOSI signal) 4
	* = spi1, Input, spi1_si- (MOSI signal) 5= ttc3, Output, ttc3_wave_out- (T
	* TC Waveform Clock) 6= ua1, Input, ua1_rxd- (UART receiver serial input)
	* 7= Not Used
	*  PSU_IOU_SLCR_MIO_PIN_49_L3_SEL                              0

	* Configures MIO Pin 49 peripheral interface mapping
	* (OFFSET, MASK, VALUE)      (0XFF1800C4, 0x000000FEU ,0x00000010U)
	*/
	PSU_Mask_Write( IOU_SLCR_MIO_PIN_49_OFFSET, 0x000000FEU, 0x00000010U );
	/*##################################################################### */

	/*
	* Register : MIO_PIN_50 @ 0XFF1800C8

	* Level 0 Mux Select 0= Level 1 Mux Output 1= gem_tsu, Input, gem_tsu_clk-
	*  (TSU clock)
	*  PSU_IOU_SLCR_MIO_PIN_50_L0_SEL                              0

	* Level 1 Mux Select 0= Level 2 Mux Output 1= Not Used
	*  PSU_IOU_SLCR_MIO_PIN_50_L1_SEL                              0

	* Level 2 Mux Select 0= Level 3 Mux Output 1= sd0, Input, sdio0_wp- (SD ca
	* rd write protect from connector) 2= sd1, Input, sd1_cmd_in- (Command Ind
	* icator) = sd1, Output, sdio1_cmd_out- (Command Indicator) 3= Not Used
	*  PSU_IOU_SLCR_MIO_PIN_50_L2_SEL                              2

	* Level 3 Mux Select 0= gpio1, Input, gpio_1_pin_in[24]- (GPIO bank 1) 0=
	* gpio1, Output, gpio_1_pin_out[24]- (GPIO bank 1) 1= can0, Input, can0_ph
	* y_rx- (Can RX signal) 2= i2c0, Input, i2c0_scl_input- (SCL signal) 2= i2
	* c0, Output, i2c0_scl_out- (SCL signal) 3= swdt0, Input, swdt0_clk_in- (W
	* atch Dog Timer Input clock) 4= mdio1, Output, gem1_mdc- (MDIO Clock) 5=
	* ttc2, Input, ttc2_clk_in- (TTC Clock) 6= ua0, Input, ua0_rxd- (UART rece
	* iver serial input) 7= Not Used
	*  PSU_IOU_SLCR_MIO_PIN_50_L3_SEL                              0

	* Configures MIO Pin 50 peripheral interface mapping
	* (OFFSET, MASK, VALUE)      (0XFF1800C8, 0x000000FEU ,0x00000010U)
	*/
	PSU_Mask_Write( IOU_SLCR_MIO_PIN_50_OFFSET, 0x000000FEU, 0x00000010U );
	/*##################################################################### */

	/*
	* Register : MIO_PIN_51 @ 0XFF1800CC

	* Level 0 Mux Select 0= Level 1 Mux Output 1= gem_tsu, Input, gem_tsu_clk-
	*  (TSU clock)
	*  PSU_IOU_SLCR_MIO_PIN_51_L0_SEL                              0

	* Level 1 Mux Select 0= Level 2 Mux Output 1= Not Used
	*  PSU_IOU_SLCR_MIO_PIN_51_L1_SEL                              0

	* Level 2 Mux Select 0= Level 3 Mux Output 1= Not Used 2= sd1, Output, sdi
	* o1_clk_out- (SDSDIO clock) 3= Not Used
	*  PSU_IOU_SLCR_MIO_PIN_51_L2_SEL                              2

	* Level 3 Mux Select 0= gpio1, Input, gpio_1_pin_in[25]- (GPIO bank 1) 0=
	* gpio1, Output, gpio_1_pin_out[25]- (GPIO bank 1) 1= can0, Output, can0_p
	* hy_tx- (Can TX signal) 2= i2c0, Input, i2c0_sda_input- (SDA signal) 2= i
	* 2c0, Output, i2c0_sda_out- (SDA signal) 3= swdt0, Output, swdt0_rst_out-
	*  (Watch Dog Timer Output clock) 4= mdio1, Input, gem1_mdio_in- (MDIO Dat
	* a) 4= mdio1, Output, gem1_mdio_out- (MDIO Data) 5= ttc2, Output, ttc2_wa
	* ve_out- (TTC Waveform Clock) 6= ua0, Output, ua0_txd- (UART transmitter
	* serial output) 7= Not Used
	*  PSU_IOU_SLCR_MIO_PIN_51_L3_SEL                              0

	* Configures MIO Pin 51 peripheral interface mapping
	* (OFFSET, MASK, VALUE)      (0XFF1800CC, 0x000000FEU ,0x00000010U)
	*/
	PSU_Mask_Write( IOU_SLCR_MIO_PIN_51_OFFSET, 0x000000FEU, 0x00000010U );
	/*##################################################################### */

	/*
	* Register : MIO_PIN_52 @ 0XFF1800D0

	* Level 0 Mux Select 0= Level 1 Mux Output 1= gem2, Output, gem2_rgmii_tx_
	* clk- (TX RGMII clock)
	*  PSU_IOU_SLCR_MIO_PIN_52_L0_SEL                              0

	* Level 1 Mux Select 0= Level 2 Mux Output 1= usb0, Input, usb0_ulpi_clk_i
	* n- (ULPI Clock)
	*  PSU_IOU_SLCR_MIO_PIN_52_L1_SEL                              1

	* Level 2 Mux Select 0= Level 3 Mux Output 1= Not Used 2= Not Used 3= Not
	* Used
	*  PSU_IOU_SLCR_MIO_PIN_52_L2_SEL                              0

	* Level 3 Mux Select 0= gpio2, Input, gpio_2_pin_in[0]- (GPIO bank 2) 0= g
	* pio2, Output, gpio_2_pin_out[0]- (GPIO bank 2) 1= can1, Output, can1_phy
	* _tx- (Can TX signal) 2= i2c1, Input, i2c1_scl_input- (SCL signal) 2= i2c
	* 1, Output, i2c1_scl_out- (SCL signal) 3= pjtag, Input, pjtag_tck- (PJTAG
	*  TCK) 4= spi0, Input, spi0_sclk_in- (SPI Clock) 4= spi0, Output, spi0_sc
	* lk_out- (SPI Clock) 5= ttc1, Input, ttc1_clk_in- (TTC Clock) 6= ua1, Out
	* put, ua1_txd- (UART transmitter serial output) 7= trace, Output, trace_c
	* lk- (Trace Port Clock)
	*  PSU_IOU_SLCR_MIO_PIN_52_L3_SEL                              0

	* Configures MIO Pin 52 peripheral interface mapping
	* (OFFSET, MASK, VALUE)      (0XFF1800D0, 0x000000FEU ,0x00000004U)
	*/
	PSU_Mask_Write( IOU_SLCR_MIO_PIN_52_OFFSET, 0x000000FEU, 0x00000004U );
	/*##################################################################### */

	/*
	* Register : MIO_PIN_53 @ 0XFF1800D4

	* Level 0 Mux Select 0= Level 1 Mux Output 1= gem2, Output, gem2_rgmii_txd
	* [0]- (TX RGMII data)
	*  PSU_IOU_SLCR_MIO_PIN_53_L0_SEL                              0

	* Level 1 Mux Select 0= Level 2 Mux Output 1= usb0, Input, usb0_ulpi_dir-
	* (Data bus direction control)
	*  PSU_IOU_SLCR_MIO_PIN_53_L1_SEL                              1

	* Level 2 Mux Select 0= Level 3 Mux Output 1= Not Used 2= Not Used 3= Not
	* Used
	*  PSU_IOU_SLCR_MIO_PIN_53_L2_SEL                              0

	* Level 3 Mux Select 0= gpio2, Input, gpio_2_pin_in[1]- (GPIO bank 2) 0= g
	* pio2, Output, gpio_2_pin_out[1]- (GPIO bank 2) 1= can1, Input, can1_phy_
	* rx- (Can RX signal) 2= i2c1, Input, i2c1_sda_input- (SDA signal) 2= i2c1
	* , Output, i2c1_sda_out- (SDA signal) 3= pjtag, Input, pjtag_tdi- (PJTAG
	* TDI) 4= spi0, Output, spi0_n_ss_out[2]- (SPI Master Selects) 5= ttc1, Ou
	* tput, ttc1_wave_out- (TTC Waveform Clock) 6= ua1, Input, ua1_rxd- (UART
	* receiver serial input) 7= trace, Output, trace_ctl- (Trace Port Control
	* Signal)
	*  PSU_IOU_SLCR_MIO_PIN_53_L3_SEL                              0

	* Configures MIO Pin 53 peripheral interface mapping
	* (OFFSET, MASK, VALUE)      (0XFF1800D4, 0x000000FEU ,0x00000004U)
	*/
	PSU_Mask_Write( IOU_SLCR_MIO_PIN_53_OFFSET, 0x000000FEU, 0x00000004U );
	/*##################################################################### */

	/*
	* Register : MIO_PIN_54 @ 0XFF1800D8

	* Level 0 Mux Select 0= Level 1 Mux Output 1= gem2, Output, gem2_rgmii_txd
	* [1]- (TX RGMII data)
	*  PSU_IOU_SLCR_MIO_PIN_54_L0_SEL                              0

	* Level 1 Mux Select 0= Level 2 Mux Output 1= usb0, Input, usb0_ulpi_rx_da
	* ta[2]- (ULPI data bus) 1= usb0, Output, usb0_ulpi_tx_data[2]- (ULPI data
	*  bus)
	*  PSU_IOU_SLCR_MIO_PIN_54_L1_SEL                              1

	* Level 2 Mux Select 0= Level 3 Mux Output 1= Not Used 2= Not Used 3= Not
	* Used
	*  PSU_IOU_SLCR_MIO_PIN_54_L2_SEL                              0

	* Level 3 Mux Select 0= gpio2, Input, gpio_2_pin_in[2]- (GPIO bank 2) 0= g
	* pio2, Output, gpio_2_pin_out[2]- (GPIO bank 2) 1= can0, Input, can0_phy_
	* rx- (Can RX signal) 2= i2c0, Input, i2c0_scl_input- (SCL signal) 2= i2c0
	* , Output, i2c0_scl_out- (SCL signal) 3= pjtag, Output, pjtag_tdo- (PJTAG
	*  TDO) 4= spi0, Output, spi0_n_ss_out[1]- (SPI Master Selects) 5= ttc0, I
	* nput, ttc0_clk_in- (TTC Clock) 6= ua0, Input, ua0_rxd- (UART receiver se
	* rial input) 7= trace, Output, tracedq[0]- (Trace Port Databus)
	*  PSU_IOU_SLCR_MIO_PIN_54_L3_SEL                              0

	* Configures MIO Pin 54 peripheral interface mapping
	* (OFFSET, MASK, VALUE)      (0XFF1800D8, 0x000000FEU ,0x00000004U)
	*/
	PSU_Mask_Write( IOU_SLCR_MIO_PIN_54_OFFSET, 0x000000FEU, 0x00000004U );
	/*##################################################################### */

	/*
	* Register : MIO_PIN_55 @ 0XFF1800DC

	* Level 0 Mux Select 0= Level 1 Mux Output 1= gem2, Output, gem2_rgmii_txd
	* [2]- (TX RGMII data)
	*  PSU_IOU_SLCR_MIO_PIN_55_L0_SEL                              0

	* Level 1 Mux Select 0= Level 2 Mux Output 1= usb0, Input, usb0_ulpi_nxt-
	* (Data flow control signal from the PHY)
	*  PSU_IOU_SLCR_MIO_PIN_55_L1_SEL                              1

	* Level 2 Mux Select 0= Level 3 Mux Output 1= Not Used 2= Not Used 3= Not
	* Used
	*  PSU_IOU_SLCR_MIO_PIN_55_L2_SEL                              0

	* Level 3 Mux Select 0= gpio2, Input, gpio_2_pin_in[3]- (GPIO bank 2) 0= g
	* pio2, Output, gpio_2_pin_out[3]- (GPIO bank 2) 1= can0, Output, can0_phy
	* _tx- (Can TX signal) 2= i2c0, Input, i2c0_sda_input- (SDA signal) 2= i2c
	* 0, Output, i2c0_sda_out- (SDA signal) 3= pjtag, Input, pjtag_tms- (PJTAG
	*  TMS) 4= spi0, Input, spi0_n_ss_in- (SPI Master Selects) 4= spi0, Output
	* , spi0_n_ss_out[0]- (SPI Master Selects) 5= ttc0, Output, ttc0_wave_out-
	*  (TTC Waveform Clock) 6= ua0, Output, ua0_txd- (UART transmitter serial
	* output) 7= trace, Output, tracedq[1]- (Trace Port Databus)
	*  PSU_IOU_SLCR_MIO_PIN_55_L3_SEL                              0

	* Configures MIO Pin 55 peripheral interface mapping
	* (OFFSET, MASK, VALUE)      (0XFF1800DC, 0x000000FEU ,0x00000004U)
	*/
	PSU_Mask_Write( IOU_SLCR_MIO_PIN_55_OFFSET, 0x000000FEU, 0x00000004U );
	/*##################################################################### */

	/*
	* Register : MIO_PIN_56 @ 0XFF1800E0

	* Level 0 Mux Select 0= Level 1 Mux Output 1= gem2, Output, gem2_rgmii_txd
	* [3]- (TX RGMII data)
	*  PSU_IOU_SLCR_MIO_PIN_56_L0_SEL                              0

	* Level 1 Mux Select 0= Level 2 Mux Output 1= usb0, Input, usb0_ulpi_rx_da
	* ta[0]- (ULPI data bus) 1= usb0, Output, usb0_ulpi_tx_data[0]- (ULPI data
	*  bus)
	*  PSU_IOU_SLCR_MIO_PIN_56_L1_SEL                              1

	* Level 2 Mux Select 0= Level 3 Mux Output 1= Not Used 2= Not Used 3= Not
	* Used
	*  PSU_IOU_SLCR_MIO_PIN_56_L2_SEL                              0

	* Level 3 Mux Select 0= gpio2, Input, gpio_2_pin_in[4]- (GPIO bank 2) 0= g
	* pio2, Output, gpio_2_pin_out[4]- (GPIO bank 2) 1= can1, Output, can1_phy
	* _tx- (Can TX signal) 2= i2c1, Input, i2c1_scl_input- (SCL signal) 2= i2c
	* 1, Output, i2c1_scl_out- (SCL signal) 3= swdt1, Input, swdt1_clk_in- (Wa
	* tch Dog Timer Input clock) 4= spi0, Input, spi0_mi- (MISO signal) 4= spi
	* 0, Output, spi0_so- (MISO signal) 5= ttc3, Input, ttc3_clk_in- (TTC Cloc
	* k) 6= ua1, Output, ua1_txd- (UART transmitter serial output) 7= trace, O
	* utput, tracedq[2]- (Trace Port Databus)
	*  PSU_IOU_SLCR_MIO_PIN_56_L3_SEL                              0

	* Configures MIO Pin 56 peripheral interface mapping
	* (OFFSET, MASK, VALUE)      (0XFF1800E0, 0x000000FEU ,0x00000004U)
	*/
	PSU_Mask_Write( IOU_SLCR_MIO_PIN_56_OFFSET, 0x000000FEU, 0x00000004U );
	/*##################################################################### */

	/*
	* Register : MIO_PIN_57 @ 0XFF1800E4

	* Level 0 Mux Select 0= Level 1 Mux Output 1= gem2, Output, gem2_rgmii_tx_
	* ctl- (TX RGMII control)
	*  PSU_IOU_SLCR_MIO_PIN_57_L0_SEL                              0

	* Level 1 Mux Select 0= Level 2 Mux Output 1= usb0, Input, usb0_ulpi_rx_da
	* ta[1]- (ULPI data bus) 1= usb0, Output, usb0_ulpi_tx_data[1]- (ULPI data
	*  bus)
	*  PSU_IOU_SLCR_MIO_PIN_57_L1_SEL                              1

	* Level 2 Mux Select 0= Level 3 Mux Output 1= Not Used 2= Not Used 3= Not
	* Used
	*  PSU_IOU_SLCR_MIO_PIN_57_L2_SEL                              0

	* Level 3 Mux Select 0= gpio2, Input, gpio_2_pin_in[5]- (GPIO bank 2) 0= g
	* pio2, Output, gpio_2_pin_out[5]- (GPIO bank 2) 1= can1, Input, can1_phy_
	* rx- (Can RX signal) 2= i2c1, Input, i2c1_sda_input- (SDA signal) 2= i2c1
	* , Output, i2c1_sda_out- (SDA signal) 3= swdt1, Output, swdt1_rst_out- (W
	* atch Dog Timer Output clock) 4= spi0, Output, spi0_mo- (MOSI signal) 4=
	* spi0, Input, spi0_si- (MOSI signal) 5= ttc3, Output, ttc3_wave_out- (TTC
	*  Waveform Clock) 6= ua1, Input, ua1_rxd- (UART receiver serial input) 7=
	*  trace, Output, tracedq[3]- (Trace Port Databus)
	*  PSU_IOU_SLCR_MIO_PIN_57_L3_SEL                              0

	* Configures MIO Pin 57 peripheral interface mapping
	* (OFFSET, MASK, VALUE)      (0XFF1800E4, 0x000000FEU ,0x00000004U)
	*/
	PSU_Mask_Write( IOU_SLCR_MIO_PIN_57_OFFSET, 0x000000FEU, 0x00000004U );
	/*##################################################################### */

	/*
	* Register : MIO_PIN_58 @ 0XFF1800E8

	* Level 0 Mux Select 0= Level 1 Mux Output 1= gem2, Input, gem2_rgmii_rx_c
	* lk- (RX RGMII clock)
	*  PSU_IOU_SLCR_MIO_PIN_58_L0_SEL                              0

	* Level 1 Mux Select 0= Level 2 Mux Output 1= usb0, Output, usb0_ulpi_stp-
	*  (Asserted to end or interrupt transfers)
	*  PSU_IOU_SLCR_MIO_PIN_58_L1_SEL                              1

	* Level 2 Mux Select 0= Level 3 Mux Output 1= Not Used 2= Not Used 3= Not
	* Used
	*  PSU_IOU_SLCR_MIO_PIN_58_L2_SEL                              0

	* Level 3 Mux Select 0= gpio2, Input, gpio_2_pin_in[6]- (GPIO bank 2) 0= g
	* pio2, Output, gpio_2_pin_out[6]- (GPIO bank 2) 1= can0, Input, can0_phy_
	* rx- (Can RX signal) 2= i2c0, Input, i2c0_scl_input- (SCL signal) 2= i2c0
	* , Output, i2c0_scl_out- (SCL signal) 3= pjtag, Input, pjtag_tck- (PJTAG
	* TCK) 4= spi1, Input, spi1_sclk_in- (SPI Clock) 4= spi1, Output, spi1_scl
	* k_out- (SPI Clock) 5= ttc2, Input, ttc2_clk_in- (TTC Clock) 6= ua0, Inpu
	* t, ua0_rxd- (UART receiver serial input) 7= trace, Output, tracedq[4]- (
	* Trace Port Databus)
	*  PSU_IOU_SLCR_MIO_PIN_58_L3_SEL                              0

	* Configures MIO Pin 58 peripheral interface mapping
	* (OFFSET, MASK, VALUE)      (0XFF1800E8, 0x000000FEU ,0x00000004U)
	*/
	PSU_Mask_Write( IOU_SLCR_MIO_PIN_58_OFFSET, 0x000000FEU, 0x00000004U );
	/*##################################################################### */

	/*
	* Register : MIO_PIN_59 @ 0XFF1800EC

	* Level 0 Mux Select 0= Level 1 Mux Output 1= gem2, Input, gem2_rgmii_rxd[
	* 0]- (RX RGMII data)
	*  PSU_IOU_SLCR_MIO_PIN_59_L0_SEL                              0

	* Level 1 Mux Select 0= Level 2 Mux Output 1= usb0, Input, usb0_ulpi_rx_da
	* ta[3]- (ULPI data bus) 1= usb0, Output, usb0_ulpi_tx_data[3]- (ULPI data
	*  bus)
	*  PSU_IOU_SLCR_MIO_PIN_59_L1_SEL                              1

	* Level 2 Mux Select 0= Level 3 Mux Output 1= Not Used 2= Not Used 3= Not
	* Used
	*  PSU_IOU_SLCR_MIO_PIN_59_L2_SEL                              0

	* Level 3 Mux Select 0= gpio2, Input, gpio_2_pin_in[7]- (GPIO bank 2) 0= g
	* pio2, Output, gpio_2_pin_out[7]- (GPIO bank 2) 1= can0, Output, can0_phy
	* _tx- (Can TX signal) 2= i2c0, Input, i2c0_sda_input- (SDA signal) 2= i2c
	* 0, Output, i2c0_sda_out- (SDA signal) 3= pjtag, Input, pjtag_tdi- (PJTAG
	*  TDI) 4= spi1, Output, spi1_n_ss_out[2]- (SPI Master Selects) 5= ttc2, O
	* utput, ttc2_wave_out- (TTC Waveform Clock) 6= ua0, Output, ua0_txd- (UAR
	* T transmitter serial output) 7= trace, Output, tracedq[5]- (Trace Port D
	* atabus)
	*  PSU_IOU_SLCR_MIO_PIN_59_L3_SEL                              0

	* Configures MIO Pin 59 peripheral interface mapping
	* (OFFSET, MASK, VALUE)      (0XFF1800EC, 0x000000FEU ,0x00000004U)
	*/
	PSU_Mask_Write( IOU_SLCR_MIO_PIN_59_OFFSET, 0x000000FEU, 0x00000004U );
	/*##################################################################### */

	/*
	* Register : MIO_PIN_60 @ 0XFF1800F0

	* Level 0 Mux Select 0= Level 1 Mux Output 1= gem2, Input, gem2_rgmii_rxd[
	* 1]- (RX RGMII data)
	*  PSU_IOU_SLCR_MIO_PIN_60_L0_SEL                              0

	* Level 1 Mux Select 0= Level 2 Mux Output 1= usb0, Input, usb0_ulpi_rx_da
	* ta[4]- (ULPI data bus) 1= usb0, Output, usb0_ulpi_tx_data[4]- (ULPI data
	*  bus)
	*  PSU_IOU_SLCR_MIO_PIN_60_L1_SEL                              1

	* Level 2 Mux Select 0= Level 3 Mux Output 1= Not Used 2= Not Used 3= Not
	* Used
	*  PSU_IOU_SLCR_MIO_PIN_60_L2_SEL                              0

	* Level 3 Mux Select 0= gpio2, Input, gpio_2_pin_in[8]- (GPIO bank 2) 0= g
	* pio2, Output, gpio_2_pin_out[8]- (GPIO bank 2) 1= can1, Output, can1_phy
	* _tx- (Can TX signal) 2= i2c1, Input, i2c1_scl_input- (SCL signal) 2= i2c
	* 1, Output, i2c1_scl_out- (SCL signal) 3= pjtag, Output, pjtag_tdo- (PJTA
	* G TDO) 4= spi1, Output, spi1_n_ss_out[1]- (SPI Master Selects) 5= ttc1,
	* Input, ttc1_clk_in- (TTC Clock) 6= ua1, Output, ua1_txd- (UART transmitt
	* er serial output) 7= trace, Output, tracedq[6]- (Trace Port Databus)
	*  PSU_IOU_SLCR_MIO_PIN_60_L3_SEL                              0

	* Configures MIO Pin 60 peripheral interface mapping
	* (OFFSET, MASK, VALUE)      (0XFF1800F0, 0x000000FEU ,0x00000004U)
	*/
	PSU_Mask_Write( IOU_SLCR_MIO_PIN_60_OFFSET, 0x000000FEU, 0x00000004U );
	/*##################################################################### */

	/*
	* Register : MIO_PIN_61 @ 0XFF1800F4

	* Level 0 Mux Select 0= Level 1 Mux Output 1= gem2, Input, gem2_rgmii_rxd[
	* 2]- (RX RGMII data)
	*  PSU_IOU_SLCR_MIO_PIN_61_L0_SEL                              0

	* Level 1 Mux Select 0= Level 2 Mux Output 1= usb0, Input, usb0_ulpi_rx_da
	* ta[5]- (ULPI data bus) 1= usb0, Output, usb0_ulpi_tx_data[5]- (ULPI data
	*  bus)
	*  PSU_IOU_SLCR_MIO_PIN_61_L1_SEL                              1

	* Level 2 Mux Select 0= Level 3 Mux Output 1= Not Used 2= Not Used 3= Not
	* Used
	*  PSU_IOU_SLCR_MIO_PIN_61_L2_SEL                              0

	* Level 3 Mux Select 0= gpio2, Input, gpio_2_pin_in[9]- (GPIO bank 2) 0= g
	* pio2, Output, gpio_2_pin_out[9]- (GPIO bank 2) 1= can1, Input, can1_phy_
	* rx- (Can RX signal) 2= i2c1, Input, i2c1_sda_input- (SDA signal) 2= i2c1
	* , Output, i2c1_sda_out- (SDA signal) 3= pjtag, Input, pjtag_tms- (PJTAG
	* TMS) 4= spi1, Input, spi1_n_ss_in- (SPI Master Selects) 4= spi1, Output,
	*  spi1_n_ss_out[0]- (SPI Master Selects) 5= ttc1, Output, ttc1_wave_out-
	* (TTC Waveform Clock) 6= ua1, Input, ua1_rxd- (UART receiver serial input
	* ) 7= trace, Output, tracedq[7]- (Trace Port Databus)
	*  PSU_IOU_SLCR_MIO_PIN_61_L3_SEL                              0

	* Configures MIO Pin 61 peripheral interface mapping
	* (OFFSET, MASK, VALUE)      (0XFF1800F4, 0x000000FEU ,0x00000004U)
	*/
	PSU_Mask_Write( IOU_SLCR_MIO_PIN_61_OFFSET, 0x000000FEU, 0x00000004U );
	/*##################################################################### */

	/*
	* Register : MIO_PIN_62 @ 0XFF1800F8

	* Level 0 Mux Select 0= Level 1 Mux Output 1= gem2, Input, gem2_rgmii_rxd[
	* 3]- (RX RGMII data)
	*  PSU_IOU_SLCR_MIO_PIN_62_L0_SEL                              0

	* Level 1 Mux Select 0= Level 2 Mux Output 1= usb0, Input, usb0_ulpi_rx_da
	* ta[6]- (ULPI data bus) 1= usb0, Output, usb0_ulpi_tx_data[6]- (ULPI data
	*  bus)
	*  PSU_IOU_SLCR_MIO_PIN_62_L1_SEL                              1

	* Level 2 Mux Select 0= Level 3 Mux Output 1= Not Used 2= Not Used 3= Not
	* Used
	*  PSU_IOU_SLCR_MIO_PIN_62_L2_SEL                              0

	* Level 3 Mux Select 0= gpio2, Input, gpio_2_pin_in[10]- (GPIO bank 2) 0=
	* gpio2, Output, gpio_2_pin_out[10]- (GPIO bank 2) 1= can0, Input, can0_ph
	* y_rx- (Can RX signal) 2= i2c0, Input, i2c0_scl_input- (SCL signal) 2= i2
	* c0, Output, i2c0_scl_out- (SCL signal) 3= swdt0, Input, swdt0_clk_in- (W
	* atch Dog Timer Input clock) 4= spi1, Input, spi1_mi- (MISO signal) 4= sp
	* i1, Output, spi1_so- (MISO signal) 5= ttc0, Input, ttc0_clk_in- (TTC Clo
	* ck) 6= ua0, Input, ua0_rxd- (UART receiver serial input) 7= trace, Outpu
	* t, tracedq[8]- (Trace Port Databus)
	*  PSU_IOU_SLCR_MIO_PIN_62_L3_SEL                              0

	* Configures MIO Pin 62 peripheral interface mapping
	* (OFFSET, MASK, VALUE)      (0XFF1800F8, 0x000000FEU ,0x00000004U)
	*/
	PSU_Mask_Write( IOU_SLCR_MIO_PIN_62_OFFSET, 0x000000FEU, 0x00000004U );
	/*##################################################################### */

	/*
	* Register : MIO_PIN_63 @ 0XFF1800FC

	* Level 0 Mux Select 0= Level 1 Mux Output 1= gem2, Input, gem2_rgmii_rx_c
	* tl- (RX RGMII control )
	*  PSU_IOU_SLCR_MIO_PIN_63_L0_SEL                              0

	* Level 1 Mux Select 0= Level 2 Mux Output 1= usb0, Input, usb0_ulpi_rx_da
	* ta[7]- (ULPI data bus) 1= usb0, Output, usb0_ulpi_tx_data[7]- (ULPI data
	*  bus)
	*  PSU_IOU_SLCR_MIO_PIN_63_L1_SEL                              1

	* Level 2 Mux Select 0= Level 3 Mux Output 1= Not Used 2= Not Used 3= Not
	* Used
	*  PSU_IOU_SLCR_MIO_PIN_63_L2_SEL                              0

	* Level 3 Mux Select 0= gpio2, Input, gpio_2_pin_in[11]- (GPIO bank 2) 0=
	* gpio2, Output, gpio_2_pin_out[11]- (GPIO bank 2) 1= can0, Output, can0_p
	* hy_tx- (Can TX signal) 2= i2c0, Input, i2c0_sda_input- (SDA signal) 2= i
	* 2c0, Output, i2c0_sda_out- (SDA signal) 3= swdt0, Output, swdt0_rst_out-
	*  (Watch Dog Timer Output clock) 4= spi1, Output, spi1_mo- (MOSI signal)
	* 4= spi1, Input, spi1_si- (MOSI signal) 5= ttc0, Output, ttc0_wave_out- (
	* TTC Waveform Clock) 6= ua0, Output, ua0_txd- (UART transmitter serial ou
	* tput) 7= trace, Output, tracedq[9]- (Trace Port Databus)
	*  PSU_IOU_SLCR_MIO_PIN_63_L3_SEL                              0

	* Configures MIO Pin 63 peripheral interface mapping
	* (OFFSET, MASK, VALUE)      (0XFF1800FC, 0x000000FEU ,0x00000004U)
	*/
	PSU_Mask_Write( IOU_SLCR_MIO_PIN_63_OFFSET, 0x000000FEU, 0x00000004U );
	/*##################################################################### */

	/*
	* Register : MIO_PIN_64 @ 0XFF180100

	* Level 0 Mux Select 0= Level 1 Mux Output 1= gem3, Output, gem3_rgmii_tx_
	* clk- (TX RGMII clock)
	*  PSU_IOU_SLCR_MIO_PIN_64_L0_SEL                              1

	* Level 1 Mux Select 0= Level 2 Mux Output 1= usb1, Input, usb1_ulpi_clk_i
	* n- (ULPI Clock)
	*  PSU_IOU_SLCR_MIO_PIN_64_L1_SEL                              0

	* Level 2 Mux Select 0= Level 3 Mux Output 1= sd0, Output, sdio0_clk_out-
	* (SDSDIO clock) 2= Not Used 3= Not Used
	*  PSU_IOU_SLCR_MIO_PIN_64_L2_SEL                              0

	* Level 3 Mux Select 0= gpio2, Input, gpio_2_pin_in[12]- (GPIO bank 2) 0=
	* gpio2, Output, gpio_2_pin_out[12]- (GPIO bank 2) 1= can1, Output, can1_p
	* hy_tx- (Can TX signal) 2= i2c1, Input, i2c1_scl_input- (SCL signal) 2= i
	* 2c1, Output, i2c1_scl_out- (SCL signal) 3= swdt1, Input, swdt1_clk_in- (
	* Watch Dog Timer Input clock) 4= spi0, Input, spi0_sclk_in- (SPI Clock) 4
	* = spi0, Output, spi0_sclk_out- (SPI Clock) 5= ttc3, Input, ttc3_clk_in-
	* (TTC Clock) 6= ua1, Output, ua1_txd- (UART transmitter serial output) 7=
	*  trace, Output, tracedq[10]- (Trace Port Databus)
	*  PSU_IOU_SLCR_MIO_PIN_64_L3_SEL                              0

	* Configures MIO Pin 64 peripheral interface mapping
	* (OFFSET, MASK, VALUE)      (0XFF180100, 0x000000FEU ,0x00000002U)
	*/
	PSU_Mask_Write( IOU_SLCR_MIO_PIN_64_OFFSET, 0x000000FEU, 0x00000002U );
	/*##################################################################### */

	/*
	* Register : MIO_PIN_65 @ 0XFF180104

	* Level 0 Mux Select 0= Level 1 Mux Output 1= gem3, Output, gem3_rgmii_txd
	* [0]- (TX RGMII data)
	*  PSU_IOU_SLCR_MIO_PIN_65_L0_SEL                              1

	* Level 1 Mux Select 0= Level 2 Mux Output 1= usb1, Input, usb1_ulpi_dir-
	* (Data bus direction control)
	*  PSU_IOU_SLCR_MIO_PIN_65_L1_SEL                              0

	* Level 2 Mux Select 0= Level 3 Mux Output 1= sd0, Input, sdio0_cd_n- (SD
	* card detect from connector) 2= Not Used 3= Not Used
	*  PSU_IOU_SLCR_MIO_PIN_65_L2_SEL                              0

	* Level 3 Mux Select 0= gpio2, Input, gpio_2_pin_in[13]- (GPIO bank 2) 0=
	* gpio2, Output, gpio_2_pin_out[13]- (GPIO bank 2) 1= can1, Input, can1_ph
	* y_rx- (Can RX signal) 2= i2c1, Input, i2c1_sda_input- (SDA signal) 2= i2
	* c1, Output, i2c1_sda_out- (SDA signal) 3= swdt1, Output, swdt1_rst_out-
	* (Watch Dog Timer Output clock) 4= spi0, Output, spi0_n_ss_out[2]- (SPI M
	* aster Selects) 5= ttc3, Output, ttc3_wave_out- (TTC Waveform Clock) 6= u
	* a1, Input, ua1_rxd- (UART receiver serial input) 7= trace, Output, trace
	* dq[11]- (Trace Port Databus)
	*  PSU_IOU_SLCR_MIO_PIN_65_L3_SEL                              0

	* Configures MIO Pin 65 peripheral interface mapping
	* (OFFSET, MASK, VALUE)      (0XFF180104, 0x000000FEU ,0x00000002U)
	*/
	PSU_Mask_Write( IOU_SLCR_MIO_PIN_65_OFFSET, 0x000000FEU, 0x00000002U );
	/*##################################################################### */

	/*
	* Register : MIO_PIN_66 @ 0XFF180108

	* Level 0 Mux Select 0= Level 1 Mux Output 1= gem3, Output, gem3_rgmii_txd
	* [1]- (TX RGMII data)
	*  PSU_IOU_SLCR_MIO_PIN_66_L0_SEL                              1

	* Level 1 Mux Select 0= Level 2 Mux Output 1= usb1, Input, usb1_ulpi_rx_da
	* ta[2]- (ULPI data bus) 1= usb1, Output, usb1_ulpi_tx_data[2]- (ULPI data
	*  bus)
	*  PSU_IOU_SLCR_MIO_PIN_66_L1_SEL                              0

	* Level 2 Mux Select 0= Level 3 Mux Output 1= sd0, Input, sd0_cmd_in- (Com
	* mand Indicator) = sd0, Output, sdio0_cmd_out- (Command Indicator) 2= Not
	*  Used 3= Not Used
	*  PSU_IOU_SLCR_MIO_PIN_66_L2_SEL                              0

	* Level 3 Mux Select 0= gpio2, Input, gpio_2_pin_in[14]- (GPIO bank 2) 0=
	* gpio2, Output, gpio_2_pin_out[14]- (GPIO bank 2) 1= can0, Input, can0_ph
	* y_rx- (Can RX signal) 2= i2c0, Input, i2c0_scl_input- (SCL signal) 2= i2
	* c0, Output, i2c0_scl_out- (SCL signal) 3= swdt0, Input, swdt0_clk_in- (W
	* atch Dog Timer Input clock) 4= spi0, Output, spi0_n_ss_out[1]- (SPI Mast
	* er Selects) 5= ttc2, Input, ttc2_clk_in- (TTC Clock) 6= ua0, Input, ua0_
	* rxd- (UART receiver serial input) 7= trace, Output, tracedq[12]- (Trace
	* Port Databus)
	*  PSU_IOU_SLCR_MIO_PIN_66_L3_SEL                              0

	* Configures MIO Pin 66 peripheral interface mapping
	* (OFFSET, MASK, VALUE)      (0XFF180108, 0x000000FEU ,0x00000002U)
	*/
	PSU_Mask_Write( IOU_SLCR_MIO_PIN_66_OFFSET, 0x000000FEU, 0x00000002U );
	/*##################################################################### */

	/*
	* Register : MIO_PIN_67 @ 0XFF18010C

	* Level 0 Mux Select 0= Level 1 Mux Output 1= gem3, Output, gem3_rgmii_txd
	* [2]- (TX RGMII data)
	*  PSU_IOU_SLCR_MIO_PIN_67_L0_SEL                              1

	* Level 1 Mux Select 0= Level 2 Mux Output 1= usb1, Input, usb1_ulpi_nxt-
	* (Data flow control signal from the PHY)
	*  PSU_IOU_SLCR_MIO_PIN_67_L1_SEL                              0

	* Level 2 Mux Select 0= Level 3 Mux Output 1= sd0, Input, sd0_data_in[0]-
	* (8-bit Data bus) = sd0, Output, sdio0_data_out[0]- (8-bit Data bus) 2= N
	* ot Used 3= Not Used
	*  PSU_IOU_SLCR_MIO_PIN_67_L2_SEL                              0

	* Level 3 Mux Select 0= gpio2, Input, gpio_2_pin_in[15]- (GPIO bank 2) 0=
	* gpio2, Output, gpio_2_pin_out[15]- (GPIO bank 2) 1= can0, Output, can0_p
	* hy_tx- (Can TX signal) 2= i2c0, Input, i2c0_sda_input- (SDA signal) 2= i
	* 2c0, Output, i2c0_sda_out- (SDA signal) 3= swdt0, Output, swdt0_rst_out-
	*  (Watch Dog Timer Output clock) 4= spi0, Input, spi0_n_ss_in- (SPI Maste
	* r Selects) 4= spi0, Output, spi0_n_ss_out[0]- (SPI Master Selects) 5= tt
	* c2, Output, ttc2_wave_out- (TTC Waveform Clock) 6= ua0, Output, ua0_txd-
	*  (UART transmitter serial output) 7= trace, Output, tracedq[13]- (Trace
	* Port Databus)
	*  PSU_IOU_SLCR_MIO_PIN_67_L3_SEL                              0

	* Configures MIO Pin 67 peripheral interface mapping
	* (OFFSET, MASK, VALUE)      (0XFF18010C, 0x000000FEU ,0x00000002U)
	*/
	PSU_Mask_Write( IOU_SLCR_MIO_PIN_67_OFFSET, 0x000000FEU, 0x00000002U );
	/*##################################################################### */

	/*
	* Register : MIO_PIN_68 @ 0XFF180110

	* Level 0 Mux Select 0= Level 1 Mux Output 1= gem3, Output, gem3_rgmii_txd
	* [3]- (TX RGMII data)
	*  PSU_IOU_SLCR_MIO_PIN_68_L0_SEL                              1

	* Level 1 Mux Select 0= Level 2 Mux Output 1= usb1, Input, usb1_ulpi_rx_da
	* ta[0]- (ULPI data bus) 1= usb1, Output, usb1_ulpi_tx_data[0]- (ULPI data
	*  bus)
	*  PSU_IOU_SLCR_MIO_PIN_68_L1_SEL                              0

	* Level 2 Mux Select 0= Level 3 Mux Output 1= sd0, Input, sd0_data_in[1]-
	* (8-bit Data bus) = sd0, Output, sdio0_data_out[1]- (8-bit Data bus) 2= N
	* ot Used 3= Not Used
	*  PSU_IOU_SLCR_MIO_PIN_68_L2_SEL                              0

	* Level 3 Mux Select 0= gpio2, Input, gpio_2_pin_in[16]- (GPIO bank 2) 0=
	* gpio2, Output, gpio_2_pin_out[16]- (GPIO bank 2) 1= can1, Output, can1_p
	* hy_tx- (Can TX signal) 2= i2c1, Input, i2c1_scl_input- (SCL signal) 2= i
	* 2c1, Output, i2c1_scl_out- (SCL signal) 3= swdt1, Input, swdt1_clk_in- (
	* Watch Dog Timer Input clock) 4= spi0, Input, spi0_mi- (MISO signal) 4= s
	* pi0, Output, spi0_so- (MISO signal) 5= ttc1, Input, ttc1_clk_in- (TTC Cl
	* ock) 6= ua1, Output, ua1_txd- (UART transmitter serial output) 7= trace,
	*  Output, tracedq[14]- (Trace Port Databus)
	*  PSU_IOU_SLCR_MIO_PIN_68_L3_SEL                              0

	* Configures MIO Pin 68 peripheral interface mapping
	* (OFFSET, MASK, VALUE)      (0XFF180110, 0x000000FEU ,0x00000002U)
	*/
	PSU_Mask_Write( IOU_SLCR_MIO_PIN_68_OFFSET, 0x000000FEU, 0x00000002U );
	/*##################################################################### */

	/*
	* Register : MIO_PIN_69 @ 0XFF180114

	* Level 0 Mux Select 0= Level 1 Mux Output 1= gem3, Output, gem3_rgmii_tx_
	* ctl- (TX RGMII control)
	*  PSU_IOU_SLCR_MIO_PIN_69_L0_SEL                              1

	* Level 1 Mux Select 0= Level 2 Mux Output 1= usb1, Input, usb1_ulpi_rx_da
	* ta[1]- (ULPI data bus) 1= usb1, Output, usb1_ulpi_tx_data[1]- (ULPI data
	*  bus)
	*  PSU_IOU_SLCR_MIO_PIN_69_L1_SEL                              0

	* Level 2 Mux Select 0= Level 3 Mux Output 1= sd0, Input, sd0_data_in[2]-
	* (8-bit Data bus) = sd0, Output, sdio0_data_out[2]- (8-bit Data bus) 2= s
	* d1, Input, sdio1_wp- (SD card write protect from connector) 3= Not Used
	*  PSU_IOU_SLCR_MIO_PIN_69_L2_SEL                              0

	* Level 3 Mux Select 0= gpio2, Input, gpio_2_pin_in[17]- (GPIO bank 2) 0=
	* gpio2, Output, gpio_2_pin_out[17]- (GPIO bank 2) 1= can1, Input, can1_ph
	* y_rx- (Can RX signal) 2= i2c1, Input, i2c1_sda_input- (SDA signal) 2= i2
	* c1, Output, i2c1_sda_out- (SDA signal) 3= swdt1, Output, swdt1_rst_out-
	* (Watch Dog Timer Output clock) 4= spi0, Output, spi0_mo- (MOSI signal) 4
	* = spi0, Input, spi0_si- (MOSI signal) 5= ttc1, Output, ttc1_wave_out- (T
	* TC Waveform Clock) 6= ua1, Input, ua1_rxd- (UART receiver serial input)
	* 7= trace, Output, tracedq[15]- (Trace Port Databus)
	*  PSU_IOU_SLCR_MIO_PIN_69_L3_SEL                              0

	* Configures MIO Pin 69 peripheral interface mapping
	* (OFFSET, MASK, VALUE)      (0XFF180114, 0x000000FEU ,0x00000002U)
	*/
	PSU_Mask_Write( IOU_SLCR_MIO_PIN_69_OFFSET, 0x000000FEU, 0x00000002U );
	/*##################################################################### */

	/*
	* Register : MIO_PIN_70 @ 0XFF180118

	* Level 0 Mux Select 0= Level 1 Mux Output 1= gem3, Input, gem3_rgmii_rx_c
	* lk- (RX RGMII clock)
	*  PSU_IOU_SLCR_MIO_PIN_70_L0_SEL                              1

	* Level 1 Mux Select 0= Level 2 Mux Output 1= usb1, Output, usb1_ulpi_stp-
	*  (Asserted to end or interrupt transfers)
	*  PSU_IOU_SLCR_MIO_PIN_70_L1_SEL                              0

	* Level 2 Mux Select 0= Level 3 Mux Output 1= sd0, Input, sd0_data_in[3]-
	* (8-bit Data bus) = sd0, Output, sdio0_data_out[3]- (8-bit Data bus) 2= s
	* d1, Output, sdio1_bus_pow- (SD card bus power) 3= Not Used
	*  PSU_IOU_SLCR_MIO_PIN_70_L2_SEL                              0

	* Level 3 Mux Select 0= gpio2, Input, gpio_2_pin_in[18]- (GPIO bank 2) 0=
	* gpio2, Output, gpio_2_pin_out[18]- (GPIO bank 2) 1= can0, Input, can0_ph
	* y_rx- (Can RX signal) 2= i2c0, Input, i2c0_scl_input- (SCL signal) 2= i2
	* c0, Output, i2c0_scl_out- (SCL signal) 3= swdt0, Input, swdt0_clk_in- (W
	* atch Dog Timer Input clock) 4= spi1, Input, spi1_sclk_in- (SPI Clock) 4=
	*  spi1, Output, spi1_sclk_out- (SPI Clock) 5= ttc0, Input, ttc0_clk_in- (
	* TTC Clock) 6= ua0, Input, ua0_rxd- (UART receiver serial input) 7= Not U
	* sed
	*  PSU_IOU_SLCR_MIO_PIN_70_L3_SEL                              0

	* Configures MIO Pin 70 peripheral interface mapping
	* (OFFSET, MASK, VALUE)      (0XFF180118, 0x000000FEU ,0x00000002U)
	*/
	PSU_Mask_Write( IOU_SLCR_MIO_PIN_70_OFFSET, 0x000000FEU, 0x00000002U );
	/*##################################################################### */

	/*
	* Register : MIO_PIN_71 @ 0XFF18011C

	* Level 0 Mux Select 0= Level 1 Mux Output 1= gem3, Input, gem3_rgmii_rxd[
	* 0]- (RX RGMII data)
	*  PSU_IOU_SLCR_MIO_PIN_71_L0_SEL                              1

	* Level 1 Mux Select 0= Level 2 Mux Output 1= usb1, Input, usb1_ulpi_rx_da
	* ta[3]- (ULPI data bus) 1= usb1, Output, usb1_ulpi_tx_data[3]- (ULPI data
	*  bus)
	*  PSU_IOU_SLCR_MIO_PIN_71_L1_SEL                              0

	* Level 2 Mux Select 0= Level 3 Mux Output 1= sd0, Input, sd0_data_in[4]-
	* (8-bit Data bus) = sd0, Output, sdio0_data_out[4]- (8-bit Data bus) 2= s
	* d1, Input, sd1_data_in[0]- (8-bit Data bus) = sd1, Output, sdio1_data_ou
	* t[0]- (8-bit Data bus) 3= Not Used
	*  PSU_IOU_SLCR_MIO_PIN_71_L2_SEL                              0

	* Level 3 Mux Select 0= gpio2, Input, gpio_2_pin_in[19]- (GPIO bank 2) 0=
	* gpio2, Output, gpio_2_pin_out[19]- (GPIO bank 2) 1= can0, Output, can0_p
	* hy_tx- (Can TX signal) 2= i2c0, Input, i2c0_sda_input- (SDA signal) 2= i
	* 2c0, Output, i2c0_sda_out- (SDA signal) 3= swdt0, Output, swdt0_rst_out-
	*  (Watch Dog Timer Output clock) 4= spi1, Output, spi1_n_ss_out[2]- (SPI
	* Master Selects) 5= ttc0, Output, ttc0_wave_out- (TTC Waveform Clock) 6=
	* ua0, Output, ua0_txd- (UART transmitter serial output) 7= Not Used
	*  PSU_IOU_SLCR_MIO_PIN_71_L3_SEL                              0

	* Configures MIO Pin 71 peripheral interface mapping
	* (OFFSET, MASK, VALUE)      (0XFF18011C, 0x000000FEU ,0x00000002U)
	*/
	PSU_Mask_Write( IOU_SLCR_MIO_PIN_71_OFFSET, 0x000000FEU, 0x00000002U );
	/*##################################################################### */

	/*
	* Register : MIO_PIN_72 @ 0XFF180120

	* Level 0 Mux Select 0= Level 1 Mux Output 1= gem3, Input, gem3_rgmii_rxd[
	* 1]- (RX RGMII data)
	*  PSU_IOU_SLCR_MIO_PIN_72_L0_SEL                              1

	* Level 1 Mux Select 0= Level 2 Mux Output 1= usb1, Input, usb1_ulpi_rx_da
	* ta[4]- (ULPI data bus) 1= usb1, Output, usb1_ulpi_tx_data[4]- (ULPI data
	*  bus)
	*  PSU_IOU_SLCR_MIO_PIN_72_L1_SEL                              0

	* Level 2 Mux Select 0= Level 3 Mux Output 1= sd0, Input, sd0_data_in[5]-
	* (8-bit Data bus) = sd0, Output, sdio0_data_out[5]- (8-bit Data bus) 2= s
	* d1, Input, sd1_data_in[1]- (8-bit Data bus) = sd1, Output, sdio1_data_ou
	* t[1]- (8-bit Data bus) 3= Not Used
	*  PSU_IOU_SLCR_MIO_PIN_72_L2_SEL                              0

	* Level 3 Mux Select 0= gpio2, Input, gpio_2_pin_in[20]- (GPIO bank 2) 0=
	* gpio2, Output, gpio_2_pin_out[20]- (GPIO bank 2) 1= can1, Output, can1_p
	* hy_tx- (Can TX signal) 2= i2c1, Input, i2c1_scl_input- (SCL signal) 2= i
	* 2c1, Output, i2c1_scl_out- (SCL signal) 3= swdt1, Input, swdt1_clk_in- (
	* Watch Dog Timer Input clock) 4= spi1, Output, spi1_n_ss_out[1]- (SPI Mas
	* ter Selects) 5= Not Used 6= ua1, Output, ua1_txd- (UART transmitter seri
	* al output) 7= Not Used
	*  PSU_IOU_SLCR_MIO_PIN_72_L3_SEL                              0

	* Configures MIO Pin 72 peripheral interface mapping
	* (OFFSET, MASK, VALUE)      (0XFF180120, 0x000000FEU ,0x00000002U)
	*/
	PSU_Mask_Write( IOU_SLCR_MIO_PIN_72_OFFSET, 0x000000FEU, 0x00000002U );
	/*##################################################################### */

	/*
	* Register : MIO_PIN_73 @ 0XFF180124

	* Level 0 Mux Select 0= Level 1 Mux Output 1= gem3, Input, gem3_rgmii_rxd[
	* 2]- (RX RGMII data)
	*  PSU_IOU_SLCR_MIO_PIN_73_L0_SEL                              1

	* Level 1 Mux Select 0= Level 2 Mux Output 1= usb1, Input, usb1_ulpi_rx_da
	* ta[5]- (ULPI data bus) 1= usb1, Output, usb1_ulpi_tx_data[5]- (ULPI data
	*  bus)
	*  PSU_IOU_SLCR_MIO_PIN_73_L1_SEL                              0

	* Level 2 Mux Select 0= Level 3 Mux Output 1= sd0, Input, sd0_data_in[6]-
	* (8-bit Data bus) = sd0, Output, sdio0_data_out[6]- (8-bit Data bus) 2= s
	* d1, Input, sd1_data_in[2]- (8-bit Data bus) = sd1, Output, sdio1_data_ou
	* t[2]- (8-bit Data bus) 3= Not Used
	*  PSU_IOU_SLCR_MIO_PIN_73_L2_SEL                              0

	* Level 3 Mux Select 0= gpio2, Input, gpio_2_pin_in[21]- (GPIO bank 2) 0=
	* gpio2, Output, gpio_2_pin_out[21]- (GPIO bank 2) 1= can1, Input, can1_ph
	* y_rx- (Can RX signal) 2= i2c1, Input, i2c1_sda_input- (SDA signal) 2= i2
	* c1, Output, i2c1_sda_out- (SDA signal) 3= swdt1, Output, swdt1_rst_out-
	* (Watch Dog Timer Output clock) 4= spi1, Input, spi1_n_ss_in- (SPI Master
	*  Selects) 4= spi1, Output, spi1_n_ss_out[0]- (SPI Master Selects) 5= Not
	*  Used 6= ua1, Input, ua1_rxd- (UART receiver serial input) 7= Not Used
	*  PSU_IOU_SLCR_MIO_PIN_73_L3_SEL                              0

	* Configures MIO Pin 73 peripheral interface mapping
	* (OFFSET, MASK, VALUE)      (0XFF180124, 0x000000FEU ,0x00000002U)
	*/
	PSU_Mask_Write( IOU_SLCR_MIO_PIN_73_OFFSET, 0x000000FEU, 0x00000002U );
	/*##################################################################### */

	/*
	* Register : MIO_PIN_74 @ 0XFF180128

	* Level 0 Mux Select 0= Level 1 Mux Output 1= gem3, Input, gem3_rgmii_rxd[
	* 3]- (RX RGMII data)
	*  PSU_IOU_SLCR_MIO_PIN_74_L0_SEL                              1

	* Level 1 Mux Select 0= Level 2 Mux Output 1= usb1, Input, usb1_ulpi_rx_da
	* ta[6]- (ULPI data bus) 1= usb1, Output, usb1_ulpi_tx_data[6]- (ULPI data
	*  bus)
	*  PSU_IOU_SLCR_MIO_PIN_74_L1_SEL                              0

	* Level 2 Mux Select 0= Level 3 Mux Output 1= sd0, Input, sd0_data_in[7]-
	* (8-bit Data bus) = sd0, Output, sdio0_data_out[7]- (8-bit Data bus) 2= s
	* d1, Input, sd1_data_in[3]- (8-bit Data bus) = sd1, Output, sdio1_data_ou
	* t[3]- (8-bit Data bus) 3= Not Used
	*  PSU_IOU_SLCR_MIO_PIN_74_L2_SEL                              0

	* Level 3 Mux Select 0= gpio2, Input, gpio_2_pin_in[22]- (GPIO bank 2) 0=
	* gpio2, Output, gpio_2_pin_out[22]- (GPIO bank 2) 1= can0, Input, can0_ph
	* y_rx- (Can RX signal) 2= i2c0, Input, i2c0_scl_input- (SCL signal) 2= i2
	* c0, Output, i2c0_scl_out- (SCL signal) 3= swdt0, Input, swdt0_clk_in- (W
	* atch Dog Timer Input clock) 4= spi1, Input, spi1_mi- (MISO signal) 4= sp
	* i1, Output, spi1_so- (MISO signal) 5= Not Used 6= ua0, Input, ua0_rxd- (
	* UART receiver serial input) 7= Not Used
	*  PSU_IOU_SLCR_MIO_PIN_74_L3_SEL                              0

	* Configures MIO Pin 74 peripheral interface mapping
	* (OFFSET, MASK, VALUE)      (0XFF180128, 0x000000FEU ,0x00000002U)
	*/
	PSU_Mask_Write( IOU_SLCR_MIO_PIN_74_OFFSET, 0x000000FEU, 0x00000002U );
	/*##################################################################### */

	/*
	* Register : MIO_PIN_75 @ 0XFF18012C

	* Level 0 Mux Select 0= Level 1 Mux Output 1= gem3, Input, gem3_rgmii_rx_c
	* tl- (RX RGMII control )
	*  PSU_IOU_SLCR_MIO_PIN_75_L0_SEL                              1

	* Level 1 Mux Select 0= Level 2 Mux Output 1= usb1, Input, usb1_ulpi_rx_da
	* ta[7]- (ULPI data bus) 1= usb1, Output, usb1_ulpi_tx_data[7]- (ULPI data
	*  bus)
	*  PSU_IOU_SLCR_MIO_PIN_75_L1_SEL                              0

	* Level 2 Mux Select 0= Level 3 Mux Output 1= sd0, Output, sdio0_bus_pow-
	* (SD card bus power) 2= sd1, Input, sd1_cmd_in- (Command Indicator) = sd1
	* , Output, sdio1_cmd_out- (Command Indicator) 3= Not Used
	*  PSU_IOU_SLCR_MIO_PIN_75_L2_SEL                              0

	* Level 3 Mux Select 0= gpio2, Input, gpio_2_pin_in[23]- (GPIO bank 2) 0=
	* gpio2, Output, gpio_2_pin_out[23]- (GPIO bank 2) 1= can0, Output, can0_p
	* hy_tx- (Can TX signal) 2= i2c0, Input, i2c0_sda_input- (SDA signal) 2= i
	* 2c0, Output, i2c0_sda_out- (SDA signal) 3= swdt0, Output, swdt0_rst_out-
	*  (Watch Dog Timer Output clock) 4= spi1, Output, spi1_mo- (MOSI signal)
	* 4= spi1, Input, spi1_si- (MOSI signal) 5= Not Used 6= ua0, Output, ua0_t
	* xd- (UART transmitter serial output) 7= Not Used
	*  PSU_IOU_SLCR_MIO_PIN_75_L3_SEL                              0

	* Configures MIO Pin 75 peripheral interface mapping
	* (OFFSET, MASK, VALUE)      (0XFF18012C, 0x000000FEU ,0x00000002U)
	*/
	PSU_Mask_Write( IOU_SLCR_MIO_PIN_75_OFFSET, 0x000000FEU, 0x00000002U );
	/*##################################################################### */

	/*
	* Register : MIO_PIN_76 @ 0XFF180130

	* Level 0 Mux Select 0= Level 1 Mux Output 1= Not Used
	*  PSU_IOU_SLCR_MIO_PIN_76_L0_SEL                              0

	* Level 1 Mux Select 0= Level 2 Mux Output 1= Not Used
	*  PSU_IOU_SLCR_MIO_PIN_76_L1_SEL                              0

	* Level 2 Mux Select 0= Level 3 Mux Output 1= sd0, Input, sdio0_wp- (SD ca
	* rd write protect from connector) 2= sd1, Output, sdio1_clk_out- (SDSDIO
	* clock) 3= Not Used
	*  PSU_IOU_SLCR_MIO_PIN_76_L2_SEL                              0

	* Level 3 Mux Select 0= gpio2, Input, gpio_2_pin_in[24]- (GPIO bank 2) 0=
	* gpio2, Output, gpio_2_pin_out[24]- (GPIO bank 2) 1= can1, Output, can1_p
	* hy_tx- (Can TX signal) 2= i2c1, Input, i2c1_scl_input- (SCL signal) 2= i
	* 2c1, Output, i2c1_scl_out- (SCL signal) 3= mdio0, Output, gem0_mdc- (MDI
	* O Clock) 4= mdio1, Output, gem1_mdc- (MDIO Clock) 5= mdio2, Output, gem2
	* _mdc- (MDIO Clock) 6= mdio3, Output, gem3_mdc- (MDIO Clock) 7= Not Used
	*  PSU_IOU_SLCR_MIO_PIN_76_L3_SEL                              6

	* Configures MIO Pin 76 peripheral interface mapping
	* (OFFSET, MASK, VALUE)      (0XFF180130, 0x000000FEU ,0x000000C0U)
	*/
	PSU_Mask_Write( IOU_SLCR_MIO_PIN_76_OFFSET, 0x000000FEU, 0x000000C0U );
	/*##################################################################### */

	/*
	* Register : MIO_PIN_77 @ 0XFF180134

	* Level 0 Mux Select 0= Level 1 Mux Output 1= Not Used
	*  PSU_IOU_SLCR_MIO_PIN_77_L0_SEL                              0

	* Level 1 Mux Select 0= Level 2 Mux Output 1= Not Used
	*  PSU_IOU_SLCR_MIO_PIN_77_L1_SEL                              0

	* Level 2 Mux Select 0= Level 3 Mux Output 1= Not Used 2= sd1, Input, sdio
	* 1_cd_n- (SD card detect from connector) 3= Not Used
	*  PSU_IOU_SLCR_MIO_PIN_77_L2_SEL                              0

	* Level 3 Mux Select 0= gpio2, Input, gpio_2_pin_in[25]- (GPIO bank 2) 0=
	* gpio2, Output, gpio_2_pin_out[25]- (GPIO bank 2) 1= can1, Input, can1_ph
	* y_rx- (Can RX signal) 2= i2c1, Input, i2c1_sda_input- (SDA signal) 2= i2
	* c1, Output, i2c1_sda_out- (SDA signal) 3= mdio0, Input, gem0_mdio_in- (M
	* DIO Data) 3= mdio0, Output, gem0_mdio_out- (MDIO Data) 4= mdio1, Input,
	* gem1_mdio_in- (MDIO Data) 4= mdio1, Output, gem1_mdio_out- (MDIO Data) 5
	* = mdio2, Input, gem2_mdio_in- (MDIO Data) 5= mdio2, Output, gem2_mdio_ou
	* t- (MDIO Data) 6= mdio3, Input, gem3_mdio_in- (MDIO Data) 6= mdio3, Outp
	* ut, gem3_mdio_out- (MDIO Data) 7= Not Used
	*  PSU_IOU_SLCR_MIO_PIN_77_L3_SEL                              6

	* Configures MIO Pin 77 peripheral interface mapping
	* (OFFSET, MASK, VALUE)      (0XFF180134, 0x000000FEU ,0x000000C0U)
	*/
	PSU_Mask_Write( IOU_SLCR_MIO_PIN_77_OFFSET, 0x000000FEU, 0x000000C0U );
	/*##################################################################### */

	/*
	* Register : MIO_MST_TRI0 @ 0XFF180204

	* Master Tri-state Enable for pin 0, active high
	*  PSU_IOU_SLCR_MIO_MST_TRI0_PIN_00_TRI                        0

	* Master Tri-state Enable for pin 1, active high
	*  PSU_IOU_SLCR_MIO_MST_TRI0_PIN_01_TRI                        0

	* Master Tri-state Enable for pin 2, active high
	*  PSU_IOU_SLCR_MIO_MST_TRI0_PIN_02_TRI                        0

	* Master Tri-state Enable for pin 3, active high
	*  PSU_IOU_SLCR_MIO_MST_TRI0_PIN_03_TRI                        0

	* Master Tri-state Enable for pin 4, active high
	*  PSU_IOU_SLCR_MIO_MST_TRI0_PIN_04_TRI                        0

	* Master Tri-state Enable for pin 5, active high
	*  PSU_IOU_SLCR_MIO_MST_TRI0_PIN_05_TRI                        0

	* Master Tri-state Enable for pin 6, active high
	*  PSU_IOU_SLCR_MIO_MST_TRI0_PIN_06_TRI                        0

	* Master Tri-state Enable for pin 7, active high
	*  PSU_IOU_SLCR_MIO_MST_TRI0_PIN_07_TRI                        0

	* Master Tri-state Enable for pin 8, active high
	*  PSU_IOU_SLCR_MIO_MST_TRI0_PIN_08_TRI                        0

	* Master Tri-state Enable for pin 9, active high
	*  PSU_IOU_SLCR_MIO_MST_TRI0_PIN_09_TRI                        0

	* Master Tri-state Enable for pin 10, active high
	*  PSU_IOU_SLCR_MIO_MST_TRI0_PIN_10_TRI                        0

	* Master Tri-state Enable for pin 11, active high
	*  PSU_IOU_SLCR_MIO_MST_TRI0_PIN_11_TRI                        0

	* Master Tri-state Enable for pin 12, active high
	*  PSU_IOU_SLCR_MIO_MST_TRI0_PIN_12_TRI                        0

	* Master Tri-state Enable for pin 13, active high
	*  PSU_IOU_SLCR_MIO_MST_TRI0_PIN_13_TRI                        0

	* Master Tri-state Enable for pin 14, active high
	*  PSU_IOU_SLCR_MIO_MST_TRI0_PIN_14_TRI                        0

	* Master Tri-state Enable for pin 15, active high
	*  PSU_IOU_SLCR_MIO_MST_TRI0_PIN_15_TRI                        0

	* Master Tri-state Enable for pin 16, active high
	*  PSU_IOU_SLCR_MIO_MST_TRI0_PIN_16_TRI                        0

	* Master Tri-state Enable for pin 17, active high
	*  PSU_IOU_SLCR_MIO_MST_TRI0_PIN_17_TRI                        0

	* Master Tri-state Enable for pin 18, active high
	*  PSU_IOU_SLCR_MIO_MST_TRI0_PIN_18_TRI                        0

	* Master Tri-state Enable for pin 19, active high
	*  PSU_IOU_SLCR_MIO_MST_TRI0_PIN_19_TRI                        0

	* Master Tri-state Enable for pin 20, active high
	*  PSU_IOU_SLCR_MIO_MST_TRI0_PIN_20_TRI                        0

	* Master Tri-state Enable for pin 21, active high
	*  PSU_IOU_SLCR_MIO_MST_TRI0_PIN_21_TRI                        0

	* Master Tri-state Enable for pin 22, active high
	*  PSU_IOU_SLCR_MIO_MST_TRI0_PIN_22_TRI                        0

	* Master Tri-state Enable for pin 23, active high
	*  PSU_IOU_SLCR_MIO_MST_TRI0_PIN_23_TRI                        0

	* Master Tri-state Enable for pin 24, active high
	*  PSU_IOU_SLCR_MIO_MST_TRI0_PIN_24_TRI                        0

	* Master Tri-state Enable for pin 25, active high
	*  PSU_IOU_SLCR_MIO_MST_TRI0_PIN_25_TRI                        0

	* Master Tri-state Enable for pin 26, active high
	*  PSU_IOU_SLCR_MIO_MST_TRI0_PIN_26_TRI                        1

	* Master Tri-state Enable for pin 27, active high
	*  PSU_IOU_SLCR_MIO_MST_TRI0_PIN_27_TRI                        0

	* Master Tri-state Enable for pin 28, active high
	*  PSU_IOU_SLCR_MIO_MST_TRI0_PIN_28_TRI                        1

	* Master Tri-state Enable for pin 29, active high
	*  PSU_IOU_SLCR_MIO_MST_TRI0_PIN_29_TRI                        0

	* Master Tri-state Enable for pin 30, active high
	*  PSU_IOU_SLCR_MIO_MST_TRI0_PIN_30_TRI                        1

	* Master Tri-state Enable for pin 31, active high
	*  PSU_IOU_SLCR_MIO_MST_TRI0_PIN_31_TRI                        1

	* MIO pin Tri-state Enables, 31:0
	* (OFFSET, MASK, VALUE)      (0XFF180204, 0xFFFFFFFFU ,0xD4000000U)
	*/
	PSU_Mask_Write( IOU_SLCR_MIO_MST_TRI0_OFFSET,
	                0xFFFFFFFFU, 0xD4000000U );
	/*##################################################################### */

	/*
	* Register : MIO_MST_TRI1 @ 0XFF180208

	* Master Tri-state Enable for pin 32, active high
	*  PSU_IOU_SLCR_MIO_MST_TRI1_PIN_32_TRI                        0

	* Master Tri-state Enable for pin 33, active high
	*  PSU_IOU_SLCR_MIO_MST_TRI1_PIN_33_TRI                        0

	* Master Tri-state Enable for pin 34, active high
	*  PSU_IOU_SLCR_MIO_MST_TRI1_PIN_34_TRI                        0

	* Master Tri-state Enable for pin 35, active high
	*  PSU_IOU_SLCR_MIO_MST_TRI1_PIN_35_TRI                        0

	* Master Tri-state Enable for pin 36, active high
	*  PSU_IOU_SLCR_MIO_MST_TRI1_PIN_36_TRI                        0

	* Master Tri-state Enable for pin 37, active high
	*  PSU_IOU_SLCR_MIO_MST_TRI1_PIN_37_TRI                        1

	* Master Tri-state Enable for pin 38, active high
	*  PSU_IOU_SLCR_MIO_MST_TRI1_PIN_38_TRI                        0

	* Master Tri-state Enable for pin 39, active high
	*  PSU_IOU_SLCR_MIO_MST_TRI1_PIN_39_TRI                        0

	* Master Tri-state Enable for pin 40, active high
	*  PSU_IOU_SLCR_MIO_MST_TRI1_PIN_40_TRI                        0

	* Master Tri-state Enable for pin 41, active high
	*  PSU_IOU_SLCR_MIO_MST_TRI1_PIN_41_TRI                        0

	* Master Tri-state Enable for pin 42, active high
	*  PSU_IOU_SLCR_MIO_MST_TRI1_PIN_42_TRI                        0

	* Master Tri-state Enable for pin 43, active high
	*  PSU_IOU_SLCR_MIO_MST_TRI1_PIN_43_TRI                        0

	* Master Tri-state Enable for pin 44, active high
	*  PSU_IOU_SLCR_MIO_MST_TRI1_PIN_44_TRI                        0

	* Master Tri-state Enable for pin 45, active high
	*  PSU_IOU_SLCR_MIO_MST_TRI1_PIN_45_TRI                        1

	* Master Tri-state Enable for pin 46, active high
	*  PSU_IOU_SLCR_MIO_MST_TRI1_PIN_46_TRI                        0

	* Master Tri-state Enable for pin 47, active high
	*  PSU_IOU_SLCR_MIO_MST_TRI1_PIN_47_TRI                        0

	* Master Tri-state Enable for pin 48, active high
	*  PSU_IOU_SLCR_MIO_MST_TRI1_PIN_48_TRI                        0

	* Master Tri-state Enable for pin 49, active high
	*  PSU_IOU_SLCR_MIO_MST_TRI1_PIN_49_TRI                        0

	* Master Tri-state Enable for pin 50, active high
	*  PSU_IOU_SLCR_MIO_MST_TRI1_PIN_50_TRI                        0

	* Master Tri-state Enable for pin 51, active high
	*  PSU_IOU_SLCR_MIO_MST_TRI1_PIN_51_TRI                        0

	* Master Tri-state Enable for pin 52, active high
	*  PSU_IOU_SLCR_MIO_MST_TRI1_PIN_52_TRI                        1

	* Master Tri-state Enable for pin 53, active high
	*  PSU_IOU_SLCR_MIO_MST_TRI1_PIN_53_TRI                        1

	* Master Tri-state Enable for pin 54, active high
	*  PSU_IOU_SLCR_MIO_MST_TRI1_PIN_54_TRI                        0

	* Master Tri-state Enable for pin 55, active high
	*  PSU_IOU_SLCR_MIO_MST_TRI1_PIN_55_TRI                        1

	* Master Tri-state Enable for pin 56, active high
	*  PSU_IOU_SLCR_MIO_MST_TRI1_PIN_56_TRI                        0

	* Master Tri-state Enable for pin 57, active high
	*  PSU_IOU_SLCR_MIO_MST_TRI1_PIN_57_TRI                        0

	* Master Tri-state Enable for pin 58, active high
	*  PSU_IOU_SLCR_MIO_MST_TRI1_PIN_58_TRI                        0

	* Master Tri-state Enable for pin 59, active high
	*  PSU_IOU_SLCR_MIO_MST_TRI1_PIN_59_TRI                        0

	* Master Tri-state Enable for pin 60, active high
	*  PSU_IOU_SLCR_MIO_MST_TRI1_PIN_60_TRI                        0

	* Master Tri-state Enable for pin 61, active high
	*  PSU_IOU_SLCR_MIO_MST_TRI1_PIN_61_TRI                        0

	* Master Tri-state Enable for pin 62, active high
	*  PSU_IOU_SLCR_MIO_MST_TRI1_PIN_62_TRI                        0

	* Master Tri-state Enable for pin 63, active high
	*  PSU_IOU_SLCR_MIO_MST_TRI1_PIN_63_TRI                        0

	* MIO pin Tri-state Enables, 63:32
	* (OFFSET, MASK, VALUE)      (0XFF180208, 0xFFFFFFFFU ,0x00B02020U)
	*/
	PSU_Mask_Write( IOU_SLCR_MIO_MST_TRI1_OFFSET,
	                0xFFFFFFFFU, 0x00B02020U );
	/*##################################################################### */

	/*
	* Register : MIO_MST_TRI2 @ 0XFF18020C

	* Master Tri-state Enable for pin 64, active high
	*  PSU_IOU_SLCR_MIO_MST_TRI2_PIN_64_TRI                        0

	* Master Tri-state Enable for pin 65, active high
	*  PSU_IOU_SLCR_MIO_MST_TRI2_PIN_65_TRI                        0

	* Master Tri-state Enable for pin 66, active high
	*  PSU_IOU_SLCR_MIO_MST_TRI2_PIN_66_TRI                        0

	* Master Tri-state Enable for pin 67, active high
	*  PSU_IOU_SLCR_MIO_MST_TRI2_PIN_67_TRI                        0

	* Master Tri-state Enable for pin 68, active high
	*  PSU_IOU_SLCR_MIO_MST_TRI2_PIN_68_TRI                        0

	* Master Tri-state Enable for pin 69, active high
	*  PSU_IOU_SLCR_MIO_MST_TRI2_PIN_69_TRI                        0

	* Master Tri-state Enable for pin 70, active high
	*  PSU_IOU_SLCR_MIO_MST_TRI2_PIN_70_TRI                        1

	* Master Tri-state Enable for pin 71, active high
	*  PSU_IOU_SLCR_MIO_MST_TRI2_PIN_71_TRI                        1

	* Master Tri-state Enable for pin 72, active high
	*  PSU_IOU_SLCR_MIO_MST_TRI2_PIN_72_TRI                        1

	* Master Tri-state Enable for pin 73, active high
	*  PSU_IOU_SLCR_MIO_MST_TRI2_PIN_73_TRI                        1

	* Master Tri-state Enable for pin 74, active high
	*  PSU_IOU_SLCR_MIO_MST_TRI2_PIN_74_TRI                        1

	* Master Tri-state Enable for pin 75, active high
	*  PSU_IOU_SLCR_MIO_MST_TRI2_PIN_75_TRI                        1

	* Master Tri-state Enable for pin 76, active high
	*  PSU_IOU_SLCR_MIO_MST_TRI2_PIN_76_TRI                        0

	* Master Tri-state Enable for pin 77, active high
	*  PSU_IOU_SLCR_MIO_MST_TRI2_PIN_77_TRI                        0

	* MIO pin Tri-state Enables, 77:64
	* (OFFSET, MASK, VALUE)      (0XFF18020C, 0x00003FFFU ,0x00000FC0U)
	*/
	PSU_Mask_Write( IOU_SLCR_MIO_MST_TRI2_OFFSET,
	                0x00003FFFU, 0x00000FC0U );
	/*##################################################################### */

	/*
	* Register : bank0_ctrl0 @ 0XFF180138

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL0_DRIVE0_BIT_0                       0

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL0_DRIVE0_BIT_1                       0

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL0_DRIVE0_BIT_2                       0

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL0_DRIVE0_BIT_3                       0

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL0_DRIVE0_BIT_4                       0

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL0_DRIVE0_BIT_5                       0

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL0_DRIVE0_BIT_6                       0

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL0_DRIVE0_BIT_7                       0

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL0_DRIVE0_BIT_8                       0

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL0_DRIVE0_BIT_9                       0

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL0_DRIVE0_BIT_10                      0

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL0_DRIVE0_BIT_11                      0

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL0_DRIVE0_BIT_12                      0

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL0_DRIVE0_BIT_13                      0

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL0_DRIVE0_BIT_14                      0

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL0_DRIVE0_BIT_15                      0

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL0_DRIVE0_BIT_16                      0

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL0_DRIVE0_BIT_17                      0

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL0_DRIVE0_BIT_18                      0

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL0_DRIVE0_BIT_19                      0

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL0_DRIVE0_BIT_20                      0

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL0_DRIVE0_BIT_21                      0

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL0_DRIVE0_BIT_22                      0

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL0_DRIVE0_BIT_23                      0

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL0_DRIVE0_BIT_24                      0

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL0_DRIVE0_BIT_25                      0

	* Drive0 control to MIO Bank 0 - control MIO[25:0]
	* (OFFSET, MASK, VALUE)      (0XFF180138, 0x03FFFFFFU ,0x00000000U)
	*/
	PSU_Mask_Write( IOU_SLCR_BANK0_CTRL0_OFFSET,
	                0x03FFFFFFU, 0x00000000U );
	/*##################################################################### */

	/*
	* Register : bank0_ctrl1 @ 0XFF18013C

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL1_DRIVE1_BIT_0                       1

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL1_DRIVE1_BIT_1                       1

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL1_DRIVE1_BIT_2                       1

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL1_DRIVE1_BIT_3                       1

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL1_DRIVE1_BIT_4                       1

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL1_DRIVE1_BIT_5                       1

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL1_DRIVE1_BIT_6                       1

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL1_DRIVE1_BIT_7                       1

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL1_DRIVE1_BIT_8                       1

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL1_DRIVE1_BIT_9                       1

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL1_DRIVE1_BIT_10                      1

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL1_DRIVE1_BIT_11                      1

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL1_DRIVE1_BIT_12                      1

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL1_DRIVE1_BIT_13                      1

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL1_DRIVE1_BIT_14                      1

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL1_DRIVE1_BIT_15                      1

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL1_DRIVE1_BIT_16                      1

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL1_DRIVE1_BIT_17                      1

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL1_DRIVE1_BIT_18                      1

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL1_DRIVE1_BIT_19                      1

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL1_DRIVE1_BIT_20                      1

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL1_DRIVE1_BIT_21                      1

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL1_DRIVE1_BIT_22                      1

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL1_DRIVE1_BIT_23                      1

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL1_DRIVE1_BIT_24                      1

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL1_DRIVE1_BIT_25                      1

	* Drive1 control to MIO Bank 0 - control MIO[25:0]
	* (OFFSET, MASK, VALUE)      (0XFF18013C, 0x03FFFFFFU ,0x03FFFFFFU)
	*/
	PSU_Mask_Write( IOU_SLCR_BANK0_CTRL1_OFFSET,
	                0x03FFFFFFU, 0x03FFFFFFU );
	/*##################################################################### */

	/*
	* Register : bank0_ctrl3 @ 0XFF180140

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL3_SCHMITT_CMOS_N_BIT_0               0

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL3_SCHMITT_CMOS_N_BIT_1               0

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL3_SCHMITT_CMOS_N_BIT_2               0

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL3_SCHMITT_CMOS_N_BIT_3               0

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL3_SCHMITT_CMOS_N_BIT_4               0

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL3_SCHMITT_CMOS_N_BIT_5               0

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL3_SCHMITT_CMOS_N_BIT_6               0

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL3_SCHMITT_CMOS_N_BIT_7               0

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL3_SCHMITT_CMOS_N_BIT_8               0

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL3_SCHMITT_CMOS_N_BIT_9               0

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL3_SCHMITT_CMOS_N_BIT_10              0

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL3_SCHMITT_CMOS_N_BIT_11              0

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL3_SCHMITT_CMOS_N_BIT_12              0

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL3_SCHMITT_CMOS_N_BIT_13              0

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL3_SCHMITT_CMOS_N_BIT_14              0

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL3_SCHMITT_CMOS_N_BIT_15              0

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL3_SCHMITT_CMOS_N_BIT_16              0

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL3_SCHMITT_CMOS_N_BIT_17              0

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL3_SCHMITT_CMOS_N_BIT_18              0

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL3_SCHMITT_CMOS_N_BIT_19              0

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL3_SCHMITT_CMOS_N_BIT_20              0

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL3_SCHMITT_CMOS_N_BIT_21              0

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL3_SCHMITT_CMOS_N_BIT_22              0

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL3_SCHMITT_CMOS_N_BIT_23              0

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL3_SCHMITT_CMOS_N_BIT_24              0

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL3_SCHMITT_CMOS_N_BIT_25              0

	* Selects either Schmitt or CMOS input for MIO Bank 0 - control MIO[25:0]
	* (OFFSET, MASK, VALUE)      (0XFF180140, 0x03FFFFFFU ,0x00000000U)
	*/
	PSU_Mask_Write( IOU_SLCR_BANK0_CTRL3_OFFSET,
	                0x03FFFFFFU, 0x00000000U );
	/*##################################################################### */

	/*
	* Register : bank0_ctrl4 @ 0XFF180144

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL4_PULL_HIGH_LOW_N_BIT_0              1

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL4_PULL_HIGH_LOW_N_BIT_1              1

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL4_PULL_HIGH_LOW_N_BIT_2              1

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL4_PULL_HIGH_LOW_N_BIT_3              1

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL4_PULL_HIGH_LOW_N_BIT_4              1

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL4_PULL_HIGH_LOW_N_BIT_5              1

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL4_PULL_HIGH_LOW_N_BIT_6              1

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL4_PULL_HIGH_LOW_N_BIT_7              1

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL4_PULL_HIGH_LOW_N_BIT_8              1

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL4_PULL_HIGH_LOW_N_BIT_9              1

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL4_PULL_HIGH_LOW_N_BIT_10             1

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL4_PULL_HIGH_LOW_N_BIT_11             1

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL4_PULL_HIGH_LOW_N_BIT_12             1

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL4_PULL_HIGH_LOW_N_BIT_13             1

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL4_PULL_HIGH_LOW_N_BIT_14             1

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL4_PULL_HIGH_LOW_N_BIT_15             1

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL4_PULL_HIGH_LOW_N_BIT_16             1

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL4_PULL_HIGH_LOW_N_BIT_17             1

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL4_PULL_HIGH_LOW_N_BIT_18             1

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL4_PULL_HIGH_LOW_N_BIT_19             1

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL4_PULL_HIGH_LOW_N_BIT_20             1

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL4_PULL_HIGH_LOW_N_BIT_21             1

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL4_PULL_HIGH_LOW_N_BIT_22             1

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL4_PULL_HIGH_LOW_N_BIT_23             1

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL4_PULL_HIGH_LOW_N_BIT_24             1

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL4_PULL_HIGH_LOW_N_BIT_25             1

	* When mio_bank0_pull_enable is set, this selects pull up or pull down for
	*  MIO Bank 0 - control MIO[25:0]
	* (OFFSET, MASK, VALUE)      (0XFF180144, 0x03FFFFFFU ,0x03FFFFFFU)
	*/
	PSU_Mask_Write( IOU_SLCR_BANK0_CTRL4_OFFSET,
	                0x03FFFFFFU, 0x03FFFFFFU );
	/*##################################################################### */

	/*
	* Register : bank0_ctrl5 @ 0XFF180148

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL5_PULL_ENABLE_BIT_0                  1

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL5_PULL_ENABLE_BIT_1                  1

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL5_PULL_ENABLE_BIT_2                  1

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL5_PULL_ENABLE_BIT_3                  1

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL5_PULL_ENABLE_BIT_4                  1

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL5_PULL_ENABLE_BIT_5                  1

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL5_PULL_ENABLE_BIT_6                  1

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL5_PULL_ENABLE_BIT_7                  1

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL5_PULL_ENABLE_BIT_8                  1

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL5_PULL_ENABLE_BIT_9                  1

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL5_PULL_ENABLE_BIT_10                 1

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL5_PULL_ENABLE_BIT_11                 1

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL5_PULL_ENABLE_BIT_12                 1

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL5_PULL_ENABLE_BIT_13                 1

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL5_PULL_ENABLE_BIT_14                 1

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL5_PULL_ENABLE_BIT_15                 1

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL5_PULL_ENABLE_BIT_16                 1

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL5_PULL_ENABLE_BIT_17                 1

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL5_PULL_ENABLE_BIT_18                 1

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL5_PULL_ENABLE_BIT_19                 1

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL5_PULL_ENABLE_BIT_20                 1

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL5_PULL_ENABLE_BIT_21                 1

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL5_PULL_ENABLE_BIT_22                 1

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL5_PULL_ENABLE_BIT_23                 1

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL5_PULL_ENABLE_BIT_24                 1

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL5_PULL_ENABLE_BIT_25                 1

	* When set, this enables mio_bank0_pullupdown to selects pull up or pull d
	* own for MIO Bank 0 - control MIO[25:0]
	* (OFFSET, MASK, VALUE)      (0XFF180148, 0x03FFFFFFU ,0x03FFFFFFU)
	*/
	PSU_Mask_Write( IOU_SLCR_BANK0_CTRL5_OFFSET,
	                0x03FFFFFFU, 0x03FFFFFFU );
	/*##################################################################### */

	/*
	* Register : bank0_ctrl6 @ 0XFF18014C

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL6_SLOW_FAST_SLEW_N_BIT_0             1

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL6_SLOW_FAST_SLEW_N_BIT_1             1

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL6_SLOW_FAST_SLEW_N_BIT_2             1

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL6_SLOW_FAST_SLEW_N_BIT_3             1

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL6_SLOW_FAST_SLEW_N_BIT_4             1

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL6_SLOW_FAST_SLEW_N_BIT_5             1

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL6_SLOW_FAST_SLEW_N_BIT_6             1

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL6_SLOW_FAST_SLEW_N_BIT_7             1

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL6_SLOW_FAST_SLEW_N_BIT_8             1

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL6_SLOW_FAST_SLEW_N_BIT_9             1

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL6_SLOW_FAST_SLEW_N_BIT_10            1

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL6_SLOW_FAST_SLEW_N_BIT_11            1

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL6_SLOW_FAST_SLEW_N_BIT_12            1

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL6_SLOW_FAST_SLEW_N_BIT_13            1

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL6_SLOW_FAST_SLEW_N_BIT_14            1

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL6_SLOW_FAST_SLEW_N_BIT_15            1

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL6_SLOW_FAST_SLEW_N_BIT_16            1

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL6_SLOW_FAST_SLEW_N_BIT_17            1

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL6_SLOW_FAST_SLEW_N_BIT_18            1

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL6_SLOW_FAST_SLEW_N_BIT_19            1

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL6_SLOW_FAST_SLEW_N_BIT_20            1

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL6_SLOW_FAST_SLEW_N_BIT_21            1

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL6_SLOW_FAST_SLEW_N_BIT_22            1

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL6_SLOW_FAST_SLEW_N_BIT_23            1

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL6_SLOW_FAST_SLEW_N_BIT_24            1

	* Each bit applies to a single IO. Bit 0 for MIO[0].
	*  PSU_IOU_SLCR_BANK0_CTRL6_SLOW_FAST_SLEW_N_BIT_25            1

	* Slew rate control to MIO Bank 0 - control MIO[25:0]
	* (OFFSET, MASK, VALUE)      (0XFF18014C, 0x03FFFFFFU ,0x03FFFFFFU)
	*/
	PSU_Mask_Write( IOU_SLCR_BANK0_CTRL6_OFFSET,
	                0x03FFFFFFU, 0x03FFFFFFU );
	/*##################################################################### */

	/*
	* Register : bank1_ctrl0 @ 0XFF180154

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL0_DRIVE0_BIT_0                       1

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL0_DRIVE0_BIT_1                       0

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL0_DRIVE0_BIT_2                       1

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL0_DRIVE0_BIT_3                       0

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL0_DRIVE0_BIT_4                       1

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL0_DRIVE0_BIT_5                       1

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL0_DRIVE0_BIT_6                       0

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL0_DRIVE0_BIT_7                       0

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL0_DRIVE0_BIT_8                       0

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL0_DRIVE0_BIT_9                       0

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL0_DRIVE0_BIT_10                      0

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL0_DRIVE0_BIT_11                      1

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL0_DRIVE0_BIT_12                      0

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL0_DRIVE0_BIT_13                      0

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL0_DRIVE0_BIT_14                      0

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL0_DRIVE0_BIT_15                      0

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL0_DRIVE0_BIT_16                      0

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL0_DRIVE0_BIT_17                      0

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL0_DRIVE0_BIT_18                      0

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL0_DRIVE0_BIT_19                      1

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL0_DRIVE0_BIT_20                      0

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL0_DRIVE0_BIT_21                      0

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL0_DRIVE0_BIT_22                      0

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL0_DRIVE0_BIT_23                      0

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL0_DRIVE0_BIT_24                      0

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL0_DRIVE0_BIT_25                      0

	* Drive0 control to MIO Bank 1 - control MIO[51:26]
	* (OFFSET, MASK, VALUE)      (0XFF180154, 0x03FFFFFFU ,0x00080835U)
	*/
	PSU_Mask_Write( IOU_SLCR_BANK1_CTRL0_OFFSET,
	                0x03FFFFFFU, 0x00080835U );
	/*##################################################################### */

	/*
	* Register : bank1_ctrl1 @ 0XFF180158

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL1_DRIVE1_BIT_0                       1

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL1_DRIVE1_BIT_1                       1

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL1_DRIVE1_BIT_2                       1

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL1_DRIVE1_BIT_3                       1

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL1_DRIVE1_BIT_4                       1

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL1_DRIVE1_BIT_5                       1

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL1_DRIVE1_BIT_6                       1

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL1_DRIVE1_BIT_7                       1

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL1_DRIVE1_BIT_8                       1

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL1_DRIVE1_BIT_9                       1

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL1_DRIVE1_BIT_10                      1

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL1_DRIVE1_BIT_11                      1

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL1_DRIVE1_BIT_12                      1

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL1_DRIVE1_BIT_13                      1

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL1_DRIVE1_BIT_14                      1

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL1_DRIVE1_BIT_15                      1

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL1_DRIVE1_BIT_16                      1

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL1_DRIVE1_BIT_17                      1

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL1_DRIVE1_BIT_18                      1

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL1_DRIVE1_BIT_19                      1

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL1_DRIVE1_BIT_20                      1

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL1_DRIVE1_BIT_21                      1

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL1_DRIVE1_BIT_22                      1

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL1_DRIVE1_BIT_23                      1

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL1_DRIVE1_BIT_24                      1

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL1_DRIVE1_BIT_25                      1

	* Drive1 control to MIO Bank 1 - control MIO[51:26]
	* (OFFSET, MASK, VALUE)      (0XFF180158, 0x03FFFFFFU ,0x03FFFFFFU)
	*/
	PSU_Mask_Write( IOU_SLCR_BANK1_CTRL1_OFFSET,
	                0x03FFFFFFU, 0x03FFFFFFU );
	/*##################################################################### */

	/*
	* Register : bank1_ctrl3 @ 0XFF18015C

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL3_SCHMITT_CMOS_N_BIT_0               0

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL3_SCHMITT_CMOS_N_BIT_1               0

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL3_SCHMITT_CMOS_N_BIT_2               0

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL3_SCHMITT_CMOS_N_BIT_3               0

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL3_SCHMITT_CMOS_N_BIT_4               0

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL3_SCHMITT_CMOS_N_BIT_5               0

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL3_SCHMITT_CMOS_N_BIT_6               0

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL3_SCHMITT_CMOS_N_BIT_7               0

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL3_SCHMITT_CMOS_N_BIT_8               0

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL3_SCHMITT_CMOS_N_BIT_9               0

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL3_SCHMITT_CMOS_N_BIT_10              0

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL3_SCHMITT_CMOS_N_BIT_11              0

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL3_SCHMITT_CMOS_N_BIT_12              0

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL3_SCHMITT_CMOS_N_BIT_13              0

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL3_SCHMITT_CMOS_N_BIT_14              0

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL3_SCHMITT_CMOS_N_BIT_15              0

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL3_SCHMITT_CMOS_N_BIT_16              0

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL3_SCHMITT_CMOS_N_BIT_17              0

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL3_SCHMITT_CMOS_N_BIT_18              0

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL3_SCHMITT_CMOS_N_BIT_19              0

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL3_SCHMITT_CMOS_N_BIT_20              0

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL3_SCHMITT_CMOS_N_BIT_21              0

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL3_SCHMITT_CMOS_N_BIT_22              0

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL3_SCHMITT_CMOS_N_BIT_23              0

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL3_SCHMITT_CMOS_N_BIT_24              0

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL3_SCHMITT_CMOS_N_BIT_25              0

	* Selects either Schmitt or CMOS input for MIO Bank 1 - control MIO[51:26]
	* (OFFSET, MASK, VALUE)      (0XFF18015C, 0x03FFFFFFU ,0x00000000U)
	*/
	PSU_Mask_Write( IOU_SLCR_BANK1_CTRL3_OFFSET,
	                0x03FFFFFFU, 0x00000000U );
	/*##################################################################### */

	/*
	* Register : bank1_ctrl4 @ 0XFF180160

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL4_PULL_HIGH_LOW_N_BIT_0              1

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL4_PULL_HIGH_LOW_N_BIT_1              1

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL4_PULL_HIGH_LOW_N_BIT_2              1

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL4_PULL_HIGH_LOW_N_BIT_3              1

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL4_PULL_HIGH_LOW_N_BIT_4              1

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL4_PULL_HIGH_LOW_N_BIT_5              1

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL4_PULL_HIGH_LOW_N_BIT_6              1

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL4_PULL_HIGH_LOW_N_BIT_7              1

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL4_PULL_HIGH_LOW_N_BIT_8              1

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL4_PULL_HIGH_LOW_N_BIT_9              1

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL4_PULL_HIGH_LOW_N_BIT_10             1

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL4_PULL_HIGH_LOW_N_BIT_11             1

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL4_PULL_HIGH_LOW_N_BIT_12             1

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL4_PULL_HIGH_LOW_N_BIT_13             1

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL4_PULL_HIGH_LOW_N_BIT_14             1

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL4_PULL_HIGH_LOW_N_BIT_15             1

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL4_PULL_HIGH_LOW_N_BIT_16             1

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL4_PULL_HIGH_LOW_N_BIT_17             1

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL4_PULL_HIGH_LOW_N_BIT_18             1

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL4_PULL_HIGH_LOW_N_BIT_19             1

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL4_PULL_HIGH_LOW_N_BIT_20             1

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL4_PULL_HIGH_LOW_N_BIT_21             1

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL4_PULL_HIGH_LOW_N_BIT_22             1

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL4_PULL_HIGH_LOW_N_BIT_23             1

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL4_PULL_HIGH_LOW_N_BIT_24             1

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL4_PULL_HIGH_LOW_N_BIT_25             1

	* When mio_bank1_pull_enable is set, this selects pull up or pull down for
	*  MIO Bank 1 - control MIO[51:26]
	* (OFFSET, MASK, VALUE)      (0XFF180160, 0x03FFFFFFU ,0x03FFFFFFU)
	*/
	PSU_Mask_Write( IOU_SLCR_BANK1_CTRL4_OFFSET,
	                0x03FFFFFFU, 0x03FFFFFFU );
	/*##################################################################### */

	/*
	* Register : bank1_ctrl5 @ 0XFF180164

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL5_PULL_ENABLE_BIT_0                  1

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL5_PULL_ENABLE_BIT_1                  1

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL5_PULL_ENABLE_BIT_2                  1

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL5_PULL_ENABLE_BIT_3                  1

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL5_PULL_ENABLE_BIT_4                  1

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL5_PULL_ENABLE_BIT_5                  1

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL5_PULL_ENABLE_BIT_6                  1

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL5_PULL_ENABLE_BIT_7                  1

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL5_PULL_ENABLE_BIT_8                  1

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL5_PULL_ENABLE_BIT_9                  1

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL5_PULL_ENABLE_BIT_10                 1

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL5_PULL_ENABLE_BIT_11                 1

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL5_PULL_ENABLE_BIT_12                 1

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL5_PULL_ENABLE_BIT_13                 1

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL5_PULL_ENABLE_BIT_14                 1

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL5_PULL_ENABLE_BIT_15                 1

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL5_PULL_ENABLE_BIT_16                 1

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL5_PULL_ENABLE_BIT_17                 1

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL5_PULL_ENABLE_BIT_18                 1

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL5_PULL_ENABLE_BIT_19                 1

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL5_PULL_ENABLE_BIT_20                 1

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL5_PULL_ENABLE_BIT_21                 1

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL5_PULL_ENABLE_BIT_22                 1

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL5_PULL_ENABLE_BIT_23                 1

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL5_PULL_ENABLE_BIT_24                 1

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL5_PULL_ENABLE_BIT_25                 1

	* When set, this enables mio_bank1_pullupdown to selects pull up or pull d
	* own for MIO Bank 1 - control MIO[51:26]
	* (OFFSET, MASK, VALUE)      (0XFF180164, 0x03FFFFFFU ,0x03FFFFFFU)
	*/
	PSU_Mask_Write( IOU_SLCR_BANK1_CTRL5_OFFSET,
	                0x03FFFFFFU, 0x03FFFFFFU );
	/*##################################################################### */

	/*
	* Register : bank1_ctrl6 @ 0XFF180168

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL6_SLOW_FAST_SLEW_N_BIT_0             0

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL6_SLOW_FAST_SLEW_N_BIT_1             1

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL6_SLOW_FAST_SLEW_N_BIT_2             0

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL6_SLOW_FAST_SLEW_N_BIT_3             1

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL6_SLOW_FAST_SLEW_N_BIT_4             0

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL6_SLOW_FAST_SLEW_N_BIT_5             0

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL6_SLOW_FAST_SLEW_N_BIT_6             1

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL6_SLOW_FAST_SLEW_N_BIT_7             1

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL6_SLOW_FAST_SLEW_N_BIT_8             1

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL6_SLOW_FAST_SLEW_N_BIT_9             1

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL6_SLOW_FAST_SLEW_N_BIT_10            1

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL6_SLOW_FAST_SLEW_N_BIT_11            0

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL6_SLOW_FAST_SLEW_N_BIT_12            1

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL6_SLOW_FAST_SLEW_N_BIT_13            1

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL6_SLOW_FAST_SLEW_N_BIT_14            1

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL6_SLOW_FAST_SLEW_N_BIT_15            1

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL6_SLOW_FAST_SLEW_N_BIT_16            1

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL6_SLOW_FAST_SLEW_N_BIT_17            1

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL6_SLOW_FAST_SLEW_N_BIT_18            1

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL6_SLOW_FAST_SLEW_N_BIT_19            0

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL6_SLOW_FAST_SLEW_N_BIT_20            1

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL6_SLOW_FAST_SLEW_N_BIT_21            1

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL6_SLOW_FAST_SLEW_N_BIT_22            1

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL6_SLOW_FAST_SLEW_N_BIT_23            1

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL6_SLOW_FAST_SLEW_N_BIT_24            1

	* Each bit applies to a single IO. Bit 0 for MIO[26].
	*  PSU_IOU_SLCR_BANK1_CTRL6_SLOW_FAST_SLEW_N_BIT_25            1

	* Slew rate control to MIO Bank 1 - control MIO[51:26]
	* (OFFSET, MASK, VALUE)      (0XFF180168, 0x03FFFFFFU ,0x03F7F7CAU)
	*/
	PSU_Mask_Write( IOU_SLCR_BANK1_CTRL6_OFFSET,
	                0x03FFFFFFU, 0x03F7F7CAU );
	/*##################################################################### */

	/*
	* Register : bank2_ctrl0 @ 0XFF180170

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL0_DRIVE0_BIT_0                       1

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL0_DRIVE0_BIT_1                       1

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL0_DRIVE0_BIT_2                       0

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL0_DRIVE0_BIT_3                       1

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL0_DRIVE0_BIT_4                       0

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL0_DRIVE0_BIT_5                       0

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL0_DRIVE0_BIT_6                       0

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL0_DRIVE0_BIT_7                       0

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL0_DRIVE0_BIT_8                       0

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL0_DRIVE0_BIT_9                       0

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL0_DRIVE0_BIT_10                      0

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL0_DRIVE0_BIT_11                      0

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL0_DRIVE0_BIT_12                      0

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL0_DRIVE0_BIT_13                      0

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL0_DRIVE0_BIT_14                      0

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL0_DRIVE0_BIT_15                      0

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL0_DRIVE0_BIT_16                      0

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL0_DRIVE0_BIT_17                      0

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL0_DRIVE0_BIT_18                      1

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL0_DRIVE0_BIT_19                      1

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL0_DRIVE0_BIT_20                      1

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL0_DRIVE0_BIT_21                      1

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL0_DRIVE0_BIT_22                      1

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL0_DRIVE0_BIT_23                      1

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL0_DRIVE0_BIT_24                      0

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL0_DRIVE0_BIT_25                      0

	* Drive0 control to MIO Bank 2 - control MIO[77:52]
	* (OFFSET, MASK, VALUE)      (0XFF180170, 0x03FFFFFFU ,0x00FC000BU)
	*/
	PSU_Mask_Write( IOU_SLCR_BANK2_CTRL0_OFFSET,
	                0x03FFFFFFU, 0x00FC000BU );
	/*##################################################################### */

	/*
	* Register : bank2_ctrl1 @ 0XFF180174

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL1_DRIVE1_BIT_0                       1

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL1_DRIVE1_BIT_1                       1

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL1_DRIVE1_BIT_2                       1

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL1_DRIVE1_BIT_3                       1

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL1_DRIVE1_BIT_4                       1

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL1_DRIVE1_BIT_5                       1

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL1_DRIVE1_BIT_6                       1

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL1_DRIVE1_BIT_7                       1

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL1_DRIVE1_BIT_8                       1

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL1_DRIVE1_BIT_9                       1

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL1_DRIVE1_BIT_10                      1

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL1_DRIVE1_BIT_11                      1

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL1_DRIVE1_BIT_12                      1

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL1_DRIVE1_BIT_13                      1

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL1_DRIVE1_BIT_14                      1

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL1_DRIVE1_BIT_15                      1

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL1_DRIVE1_BIT_16                      1

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL1_DRIVE1_BIT_17                      1

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL1_DRIVE1_BIT_18                      1

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL1_DRIVE1_BIT_19                      1

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL1_DRIVE1_BIT_20                      1

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL1_DRIVE1_BIT_21                      1

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL1_DRIVE1_BIT_22                      1

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL1_DRIVE1_BIT_23                      1

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL1_DRIVE1_BIT_24                      1

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL1_DRIVE1_BIT_25                      1

	* Drive1 control to MIO Bank 2 - control MIO[77:52]
	* (OFFSET, MASK, VALUE)      (0XFF180174, 0x03FFFFFFU ,0x03FFFFFFU)
	*/
	PSU_Mask_Write( IOU_SLCR_BANK2_CTRL1_OFFSET,
	                0x03FFFFFFU, 0x03FFFFFFU );
	/*##################################################################### */

	/*
	* Register : bank2_ctrl3 @ 0XFF180178

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL3_SCHMITT_CMOS_N_BIT_0               0

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL3_SCHMITT_CMOS_N_BIT_1               0

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL3_SCHMITT_CMOS_N_BIT_2               0

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL3_SCHMITT_CMOS_N_BIT_3               0

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL3_SCHMITT_CMOS_N_BIT_4               0

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL3_SCHMITT_CMOS_N_BIT_5               0

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL3_SCHMITT_CMOS_N_BIT_6               0

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL3_SCHMITT_CMOS_N_BIT_7               0

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL3_SCHMITT_CMOS_N_BIT_8               0

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL3_SCHMITT_CMOS_N_BIT_9               0

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL3_SCHMITT_CMOS_N_BIT_10              0

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL3_SCHMITT_CMOS_N_BIT_11              0

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL3_SCHMITT_CMOS_N_BIT_12              0

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL3_SCHMITT_CMOS_N_BIT_13              0

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL3_SCHMITT_CMOS_N_BIT_14              0

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL3_SCHMITT_CMOS_N_BIT_15              0

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL3_SCHMITT_CMOS_N_BIT_16              0

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL3_SCHMITT_CMOS_N_BIT_17              0

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL3_SCHMITT_CMOS_N_BIT_18              0

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL3_SCHMITT_CMOS_N_BIT_19              0

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL3_SCHMITT_CMOS_N_BIT_20              0

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL3_SCHMITT_CMOS_N_BIT_21              0

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL3_SCHMITT_CMOS_N_BIT_22              0

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL3_SCHMITT_CMOS_N_BIT_23              0

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL3_SCHMITT_CMOS_N_BIT_24              0

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL3_SCHMITT_CMOS_N_BIT_25              0

	* Selects either Schmitt or CMOS input for MIO Bank 2 - control MIO[77:52]
	* (OFFSET, MASK, VALUE)      (0XFF180178, 0x03FFFFFFU ,0x00000000U)
	*/
	PSU_Mask_Write( IOU_SLCR_BANK2_CTRL3_OFFSET,
	                0x03FFFFFFU, 0x00000000U );
	/*##################################################################### */

	/*
	* Register : bank2_ctrl4 @ 0XFF18017C

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL4_PULL_HIGH_LOW_N_BIT_0              1

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL4_PULL_HIGH_LOW_N_BIT_1              1

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL4_PULL_HIGH_LOW_N_BIT_2              1

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL4_PULL_HIGH_LOW_N_BIT_3              1

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL4_PULL_HIGH_LOW_N_BIT_4              1

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL4_PULL_HIGH_LOW_N_BIT_5              1

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL4_PULL_HIGH_LOW_N_BIT_6              1

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL4_PULL_HIGH_LOW_N_BIT_7              1

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL4_PULL_HIGH_LOW_N_BIT_8              1

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL4_PULL_HIGH_LOW_N_BIT_9              1

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL4_PULL_HIGH_LOW_N_BIT_10             1

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL4_PULL_HIGH_LOW_N_BIT_11             1

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL4_PULL_HIGH_LOW_N_BIT_12             1

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL4_PULL_HIGH_LOW_N_BIT_13             1

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL4_PULL_HIGH_LOW_N_BIT_14             1

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL4_PULL_HIGH_LOW_N_BIT_15             1

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL4_PULL_HIGH_LOW_N_BIT_16             1

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL4_PULL_HIGH_LOW_N_BIT_17             1

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL4_PULL_HIGH_LOW_N_BIT_18             1

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL4_PULL_HIGH_LOW_N_BIT_19             1

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL4_PULL_HIGH_LOW_N_BIT_20             1

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL4_PULL_HIGH_LOW_N_BIT_21             1

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL4_PULL_HIGH_LOW_N_BIT_22             1

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL4_PULL_HIGH_LOW_N_BIT_23             1

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL4_PULL_HIGH_LOW_N_BIT_24             1

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL4_PULL_HIGH_LOW_N_BIT_25             1

	* When mio_bank2_pull_enable is set, this selects pull up or pull down for
	*  MIO Bank 2 - control MIO[77:52]
	* (OFFSET, MASK, VALUE)      (0XFF18017C, 0x03FFFFFFU ,0x03FFFFFFU)
	*/
	PSU_Mask_Write( IOU_SLCR_BANK2_CTRL4_OFFSET,
	                0x03FFFFFFU, 0x03FFFFFFU );
	/*##################################################################### */

	/*
	* Register : bank2_ctrl5 @ 0XFF180180

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL5_PULL_ENABLE_BIT_0                  1

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL5_PULL_ENABLE_BIT_1                  1

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL5_PULL_ENABLE_BIT_2                  1

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL5_PULL_ENABLE_BIT_3                  1

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL5_PULL_ENABLE_BIT_4                  1

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL5_PULL_ENABLE_BIT_5                  1

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL5_PULL_ENABLE_BIT_6                  1

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL5_PULL_ENABLE_BIT_7                  1

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL5_PULL_ENABLE_BIT_8                  1

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL5_PULL_ENABLE_BIT_9                  1

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL5_PULL_ENABLE_BIT_10                 1

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL5_PULL_ENABLE_BIT_11                 1

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL5_PULL_ENABLE_BIT_12                 1

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL5_PULL_ENABLE_BIT_13                 1

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL5_PULL_ENABLE_BIT_14                 1

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL5_PULL_ENABLE_BIT_15                 1

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL5_PULL_ENABLE_BIT_16                 1

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL5_PULL_ENABLE_BIT_17                 1

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL5_PULL_ENABLE_BIT_18                 1

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL5_PULL_ENABLE_BIT_19                 1

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL5_PULL_ENABLE_BIT_20                 1

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL5_PULL_ENABLE_BIT_21                 1

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL5_PULL_ENABLE_BIT_22                 1

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL5_PULL_ENABLE_BIT_23                 1

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL5_PULL_ENABLE_BIT_24                 1

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL5_PULL_ENABLE_BIT_25                 1

	* When set, this enables mio_bank2_pullupdown to selects pull up or pull d
	* own for MIO Bank 2 - control MIO[77:52]
	* (OFFSET, MASK, VALUE)      (0XFF180180, 0x03FFFFFFU ,0x03FFFFFFU)
	*/
	PSU_Mask_Write( IOU_SLCR_BANK2_CTRL5_OFFSET,
	                0x03FFFFFFU, 0x03FFFFFFU );
	/*##################################################################### */

	/*
	* Register : bank2_ctrl6 @ 0XFF180184

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL6_SLOW_FAST_SLEW_N_BIT_0             0

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL6_SLOW_FAST_SLEW_N_BIT_1             0

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL6_SLOW_FAST_SLEW_N_BIT_2             1

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL6_SLOW_FAST_SLEW_N_BIT_3             0

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL6_SLOW_FAST_SLEW_N_BIT_4             1

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL6_SLOW_FAST_SLEW_N_BIT_5             1

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL6_SLOW_FAST_SLEW_N_BIT_6             1

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL6_SLOW_FAST_SLEW_N_BIT_7             1

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL6_SLOW_FAST_SLEW_N_BIT_8             1

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL6_SLOW_FAST_SLEW_N_BIT_9             1

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL6_SLOW_FAST_SLEW_N_BIT_10            1

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL6_SLOW_FAST_SLEW_N_BIT_11            1

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL6_SLOW_FAST_SLEW_N_BIT_12            1

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL6_SLOW_FAST_SLEW_N_BIT_13            1

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL6_SLOW_FAST_SLEW_N_BIT_14            1

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL6_SLOW_FAST_SLEW_N_BIT_15            1

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL6_SLOW_FAST_SLEW_N_BIT_16            1

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL6_SLOW_FAST_SLEW_N_BIT_17            1

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL6_SLOW_FAST_SLEW_N_BIT_18            0

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL6_SLOW_FAST_SLEW_N_BIT_19            0

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL6_SLOW_FAST_SLEW_N_BIT_20            0

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL6_SLOW_FAST_SLEW_N_BIT_21            0

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL6_SLOW_FAST_SLEW_N_BIT_22            0

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL6_SLOW_FAST_SLEW_N_BIT_23            0

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL6_SLOW_FAST_SLEW_N_BIT_24            1

	* Each bit applies to a single IO. Bit 0 for MIO[52].
	*  PSU_IOU_SLCR_BANK2_CTRL6_SLOW_FAST_SLEW_N_BIT_25            1

	* Slew rate control to MIO Bank 2 - control MIO[77:52]
	* (OFFSET, MASK, VALUE)      (0XFF180184, 0x03FFFFFFU ,0x0303FFF4U)
	*/
	PSU_Mask_Write( IOU_SLCR_BANK2_CTRL6_OFFSET,
	                0x03FFFFFFU, 0x0303FFF4U );
	/*##################################################################### */

	/*
	* LOOPBACK
	*/
	/*
	* Register : MIO_LOOPBACK @ 0XFF180200

	* I2C Loopback Control. 0 = Connect I2C inputs according to MIO mapping. 1
	*  = Loop I2C 0 outputs to I2C 1 inputs, and I2C 1 outputs to I2C 0 inputs
	* .
	*  PSU_IOU_SLCR_MIO_LOOPBACK_I2C0_LOOP_I2C1                    0

	* CAN Loopback Control. 0 = Connect CAN inputs according to MIO mapping. 1
	*  = Loop CAN 0 Tx to CAN 1 Rx, and CAN 1 Tx to CAN 0 Rx.
	*  PSU_IOU_SLCR_MIO_LOOPBACK_CAN0_LOOP_CAN1                    0

	* UART Loopback Control. 0 = Connect UART inputs according to MIO mapping.
	*  1 = Loop UART 0 outputs to UART 1 inputs, and UART 1 outputs to UART 0
	* inputs. RXD/TXD cross-connected. RTS/CTS cross-connected. DSR, DTR, DCD
	* and RI not used.
	*  PSU_IOU_SLCR_MIO_LOOPBACK_UA0_LOOP_UA1                      0

	* SPI Loopback Control. 0 = Connect SPI inputs according to MIO mapping. 1
	*  = Loop SPI 0 outputs to SPI 1 inputs, and SPI 1 outputs to SPI 0 inputs
	* . The other SPI core will appear on the LS Slave Select.
	*  PSU_IOU_SLCR_MIO_LOOPBACK_SPI0_LOOP_SPI1                    0

	* Loopback function within MIO
	* (OFFSET, MASK, VALUE)      (0XFF180200, 0x0000000FU ,0x00000000U)
	*/
	PSU_Mask_Write( IOU_SLCR_MIO_LOOPBACK_OFFSET,
	                0x0000000FU, 0x00000000U );
	/*##################################################################### */


	return 1;
}
