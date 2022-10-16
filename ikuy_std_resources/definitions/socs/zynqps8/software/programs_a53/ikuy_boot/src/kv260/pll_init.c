#include "core/core.h"
#include "platform/reg_access.h"
#include "utils/boot_psi.h"
#include "platform/memory_map.h"
#include "platform/registers/crl_apb.h"
#include "platform/registers/crf_apb.h"

__attribute__((__section__(".hwregs")))
static PSI_IWord const pll_init[] = {
		PSI_SET_REGISTER_BANK(CRL_APB),

		PSI_WRITE_MASKED_32(CRL_APB, PSSYSMON_REF_CTRL, 0x013F3F07U, 0x01012302U),
		PSI_WRITE_MASKED_32(CRL_APB, RST_LPD_IOU2, 0x00000001U, 0x00000001U),

		PSI_WRITE_MASKED_32(CRL_APB, RPLL_CFG, 0xFE7FEDEFU, 0x7E4B0C62U),
		PSI_WRITE_MASKED_32(CRL_APB, RPLL_CTRL,
												HW_REG_FIELD_MASK(CRL_APB, RPLL_CTRL, POST_SRC) |
												HW_REG_FIELD_MASK(CRL_APB, RPLL_CTRL, PRE_SRC) |
												HW_REG_FIELD_MASK(CRL_APB, RPLL_CTRL, DIV2) |
												HW_REG_FIELD_MASK(CRL_APB, RPLL_CTRL, FBDIV),

												HW_REG_ENCODE_ENUM(CRL_APB, RPLL_CTRL, POST_SRC, PS_REF_CLK) |  // PS_REF_CLK 33.3Mhz
												HW_REG_ENCODE_ENUM(CRL_APB, RPLL_CTRL, PRE_SRC, PS_REF_CLK) |     // PS_REF_CLK
												HW_REG_ENCODE_FIELD(CRL_APB, RPLL_CTRL, DIV2, 1) |        // / 2
												HW_REG_ENCODE_FIELD(CRL_APB, RPLL_CTRL, FBDIV, 0x40)      // / 64 = 33.3 * 32 = 1064Mhz
												),
		PSI_WRITE_MASKED_32(CRL_APB, RPLL_CTRL,
												HW_REG_FIELD_MASK(CRL_APB, RPLL_CTRL, BYPASS),
												HW_REG_ENCODE_FIELD(CRL_APB, RPLL_CTRL, BYPASS, 1)),
		PSI_WRITE_MASKED_32(CRL_APB, RPLL_CTRL,
												HW_REG_FIELD_MASK(CRL_APB, RPLL_CTRL, RESET),
												HW_REG_ENCODE_FIELD(CRL_APB, RPLL_CTRL, RESET, 1)),
		PSI_WRITE_MASKED_32(CRL_APB, RPLL_CTRL,
												HW_REG_FIELD_MASK(CRL_APB, RPLL_CTRL, RESET),
												HW_REG_ENCODE_FIELD(CRL_APB, RPLL_CTRL, RESET, 0)),
		PSI_POLL_MASKED_32(CRL_APB, PLL_STATUS, CRL_APB_PLL_STATUS_RPLL_LOCK_MASK),
		PSI_WRITE_MASKED_32(CRL_APB, RPLL_CTRL,
												HW_REG_FIELD_MASK(CRL_APB, RPLL_CTRL, BYPASS),
												HW_REG_ENCODE_FIELD(CRL_APB, RPLL_CTRL, BYPASS, 0)),
		PSI_WRITE_32(CRL_APB, RPLL_TO_FPD_CTRL,
											 HW_REG_ENCODE_FIELD(CRL_APB, RPLL_TO_FPD_CTRL, DIVISOR0,0x2)), // RPLL_TO_FPD = 532 Mhz

		PSI_WRITE_MASKED_32(CRL_APB, PSSYSMON_REF_CTRL, 0x013F3F07U, 0x01012300U),

		PSI_WRITE_MASKED_32(CRL_APB, IOPLL_CFG, CRL_APB_IOPLL_CFG_USERMASK, 0x7E4B0C82U),
		PSI_WRITE_MASKED_32(CRL_APB, IOPLL_CTRL,
												HW_REG_FIELD_MASK(CRL_APB, IOPLL_CTRL, POST_SRC) |
												HW_REG_FIELD_MASK(CRL_APB, IOPLL_CTRL, PRE_SRC) |
												HW_REG_FIELD_MASK(CRL_APB, IOPLL_CTRL, DIV2) |
												HW_REG_FIELD_MASK(CRL_APB, IOPLL_CTRL, FBDIV),

												HW_REG_ENCODE_ENUM(CRL_APB, IOPLL_CTRL, POST_SRC, PS_REF_CLK) |  // PS_REF_CLK 33.3Mhz
												HW_REG_ENCODE_ENUM(CRL_APB, IOPLL_CTRL, PRE_SRC, PS_REF_CLK) |     // PS_REF_CLK
												HW_REG_ENCODE_FIELD(CRL_APB, IOPLL_CTRL, DIV2, 1) |        // / 2
												HW_REG_ENCODE_FIELD(CRL_APB, IOPLL_CTRL, FBDIV, 0x5A)      // / 90 = 33.3 * 45 = 1500Mhz
		),
		PSI_WRITE_MASKED_32(CRL_APB, IOPLL_CTRL,
												HW_REG_FIELD_MASK(CRL_APB, IOPLL_CTRL, BYPASS),
												HW_REG_ENCODE_FIELD(CRL_APB, IOPLL_CTRL, BYPASS, 1)),
		PSI_WRITE_MASKED_32(CRL_APB, IOPLL_CTRL,
												HW_REG_FIELD_MASK(CRL_APB, IOPLL_CTRL, RESET),
												HW_REG_ENCODE_FIELD(CRL_APB, IOPLL_CTRL, RESET, 1)),
		PSI_WRITE_MASKED_32(CRL_APB, IOPLL_CTRL,
												HW_REG_FIELD_MASK(CRL_APB, IOPLL_CTRL, RESET),
												HW_REG_ENCODE_FIELD(CRL_APB, IOPLL_CTRL, RESET, 0)),
		PSI_POLL_MASKED_32(CRL_APB, PLL_STATUS, CRL_APB_PLL_STATUS_IOPLL_LOCK_MASK),
		PSI_WRITE_MASKED_32(CRL_APB, IOPLL_CTRL,
												HW_REG_FIELD_MASK(CRL_APB, IOPLL_CTRL, BYPASS),
												HW_REG_ENCODE_FIELD(CRL_APB, IOPLL_CTRL, BYPASS, 0)),
		PSI_WRITE_32(CRL_APB, IOPLL_TO_FPD_CTRL,
												HW_REG_ENCODE_FIELD(CRL_APB, IOPLL_TO_FPD_CTRL, DIVISOR0,0x3)), // IOPLL_TO_FPD = 500 Mhz

		PSI_SET_REGISTER_BANK(CRF_APB),
		PSI_WRITE_MASKED_32(CRF_APB, APLL_CFG, 0xFE7FEDEFU, 0x7E4B0C62U),
		PSI_WRITE_MASKED_32(CRF_APB, APLL_CTRL,
												HW_REG_FIELD_MASK(CRF_APB, APLL_CTRL, POST_SRC) |
												HW_REG_FIELD_MASK(CRF_APB, APLL_CTRL, PRE_SRC) |
												HW_REG_FIELD_MASK(CRF_APB, APLL_CTRL, DIV2) |
												HW_REG_FIELD_MASK(CRF_APB, APLL_CTRL, FBDIV),
												HW_REG_ENCODE_ENUM(CRF_APB, APLL_CTRL, POST_SRC, PS_REF_CLK) |
												HW_REG_ENCODE_ENUM(CRF_APB, APLL_CTRL, PRE_SRC, PS_REF_CLK) |
												HW_REG_ENCODE_FIELD(CRF_APB, APLL_CTRL, DIV2, 1)|				// / 2
												HW_REG_ENCODE_FIELD(CRF_APB, APLL_CTRL, FBDIV, 0x50) ), // / 80 = 33.3 * 40 = 1332Mhz
		PSI_WRITE_MASKED_32(CRF_APB, APLL_CTRL,
												HW_REG_FIELD_MASK(CRF_APB, APLL_CTRL, BYPASS),
												HW_REG_ENCODE_FIELD(CRF_APB, APLL_CTRL, BYPASS, 1)),
		PSI_WRITE_MASKED_32(CRF_APB, APLL_CTRL,
												HW_REG_FIELD_MASK(CRF_APB, APLL_CTRL, RESET),
												HW_REG_ENCODE_FIELD(CRF_APB, APLL_CTRL, RESET, 1)),
		PSI_WRITE_MASKED_32(CRF_APB, APLL_CTRL,
												HW_REG_FIELD_MASK(CRF_APB, APLL_CTRL, RESET),
												HW_REG_ENCODE_FIELD(CRF_APB, APLL_CTRL, RESET, 0)),
		PSI_POLL_MASKED_32(CRF_APB, PLL_STATUS, 0x00000001U),
		PSI_WRITE_MASKED_32(CRF_APB, APLL_CTRL,
												HW_REG_FIELD_MASK(CRF_APB, APLL_CTRL, BYPASS),
												HW_REG_ENCODE_FIELD(CRF_APB, APLL_CTRL, BYPASS, 0)),
		PSI_WRITE_32(CRF_APB, APLL_TO_LPD_CTRL,
								 				HW_REG_ENCODE_FIELD(CRF_APB, APLL_TO_LPD_CTRL, DIVISOR0,0x3)), // APLL_TO_LPD = 444 Mhz
		PSI_WRITE_MASKED_32(CRF_APB, APLL_FRAC_CFG, 0x8000FFFFU, 0x80000033U),

		PSI_WRITE_MASKED_32(CRF_APB, DPLL_CFG, 0xFE7FEDEFU, 0x7E4B0C62U),
		PSI_WRITE_MASKED_32(CRF_APB, DPLL_CTRL,
												HW_REG_FIELD_MASK(CRF_APB, DPLL_CTRL, POST_SRC) |
												HW_REG_FIELD_MASK(CRF_APB, DPLL_CTRL, PRE_SRC) |
												HW_REG_FIELD_MASK(CRF_APB, DPLL_CTRL, DIV2) |
												HW_REG_FIELD_MASK(CRF_APB, DPLL_CTRL, FBDIV),
												HW_REG_ENCODE_ENUM(CRF_APB, DPLL_CTRL, POST_SRC, PS_REF_CLK) |
												HW_REG_ENCODE_ENUM(CRF_APB, DPLL_CTRL, PRE_SRC, PS_REF_CLK) |
												HW_REG_ENCODE_FIELD(CRF_APB, DPLL_CTRL, DIV2, 1)|				// / 2
												HW_REG_ENCODE_FIELD(CRF_APB, DPLL_CTRL, FBDIV, 0x40) ), // / 64 = 33.3 * 32 = 1066Mhz
		PSI_WRITE_MASKED_32(CRF_APB, DPLL_CTRL,
												HW_REG_FIELD_MASK(CRF_APB, DPLL_CTRL, BYPASS),
												HW_REG_ENCODE_FIELD(CRF_APB, DPLL_CTRL, BYPASS, 1)),
		PSI_WRITE_MASKED_32(CRF_APB, DPLL_CTRL,
												HW_REG_FIELD_MASK(CRF_APB, DPLL_CTRL, RESET),
												HW_REG_ENCODE_FIELD(CRF_APB, DPLL_CTRL, RESET, 1)),
		PSI_WRITE_MASKED_32(CRF_APB, DPLL_CTRL,
												HW_REG_FIELD_MASK(CRF_APB, DPLL_CTRL, RESET),
												HW_REG_ENCODE_FIELD(CRF_APB, DPLL_CTRL, RESET, 0)),
		PSI_POLL_MASKED_32(CRF_APB, PLL_STATUS, 0x00000002U),
		PSI_WRITE_MASKED_32(CRF_APB, DPLL_CTRL,
												HW_REG_FIELD_MASK(CRF_APB, DPLL_CTRL, BYPASS),
												HW_REG_ENCODE_FIELD(CRF_APB, DPLL_CTRL, BYPASS, 0)),
		PSI_WRITE_32(CRF_APB, DPLL_TO_LPD_CTRL,
								 				HW_REG_ENCODE_FIELD(CRF_APB, DPLL_TO_LPD_CTRL, DIVISOR0,0x2)), // DPLL_TO_LPD = 533 Mhz

		PSI_WRITE_MASKED_32(CRF_APB, VPLL_CFG, 0xFE7FEDEFU, 0x7E4B0C82U),
		PSI_WRITE_32(CRF_APB, VPLL_CTRL,
												HW_REG_ENCODE_ENUM(CRF_APB, VPLL_CTRL, POST_SRC, PS_REF_CLK) |
												HW_REG_ENCODE_ENUM(CRF_APB, VPLL_CTRL, PRE_SRC, PS_REF_CLK) |
												HW_REG_ENCODE_FIELD(CRF_APB, VPLL_CTRL, DIV2, 1)|				// / 2
												HW_REG_ENCODE_FIELD(CRF_APB, VPLL_CTRL, FBDIV, 0x5a) |  // / 90 = 33.3 * 45 = 1500Mhz
												HW_REG_ENCODE_FIELD(CRF_APB, VPLL_CTRL, BYPASS, 1) |
												HW_REG_ENCODE_FIELD(CRF_APB, VPLL_CTRL, RESET, 1)
												),
		PSI_DELAY_US(20),
		PSI_WRITE_MASKED_32(CRF_APB, VPLL_CTRL,
												HW_REG_FIELD_MASK(CRF_APB, VPLL_CTRL, RESET),
												HW_REG_ENCODE_FIELD(CRF_APB, VPLL_CTRL, RESET, 0)),
		PSI_POLL_MASKED_32(CRF_APB, PLL_STATUS, 0x00000004U),
		PSI_WRITE_MASKED_32(CRF_APB, VPLL_CTRL,
												HW_REG_FIELD_MASK(CRF_APB, VPLL_CTRL, BYPASS),
												HW_REG_ENCODE_FIELD(CRF_APB, VPLL_CTRL, BYPASS, 0)),
		PSI_WRITE_32(CRF_APB, VPLL_TO_LPD_CTRL,
								 HW_REG_ENCODE_FIELD(CRF_APB, VPLL_TO_LPD_CTRL, DIVISOR0,0x3)), // VPLL_TO_LPD = 500 Mhz

		PSI_END_PROGRAM
};
void pllRunInitProgram() {
	psi_RunRegisterProgram(pll_init);
}

#if 0
unsigned long psu_pll_init_data(void)
{
	/*
	* RPLL INIT
	*/
	/*
	* Register : RPLL_CFG @ 0XFF5E0034

	* PLL loop filter resistor control
	*  PSU_CRL_APB_RPLL_CFG_RES                                    0x2

	* PLL charge pump control
	*  PSU_CRL_APB_RPLL_CFG_CP                                     0x3

	* PLL loop filter high frequency capacitor control
	*  PSU_CRL_APB_RPLL_CFG_LFHF                                   0x3

	* Lock circuit counter setting
	*  PSU_CRL_APB_RPLL_CFG_LOCK_CNT                               0x258

	* Lock circuit configuration settings for lock windowsize
	*  PSU_CRL_APB_RPLL_CFG_LOCK_DLY                               0x3f

	* Helper data. Values are to be looked up in a table from Data Sheet
	* (OFFSET, MASK, VALUE)      (0XFF5E0034, 0xFE7FEDEFU ,0x7E4B0C62U)
	*/
	PSU_Mask_Write(CRL_APB_RPLL_CFG_OFFSET, 0xFE7FEDEFU, 0x7E4B0C62U);
	/*##################################################################### */

	/*
	* UPDATE FB_DIV
	*/
	/*
	* Register : RPLL_CTRL @ 0XFF5E0030

	* Mux select for determining which clock feeds this PLL. 0XX pss_ref_clk i
	* s the source 100 video clk is the source 101 pss_alt_ref_clk is the sour
	* ce 110 aux_refclk[X] is the source 111 gt_crx_ref_clk is the source
	*  PSU_CRL_APB_RPLL_CTRL_PRE_SRC                               0x0

	* The integer portion of the feedback divider to the PLL
	*  PSU_CRL_APB_RPLL_CTRL_FBDIV                                 0x40

	* This turns on the divide by 2 that is inside of the PLL. This does not c
	* hange the VCO frequency, just the output frequency
	*  PSU_CRL_APB_RPLL_CTRL_DIV2                                  0x1

	* PLL Basic Control
	* (OFFSET, MASK, VALUE)      (0XFF5E0030, 0x00717F00U ,0x00014000U)
	*/
	PSU_Mask_Write(CRL_APB_RPLL_CTRL_OFFSET, 0x00717F00U, 0x00014000U);
	/*##################################################################### */

	/*
	* BY PASS PLL
	*/
	/*
	* Register : RPLL_CTRL @ 0XFF5E0030

	* Bypasses the PLL clock. The usable clock will be determined from the POS
	* T_SRC field. (This signal may only be toggled after 4 cycles of the old
	* clock and 4 cycles of the new clock. This is not usually an issue, but d
	* esigners must be aware.)
	*  PSU_CRL_APB_RPLL_CTRL_BYPASS                                1

	* PLL Basic Control
	* (OFFSET, MASK, VALUE)      (0XFF5E0030, 0x00000008U ,0x00000008U)
	*/
	PSU_Mask_Write(CRL_APB_RPLL_CTRL_OFFSET, 0x00000008U, 0x00000008U);
	/*##################################################################### */

	/*
	* ASSERT RESET
	*/
	/*
	* Register : RPLL_CTRL @ 0XFF5E0030

	* Asserts Reset to the PLL. When asserting reset, the PLL must already be
	* in BYPASS.
	*  PSU_CRL_APB_RPLL_CTRL_RESET                                 1

	* PLL Basic Control
	* (OFFSET, MASK, VALUE)      (0XFF5E0030, 0x00000001U ,0x00000001U)
	*/
	PSU_Mask_Write(CRL_APB_RPLL_CTRL_OFFSET, 0x00000001U, 0x00000001U);
	/*##################################################################### */

	/*
	* DEASSERT RESET
	*/
	/*
	* Register : RPLL_CTRL @ 0XFF5E0030

	* Asserts Reset to the PLL. When asserting reset, the PLL must already be
	* in BYPASS.
	*  PSU_CRL_APB_RPLL_CTRL_RESET                                 0

	* PLL Basic Control
	* (OFFSET, MASK, VALUE)      (0XFF5E0030, 0x00000001U ,0x00000000U)
	*/
	PSU_Mask_Write(CRL_APB_RPLL_CTRL_OFFSET, 0x00000001U, 0x00000000U);
	/*##################################################################### */

	/*
	* CHECK PLL STATUS
	*/
	/*
	* Register : PLL_STATUS @ 0XFF5E0040

	* RPLL is locked
	*  PSU_CRL_APB_PLL_STATUS_RPLL_LOCK                            1
	* (OFFSET, MASK, VALUE)      (0XFF5E0040, 0x00000002U ,0x00000002U)
	*/
	mask_poll(CRL_APB_PLL_STATUS_OFFSET, 0x00000002U);

	/*##################################################################### */

	/*
	* REMOVE PLL BY PASS
	*/
	/*
	* Register : RPLL_CTRL @ 0XFF5E0030

	* Bypasses the PLL clock. The usable clock will be determined from the POS
	* T_SRC field. (This signal may only be toggled after 4 cycles of the old
	* clock and 4 cycles of the new clock. This is not usually an issue, but d
	* esigners must be aware.)
	*  PSU_CRL_APB_RPLL_CTRL_BYPASS                                0

	* PLL Basic Control
	* (OFFSET, MASK, VALUE)      (0XFF5E0030, 0x00000008U ,0x00000000U)
	*/
	PSU_Mask_Write(CRL_APB_RPLL_CTRL_OFFSET, 0x00000008U, 0x00000000U);
	/*##################################################################### */

	/*
	* Register : RPLL_TO_FPD_CTRL @ 0XFF5E0048

	* Divisor value for this clock.
	*  PSU_CRL_APB_RPLL_TO_FPD_CTRL_DIVISOR0                       0x2

	* Control for a clock that will be generated in the LPD, but used in the F
	* PD as a clock source for the peripheral clock muxes.
	* (OFFSET, MASK, VALUE)      (0XFF5E0048, 0x00003F00U ,0x00000200U)
	*/
	PSU_Mask_Write(CRL_APB_RPLL_TO_FPD_CTRL_OFFSET,
								 0x00003F00U, 0x00000200U);
	/*##################################################################### */

	/*
	* RPLL FRAC CFG
	*/
	/*
	* SYSMON CLOCK PRESET TO RPLL AGAIN TO AVOID GLITCH WHEN NEXT IOPLL WILL B
	* E PUT IN BYPASS MODE
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
	*  PSU_CRL_APB_AMS_REF_CTRL_SRCSEL                             0

	* Clock active signal. Switch to 0 to disable the clock
	*  PSU_CRL_APB_AMS_REF_CTRL_CLKACT                             1

	* This register controls this reference clock
	* (OFFSET, MASK, VALUE)      (0XFF5E0108, 0x013F3F07U ,0x01012300U)
	*/
	PSU_Mask_Write(CRL_APB_AMS_REF_CTRL_OFFSET,
								 0x013F3F07U, 0x01012300U);
	/*##################################################################### */

	/*
	* IOPLL INIT
	*/
	/*
	* Register : IOPLL_CFG @ 0XFF5E0024

	* PLL loop filter resistor control
	*  PSU_CRL_APB_IOPLL_CFG_RES                                   0x2

	* PLL charge pump control
	*  PSU_CRL_APB_IOPLL_CFG_CP                                    0x4

	* PLL loop filter high frequency capacitor control
	*  PSU_CRL_APB_IOPLL_CFG_LFHF                                  0x3

	* Lock circuit counter setting
	*  PSU_CRL_APB_IOPLL_CFG_LOCK_CNT                              0x258

	* Lock circuit configuration settings for lock windowsize
	*  PSU_CRL_APB_IOPLL_CFG_LOCK_DLY                              0x3f

	* Helper data. Values are to be looked up in a table from Data Sheet
	* (OFFSET, MASK, VALUE)      (0XFF5E0024, 0xFE7FEDEFU ,0x7E4B0C82U)
	*/
	PSU_Mask_Write(CRL_APB_IOPLL_CFG_OFFSET, 0xFE7FEDEFU, 0x7E4B0C82U);
	/*##################################################################### */

	/*
	* UPDATE FB_DIV
	*/
	/*
	* Register : IOPLL_CTRL @ 0XFF5E0020

	* Mux select for determining which clock feeds this PLL. 0XX pss_ref_clk i
	* s the source 100 video clk is the source 101 pss_alt_ref_clk is the sour
	* ce 110 aux_refclk[X] is the source 111 gt_crx_ref_clk is the source
	*  PSU_CRL_APB_IOPLL_CTRL_PRE_SRC                              0x0

	* The integer portion of the feedback divider to the PLL
	*  PSU_CRL_APB_IOPLL_CTRL_FBDIV                                0x5a

	* This turns on the divide by 2 that is inside of the PLL. This does not c
	* hange the VCO frequency, just the output frequency
	*  PSU_CRL_APB_IOPLL_CTRL_DIV2                                 0x1

	* PLL Basic Control
	* (OFFSET, MASK, VALUE)      (0XFF5E0020, 0x00717F00U ,0x00015A00U)
	*/
	PSU_Mask_Write(CRL_APB_IOPLL_CTRL_OFFSET, 0x00717F00U, 0x00015A00U);
	/*##################################################################### */

	/*
	* BY PASS PLL
	*/
	/*
	* Register : IOPLL_CTRL @ 0XFF5E0020

	* Bypasses the PLL clock. The usable clock will be determined from the POS
	* T_SRC field. (This signal may only be toggled after 4 cycles of the old
	* clock and 4 cycles of the new clock. This is not usually an issue, but d
	* esigners must be aware.)
	*  PSU_CRL_APB_IOPLL_CTRL_BYPASS                               1

	* PLL Basic Control
	* (OFFSET, MASK, VALUE)      (0XFF5E0020, 0x00000008U ,0x00000008U)
	*/
	PSU_Mask_Write(CRL_APB_IOPLL_CTRL_OFFSET, 0x00000008U, 0x00000008U);
	/*##################################################################### */

	/*
	* ASSERT RESET
	*/
	/*
	* Register : IOPLL_CTRL @ 0XFF5E0020

	* Asserts Reset to the PLL. When asserting reset, the PLL must already be
	* in BYPASS.
	*  PSU_CRL_APB_IOPLL_CTRL_RESET                                1

	* PLL Basic Control
	* (OFFSET, MASK, VALUE)      (0XFF5E0020, 0x00000001U ,0x00000001U)
	*/
	PSU_Mask_Write(CRL_APB_IOPLL_CTRL_OFFSET, 0x00000001U, 0x00000001U);
	/*##################################################################### */

	/*
	* DEASSERT RESET
	*/
	/*
	* Register : IOPLL_CTRL @ 0XFF5E0020

	* Asserts Reset to the PLL. When asserting reset, the PLL must already be
	* in BYPASS.
	*  PSU_CRL_APB_IOPLL_CTRL_RESET                                0

	* PLL Basic Control
	* (OFFSET, MASK, VALUE)      (0XFF5E0020, 0x00000001U ,0x00000000U)
	*/
	PSU_Mask_Write(CRL_APB_IOPLL_CTRL_OFFSET, 0x00000001U, 0x00000000U);
	/*##################################################################### */

	/*
	* CHECK PLL STATUS
	*/
	/*
	* Register : PLL_STATUS @ 0XFF5E0040

	* IOPLL is locked
	*  PSU_CRL_APB_PLL_STATUS_IOPLL_LOCK                           1
	* (OFFSET, MASK, VALUE)      (0XFF5E0040, 0x00000001U ,0x00000001U)
	*/
	mask_poll(CRL_APB_PLL_STATUS_OFFSET, 0x00000001U);

	/*##################################################################### */

	/*
	* REMOVE PLL BY PASS
	*/
	/*
	* Register : IOPLL_CTRL @ 0XFF5E0020

	* Bypasses the PLL clock. The usable clock will be determined from the POS
	* T_SRC field. (This signal may only be toggled after 4 cycles of the old
	* clock and 4 cycles of the new clock. This is not usually an issue, but d
	* esigners must be aware.)
	*  PSU_CRL_APB_IOPLL_CTRL_BYPASS                               0

	* PLL Basic Control
	* (OFFSET, MASK, VALUE)      (0XFF5E0020, 0x00000008U ,0x00000000U)
	*/
	PSU_Mask_Write(CRL_APB_IOPLL_CTRL_OFFSET, 0x00000008U, 0x00000000U);
	/*##################################################################### */

	/*
	* Register : IOPLL_TO_FPD_CTRL @ 0XFF5E0044

	* Divisor value for this clock.
	*  PSU_CRL_APB_IOPLL_TO_FPD_CTRL_DIVISOR0                      0x3

	* Control for a clock that will be generated in the LPD, but used in the F
	* PD as a clock source for the peripheral clock muxes.
	* (OFFSET, MASK, VALUE)      (0XFF5E0044, 0x00003F00U ,0x00000300U)
	*/
	PSU_Mask_Write(CRL_APB_IOPLL_TO_FPD_CTRL_OFFSET,
								 0x00003F00U, 0x00000300U);
	/*##################################################################### */

	/*
	* IOPLL FRAC CFG
	*/
	/*
	* APU_PLL INIT
	*/
	/*
	* Register : APLL_CFG @ 0XFD1A0024

	* PLL loop filter resistor control
	*  PSU_CRF_APB_APLL_CFG_RES                                    0x2

	* PLL charge pump control
	*  PSU_CRF_APB_APLL_CFG_CP                                     0x3

	* PLL loop filter high frequency capacitor control
	*  PSU_CRF_APB_APLL_CFG_LFHF                                   0x3

	* Lock circuit counter setting
	*  PSU_CRF_APB_APLL_CFG_LOCK_CNT                               0x258

	* Lock circuit configuration settings for lock windowsize
	*  PSU_CRF_APB_APLL_CFG_LOCK_DLY                               0x3f

	* Helper data. Values are to be looked up in a table from Data Sheet
	* (OFFSET, MASK, VALUE)      (0XFD1A0024, 0xFE7FEDEFU ,0x7E4B0C62U)
	*/
	PSU_Mask_Write(CRF_APB_APLL_CFG_OFFSET, 0xFE7FEDEFU, 0x7E4B0C62U);
	/*##################################################################### */

	/*
	* UPDATE FB_DIV
	*/
	/*
	* Register : APLL_CTRL @ 0XFD1A0020

	* Mux select for determining which clock feeds this PLL. 0XX pss_ref_clk i
	* s the source 100 video clk is the source 101 pss_alt_ref_clk is the sour
	* ce 110 aux_refclk[X] is the source 111 gt_crx_ref_clk is the source
	*  PSU_CRF_APB_APLL_CTRL_PRE_SRC                               0x0

	* The integer portion of the feedback divider to the PLL
	*  PSU_CRF_APB_APLL_CTRL_FBDIV                                 0x50

	* This turns on the divide by 2 that is inside of the PLL. This does not c
	* hange the VCO frequency, just the output frequency
	*  PSU_CRF_APB_APLL_CTRL_DIV2                                  0x1

	* PLL Basic Control
	* (OFFSET, MASK, VALUE)      (0XFD1A0020, 0x00717F00U ,0x00015000U)
	*/
	PSU_Mask_Write(CRF_APB_APLL_CTRL_OFFSET, 0x00717F00U, 0x00015000U);
	/*##################################################################### */

	/*
	* BY PASS PLL
	*/
	/*
	* Register : APLL_CTRL @ 0XFD1A0020

	* Bypasses the PLL clock. The usable clock will be determined from the POS
	* T_SRC field. (This signal may only be toggled after 4 cycles of the old
	* clock and 4 cycles of the new clock. This is not usually an issue, but d
	* esigners must be aware.)
	*  PSU_CRF_APB_APLL_CTRL_BYPASS                                1

	* PLL Basic Control
	* (OFFSET, MASK, VALUE)      (0XFD1A0020, 0x00000008U ,0x00000008U)
	*/
	PSU_Mask_Write(CRF_APB_APLL_CTRL_OFFSET, 0x00000008U, 0x00000008U);
	/*##################################################################### */

	/*
	* ASSERT RESET
	*/
	/*
	* Register : APLL_CTRL @ 0XFD1A0020

	* Asserts Reset to the PLL. When asserting reset, the PLL must already be
	* in BYPASS.
	*  PSU_CRF_APB_APLL_CTRL_RESET                                 1

	* PLL Basic Control
	* (OFFSET, MASK, VALUE)      (0XFD1A0020, 0x00000001U ,0x00000001U)
	*/
	PSU_Mask_Write(CRF_APB_APLL_CTRL_OFFSET, 0x00000001U, 0x00000001U);
	/*##################################################################### */

	/*
	* DEASSERT RESET
	*/
	/*
	* Register : APLL_CTRL @ 0XFD1A0020

	* Asserts Reset to the PLL. When asserting reset, the PLL must already be
	* in BYPASS.
	*  PSU_CRF_APB_APLL_CTRL_RESET                                 0

	* PLL Basic Control
	* (OFFSET, MASK, VALUE)      (0XFD1A0020, 0x00000001U ,0x00000000U)
	*/
	PSU_Mask_Write(CRF_APB_APLL_CTRL_OFFSET, 0x00000001U, 0x00000000U);
	/*##################################################################### */

	/*
	* CHECK PLL STATUS
	*/
	/*
	* Register : PLL_STATUS @ 0XFD1A0044

	* APLL is locked
	*  PSU_CRF_APB_PLL_STATUS_APLL_LOCK                            1
	* (OFFSET, MASK, VALUE)      (0XFD1A0044, 0x00000001U ,0x00000001U)
	*/
	mask_poll(CRF_APB_PLL_STATUS_OFFSET, 0x00000001U);

	/*##################################################################### */

	/*
	* REMOVE PLL BY PASS
	*/
	/*
	* Register : APLL_CTRL @ 0XFD1A0020

	* Bypasses the PLL clock. The usable clock will be determined from the POS
	* T_SRC field. (This signal may only be toggled after 4 cycles of the old
	* clock and 4 cycles of the new clock. This is not usually an issue, but d
	* esigners must be aware.)
	*  PSU_CRF_APB_APLL_CTRL_BYPASS                                0

	* PLL Basic Control
	* (OFFSET, MASK, VALUE)      (0XFD1A0020, 0x00000008U ,0x00000000U)
	*/
	PSU_Mask_Write(CRF_APB_APLL_CTRL_OFFSET, 0x00000008U, 0x00000000U);
	/*##################################################################### */

	/*
	* Register : APLL_TO_LPD_CTRL @ 0XFD1A0048

	* Divisor value for this clock.
	*  PSU_CRF_APB_APLL_TO_LPD_CTRL_DIVISOR0                       0x3

	* Control for a clock that will be generated in the FPD, but used in the L
	* PD as a clock source for the peripheral clock muxes.
	* (OFFSET, MASK, VALUE)      (0XFD1A0048, 0x00003F00U ,0x00000300U)
	*/
	PSU_Mask_Write(CRF_APB_APLL_TO_LPD_CTRL_OFFSET,
								 0x00003F00U, 0x00000300U);
	/*##################################################################### */

	/*
	* APLL FRAC CFG
	*/
	/*
	* Register : APLL_FRAC_CFG @ 0XFD1A0028

	* Fractional SDM bypass control. When 0, PLL is in integer mode and it ign
	* ores all fractional data. When 1, PLL is in fractional mode and uses DAT
	* A of this register for the fractional portion of the feedback divider.
	*  PSU_CRF_APB_APLL_FRAC_CFG_ENABLED                           0x1

	* Fractional value for the Feedback value.
	*  PSU_CRF_APB_APLL_FRAC_CFG_DATA                              0x33

	* Fractional control for the PLL
	* (OFFSET, MASK, VALUE)      (0XFD1A0028, 0x8000FFFFU ,0x80000033U)
	*/
	PSU_Mask_Write(CRF_APB_APLL_FRAC_CFG_OFFSET,
								 0x8000FFFFU, 0x80000033U);
	/*##################################################################### */

	/*
	* DDR_PLL INIT
	*/
	/*
	* Register : DPLL_CFG @ 0XFD1A0030

	* PLL loop filter resistor control
	*  PSU_CRF_APB_DPLL_CFG_RES                                    0x2

	* PLL charge pump control
	*  PSU_CRF_APB_DPLL_CFG_CP                                     0x3

	* PLL loop filter high frequency capacitor control
	*  PSU_CRF_APB_DPLL_CFG_LFHF                                   0x3

	* Lock circuit counter setting
	*  PSU_CRF_APB_DPLL_CFG_LOCK_CNT                               0x258

	* Lock circuit configuration settings for lock windowsize
	*  PSU_CRF_APB_DPLL_CFG_LOCK_DLY                               0x3f

	* Helper data. Values are to be looked up in a table from Data Sheet
	* (OFFSET, MASK, VALUE)      (0XFD1A0030, 0xFE7FEDEFU ,0x7E4B0C62U)
	*/
	PSU_Mask_Write(CRF_APB_DPLL_CFG_OFFSET, 0xFE7FEDEFU, 0x7E4B0C62U);
	/*##################################################################### */

	/*
	* UPDATE FB_DIV
	*/
	/*
	* Register : DPLL_CTRL @ 0XFD1A002C

	* Mux select for determining which clock feeds this PLL. 0XX pss_ref_clk i
	* s the source 100 video clk is the source 101 pss_alt_ref_clk is the sour
	* ce 110 aux_refclk[X] is the source 111 gt_crx_ref_clk is the source
	*  PSU_CRF_APB_DPLL_CTRL_PRE_SRC                               0x0

	* The integer portion of the feedback divider to the PLL
	*  PSU_CRF_APB_DPLL_CTRL_FBDIV                                 0x40

	* This turns on the divide by 2 that is inside of the PLL. This does not c
	* hange the VCO frequency, just the output frequency
	*  PSU_CRF_APB_DPLL_CTRL_DIV2                                  0x1

	* PLL Basic Control
	* (OFFSET, MASK, VALUE)      (0XFD1A002C, 0x00717F00U ,0x00014000U)
	*/
	PSU_Mask_Write(CRF_APB_DPLL_CTRL_OFFSET, 0x00717F00U, 0x00014000U);
	/*##################################################################### */

	/*
	* BY PASS PLL
	*/
	/*
	* Register : DPLL_CTRL @ 0XFD1A002C

	* Bypasses the PLL clock. The usable clock will be determined from the POS
	* T_SRC field. (This signal may only be toggled after 4 cycles of the old
	* clock and 4 cycles of the new clock. This is not usually an issue, but d
	* esigners must be aware.)
	*  PSU_CRF_APB_DPLL_CTRL_BYPASS                                1

	* PLL Basic Control
	* (OFFSET, MASK, VALUE)      (0XFD1A002C, 0x00000008U ,0x00000008U)
	*/
	PSU_Mask_Write(CRF_APB_DPLL_CTRL_OFFSET, 0x00000008U, 0x00000008U);
	/*##################################################################### */

	/*
	* ASSERT RESET
	*/
	/*
	* Register : DPLL_CTRL @ 0XFD1A002C

	* Asserts Reset to the PLL. When asserting reset, the PLL must already be
	* in BYPASS.
	*  PSU_CRF_APB_DPLL_CTRL_RESET                                 1

	* PLL Basic Control
	* (OFFSET, MASK, VALUE)      (0XFD1A002C, 0x00000001U ,0x00000001U)
	*/
	PSU_Mask_Write(CRF_APB_DPLL_CTRL_OFFSET, 0x00000001U, 0x00000001U);
	/*##################################################################### */

	/*
	* DEASSERT RESET
	*/
	/*
	* Register : DPLL_CTRL @ 0XFD1A002C

	* Asserts Reset to the PLL. When asserting reset, the PLL must already be
	* in BYPASS.
	*  PSU_CRF_APB_DPLL_CTRL_RESET                                 0

	* PLL Basic Control
	* (OFFSET, MASK, VALUE)      (0XFD1A002C, 0x00000001U ,0x00000000U)
	*/
	PSU_Mask_Write(CRF_APB_DPLL_CTRL_OFFSET, 0x00000001U, 0x00000000U);
	/*##################################################################### */

	/*
	* CHECK PLL STATUS
	*/
	/*
	* Register : PLL_STATUS @ 0XFD1A0044

	* DPLL is locked
	*  PSU_CRF_APB_PLL_STATUS_DPLL_LOCK                            1
	* (OFFSET, MASK, VALUE)      (0XFD1A0044, 0x00000002U ,0x00000002U)
	*/
	mask_poll(CRF_APB_PLL_STATUS_OFFSET, 0x00000002U);

	/*##################################################################### */

	/*
	* REMOVE PLL BY PASS
	*/
	/*
	* Register : DPLL_CTRL @ 0XFD1A002C

	* Bypasses the PLL clock. The usable clock will be determined from the POS
	* T_SRC field. (This signal may only be toggled after 4 cycles of the old
	* clock and 4 cycles of the new clock. This is not usually an issue, but d
	* esigners must be aware.)
	*  PSU_CRF_APB_DPLL_CTRL_BYPASS                                0

	* PLL Basic Control
	* (OFFSET, MASK, VALUE)      (0XFD1A002C, 0x00000008U ,0x00000000U)
	*/
	PSU_Mask_Write(CRF_APB_DPLL_CTRL_OFFSET, 0x00000008U, 0x00000000U);
	/*##################################################################### */

	/*
	* Register : DPLL_TO_LPD_CTRL @ 0XFD1A004C

	* Divisor value for this clock.
	*  PSU_CRF_APB_DPLL_TO_LPD_CTRL_DIVISOR0                       0x2

	* Control for a clock that will be generated in the FPD, but used in the L
	* PD as a clock source for the peripheral clock muxes.
	* (OFFSET, MASK, VALUE)      (0XFD1A004C, 0x00003F00U ,0x00000200U)
	*/
	PSU_Mask_Write(CRF_APB_DPLL_TO_LPD_CTRL_OFFSET,
								 0x00003F00U, 0x00000200U);
	/*##################################################################### */

	/*
	* DPLL FRAC CFG
	*/
	/*
	* VIDEO_PLL INIT
	*/
	/*
	* Register : VPLL_CFG @ 0XFD1A003C

	* PLL loop filter resistor control
	*  PSU_CRF_APB_VPLL_CFG_RES                                    0x2

	* PLL charge pump control
	*  PSU_CRF_APB_VPLL_CFG_CP                                     0x4

	* PLL loop filter high frequency capacitor control
	*  PSU_CRF_APB_VPLL_CFG_LFHF                                   0x3

	* Lock circuit counter setting
	*  PSU_CRF_APB_VPLL_CFG_LOCK_CNT                               0x258

	* Lock circuit configuration settings for lock windowsize
	*  PSU_CRF_APB_VPLL_CFG_LOCK_DLY                               0x3f

	* Helper data. Values are to be looked up in a table from Data Sheet
	* (OFFSET, MASK, VALUE)      (0XFD1A003C, 0xFE7FEDEFU ,0x7E4B0C82U)
	*/
	PSU_Mask_Write(CRF_APB_VPLL_CFG_OFFSET, 0xFE7FEDEFU, 0x7E4B0C82U);
	/*##################################################################### */

	/*
	* UPDATE FB_DIV
	*/
	/*
	* Register : VPLL_CTRL @ 0XFD1A0038

	* Mux select for determining which clock feeds this PLL. 0XX pss_ref_clk i
	* s the source 100 video clk is the source 101 pss_alt_ref_clk is the sour
	* ce 110 aux_refclk[X] is the source 111 gt_crx_ref_clk is the source
	*  PSU_CRF_APB_VPLL_CTRL_PRE_SRC                               0x0

	* The integer portion of the feedback divider to the PLL
	*  PSU_CRF_APB_VPLL_CTRL_FBDIV                                 0x5a

	* This turns on the divide by 2 that is inside of the PLL. This does not c
	* hange the VCO frequency, just the output frequency
	*  PSU_CRF_APB_VPLL_CTRL_DIV2                                  0x1

	* PLL Basic Control
	* (OFFSET, MASK, VALUE)      (0XFD1A0038, 0x00717F00U ,0x00015A00U)
	*/
	PSU_Mask_Write(CRF_APB_VPLL_CTRL_OFFSET, 0x00717F00U, 0x00015A00U);
	/*##################################################################### */

	/*
	* BY PASS PLL
	*/
	/*
	* Register : VPLL_CTRL @ 0XFD1A0038

	* Bypasses the PLL clock. The usable clock will be determined from the POS
	* T_SRC field. (This signal may only be toggled after 4 cycles of the old
	* clock and 4 cycles of the new clock. This is not usually an issue, but d
	* esigners must be aware.)
	*  PSU_CRF_APB_VPLL_CTRL_BYPASS                                1

	* PLL Basic Control
	* (OFFSET, MASK, VALUE)      (0XFD1A0038, 0x00000008U ,0x00000008U)
	*/
	PSU_Mask_Write(CRF_APB_VPLL_CTRL_OFFSET, 0x00000008U, 0x00000008U);
	/*##################################################################### */

	/*
	* ASSERT RESET
	*/
	/*
	* Register : VPLL_CTRL @ 0XFD1A0038

	* Asserts Reset to the PLL. When asserting reset, the PLL must already be
	* in BYPASS.
	*  PSU_CRF_APB_VPLL_CTRL_RESET                                 1

	* PLL Basic Control
	* (OFFSET, MASK, VALUE)      (0XFD1A0038, 0x00000001U ,0x00000001U)
	*/
	PSU_Mask_Write(CRF_APB_VPLL_CTRL_OFFSET, 0x00000001U, 0x00000001U);
	/*##################################################################### */

	/*
	* DEASSERT RESET
	*/
	/*
	* Register : VPLL_CTRL @ 0XFD1A0038

	* Asserts Reset to the PLL. When asserting reset, the PLL must already be
	* in BYPASS.
	*  PSU_CRF_APB_VPLL_CTRL_RESET                                 0

	* PLL Basic Control
	* (OFFSET, MASK, VALUE)      (0XFD1A0038, 0x00000001U ,0x00000000U)
	*/
	PSU_Mask_Write(CRF_APB_VPLL_CTRL_OFFSET, 0x00000001U, 0x00000000U);
	/*##################################################################### */

	/*
	* CHECK PLL STATUS
	*/
	/*
	* Register : PLL_STATUS @ 0XFD1A0044

	* VPLL is locked
	*  PSU_CRF_APB_PLL_STATUS_VPLL_LOCK                            1
	* (OFFSET, MASK, VALUE)      (0XFD1A0044, 0x00000004U ,0x00000004U)
	*/
	mask_poll(CRF_APB_PLL_STATUS_OFFSET, 0x00000004U);

	/*##################################################################### */

	/*
	* REMOVE PLL BY PASS
	*/
	/*
	* Register : VPLL_CTRL @ 0XFD1A0038

	* Bypasses the PLL clock. The usable clock will be determined from the POS
	* T_SRC field. (This signal may only be toggled after 4 cycles of the old
	* clock and 4 cycles of the new clock. This is not usually an issue, but d
	* esigners must be aware.)
	*  PSU_CRF_APB_VPLL_CTRL_BYPASS                                0

	* PLL Basic Control
	* (OFFSET, MASK, VALUE)      (0XFD1A0038, 0x00000008U ,0x00000000U)
	*/
	PSU_Mask_Write(CRF_APB_VPLL_CTRL_OFFSET, 0x00000008U, 0x00000000U);
	/*##################################################################### */

	/*
	* Register : VPLL_TO_LPD_CTRL @ 0XFD1A0050

	* Divisor value for this clock.
	*  PSU_CRF_APB_VPLL_TO_LPD_CTRL_DIVISOR0                       0x3

	* Control for a clock that will be generated in the FPD, but used in the L
	* PD as a clock source for the peripheral clock muxes.
	* (OFFSET, MASK, VALUE)      (0XFD1A0050, 0x00003F00U ,0x00000300U)
	*/
	PSU_Mask_Write(CRF_APB_VPLL_TO_LPD_CTRL_OFFSET,
								 0x00003F00U, 0x00000300U);
	/*##################################################################### */

	/*
	* VIDEO FRAC CFG
	*/

	return 1;
}
#endif // if 0
