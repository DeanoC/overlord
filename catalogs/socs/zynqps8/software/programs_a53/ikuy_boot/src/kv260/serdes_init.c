#include "core/core.h"
#include "dbg/raw_print.h"
#include "dbg/print.h"
#include "platform/reg_access.h"
#include "utils/boot_psi.h"
#include "platform/memory_map.h"
#include "platform/registers/crl_apb.h"
#include "platform/registers/crf_apb.h"
#include "platform/registers/serdes.h"
#include "platform/registers/dp.h"
#include "platform/registers/usb3_regs.h"
#include "platform/registers/usb3_xhci.h"

#define MASK_POLL_TIME 1100000

//Kishore -- ILL calibration code ends
/*Following SERDES programming sequences that a user need to follow to work
 * around the known limitation with SERDES. These sequences should done
 * before STEP 1 and STEP 2 as described in previous section. These
 * programming steps are *required for current silicon version and are
 * likely to undergo further changes with subsequent silicon versions.
 */
void serdesFixcal(void)
{
	uint32_t rdata = 0;

	// Each element of array stands for count of occurrence of valid code.

	// The valid codes are from 0x26 to 0x3C. There are 23 valid codes in total.
	uint32_t match_pmos_code[23] = { 0 };
	uint32_t match_nmos_code[23] = { 0 };
	// The valid codes are from 0xC to 0x12. There are 7 valid codes in total.
	uint32_t match_ical_code[7] = { 0 };
	// The valid codes are from 0x6 to 0xC. There are 7 valid codes in total.
	uint32_t match_rcal_code[7] = { 0 };

	uint32_t p_code = 0;
	uint32_t n_code = 0;
	uint32_t i_code = 0;
	uint32_t r_code = 0;
	uint32_t repeat_count = 0;
	int i = 0;

	HW_REG_RMW1(SERDES, UNKNOWN0, 0x3, 0x1);

	// check supply good status before starting AFE sequencing
	int count = 0;
	do
	{
		if (count == MASK_POLL_TIME )
			break;
		rdata = HW_REG_READ1(SERDES, POWER_GOOD);
		count++;
	}while((rdata&0x0000000E) !=0x0000000E);

	do {
		//Clear ICM_CFG value
		HW_REG_WRITE1(SERDES, ICM_CFG0, 0x00000000);
		HW_REG_WRITE1(SERDES, ICM_CFG1, 0x00000000);

		//Set ICM_CFG value
		//This will trigger re-calibration of all stages
		HW_REG_WRITE1(SERDES, ICM_CFG0, 0x00000001);
		HW_REG_WRITE1(SERDES, ICM_CFG1, 0x00000000);

		// is calibration done? polling on L3_CALIB_DONE_STATUS
		count = 0;
		do {
			if (count == MASK_POLL_TIME ) {
				debug_printf("SERDES initialization timed out\n\r");
				return;
			}
			rdata = HW_REG_READ1(SERDES, L3_CALIB_DONE_STATUS) & 0x2;
			count++;
		} while( !rdata );

		p_code = HW_REG_READ1(SERDES, PMOS_CODE);
		n_code = HW_REG_READ1(SERDES, NMOS_CODE);
		//m_code = HW_REG_READ1(SERDES, MPHY_CODE);
		i_code = HW_REG_READ1(SERDES, ICAL_CODE);
		r_code = HW_REG_READ1(SERDES, RX_CODE);
		//u_code = HW_REG_READ1(SERDES, USB2_CODE);

		// PMOS code in acceptable range
		if ((p_code >= 0x26) && (p_code <= 0x3C))
			match_pmos_code[p_code - 0x26] += 1;

		// NMOS code in acceptable range
		if ((n_code >= 0x26) && (n_code <= 0x3C))
			match_nmos_code[n_code - 0x26] += 1;

		// ical code in acceptable range
		if ((i_code >= 0xC) && (i_code <= 0x12))
			match_ical_code[i_code - 0xC] += 1;

		// rcal code in acceptable range
		if ((r_code >= 0x6) && (r_code <= 0xC))
			match_rcal_code[r_code - 0x6] += 1;

	} while (repeat_count++ < 10);

	// find the valid code which resulted in maximum times in 10 iterations
	for (i = 0; i < 23; i++) {
		if (match_pmos_code[i] >= match_pmos_code[0]) {
			match_pmos_code[0] = match_pmos_code[i];
			p_code = 0x26 + i;
		}
		if (match_nmos_code[i] >= match_nmos_code[0]) {
			match_nmos_code[0] = match_nmos_code[i];
			n_code = 0x26 + i;
		}
	}

	for (i = 0; i < 7; i++) {
		if (match_ical_code[i] >= match_ical_code[0]) {
			match_ical_code[0] = match_ical_code[i];
			i_code = 0xC + i;
		}
		if (match_rcal_code[i] >= match_rcal_code[0]) {
			match_rcal_code[0] = match_rcal_code[i];
			r_code = 0x6 + i;
		}
	}

	// L3_TM_CALIB_DIG20[3] PSW MSB Override
	// L3_TM_CALIB_DIG20[2:0]	PSW Code [4:2]
	HW_REG_RMW1(SERDES, L3_TM_CALIB_DIG20, 0xF, (0x8 | ((p_code >> 2) & 0x7)) );

	// L3_TM_CALIB_DIG19[7:6]	PSW Code [1:0]
	// L3_TM_CALIB_DIG19[5]	PSW Override
	// L3_TM_CALIB_DIG19[2]	NSW MSB Override
	// L3_TM_CALIB_DIG19[1:0]	NSW Code [4:3]
	HW_REG_RMW1(SERDES, L3_TM_CALIB_DIG19, 0xE7, ((p_code & 0x3) << 6) | 0x20 | 0x4 | ((n_code >> 3) & 0x3) );

	// L3_TM_CALIB_DIG18[7:5]	NSW Code [2:0]
	// L3_TM_CALIB_DIG18[4]	NSW Override
	HW_REG_RMW1(SERDES, L3_TM_CALIB_DIG18, 0xF0, (((n_code & 0x7) << 5) | 0x10) );


	// L3_TM_CALIB_DIG16[2:0]	RX Code [3:1]
	HW_REG_RMW1(SERDES, L3_TM_CALIB_DIG16, 0x7, ((r_code >> 1) & 0x7) );

	// L3_TM_CALIB_DIG15[7]	RX Code [0]
	// L3_TM_CALIB_DIG15[6]	RX CODE Override
	// L3_TM_CALIB_DIG15[3]	ICAL MSB Override
	// L3_TM_CALIB_DIG15[2:0]	ICAL Code [3:1]
	HW_REG_RMW1(SERDES, L3_TM_CALIB_DIG15, 0xCF, ((r_code & 0x1) << 7) | 0x40 | 0x8 | ((i_code >> 1) & 0x7) );

	// L3_TM_CALIB_DIG14[7]	ICAL Code [0]
	// L3_TM_CALIB_DIG14[6]	ICAL Override
	HW_REG_RMW1(SERDES, L3_TM_CALIB_DIG14, 0xC0, ((i_code & 0x1) << 7) | 0x40 );

	// Enable PLL Coarse Code saturation Logic
	HW_REG_WRITE1(SERDES, L0_TM_PLL_DIG_37, 0x00000010);
	HW_REG_WRITE1(SERDES, L1_TM_PLL_DIG_37, 0x00000010);
	HW_REG_WRITE1(SERDES, L2_TM_PLL_DIG_37, 0x00000010);
	HW_REG_WRITE1(SERDES, L3_TM_PLL_DIG_37, 0x00000010);
}

__attribute__((__section__(".hwregs")))
static PSI_IWord const resetin_init[] = {
	// Putting USB0 into reset
	PSI_FAR_WRITE_MASKED_32(CRL_APB, RST_LPD_TOP, 0x00000540U, 0x00000540U),
	// Putting GEM0 into reset
	PSI_FAR_WRITE_MASKED_32(CRL_APB, RST_LPD_IOU0, 0x00000008U, 0x00000008U),
	// Putting DP into reset
	PSI_FAR_WRITE_MASKED_32(DP, TX_PHY_POWER_DOWN, 0x0000000FU, 0x0000000AU),
	PSI_FAR_WRITE_MASKED_32(DP, PHY_RESET, 0x00000002U, 0x00000002U),
	PSI_FAR_WRITE_MASKED_32(CRF_APB, RST_FPD_TOP, 0x00010000U, 0x00010000U),

	PSI_END_PROGRAM
};
__attribute__((__section__(".hwregs")))
static PSI_IWord const serdes_init[] = {
	PSI_SET_REGISTER_BANK(SERDES),
	PSI_WRITE_MASKED_32(SERDES, PLL_REF_SEL0, 0x0000001FU, 0x00000009U),
	PSI_WRITE_MASKED_32(SERDES, PLL_REF_SEL1, 0x0000001FU, 0x00000009U),
	PSI_WRITE_MASKED_32(SERDES, PLL_REF_SEL2, 0x0000001FU, 0x00000008U),
	PSI_WRITE_MASKED_32(SERDES, L0_L0_REF_CLK_SEL, 0x00000080U, 0x00000080U),
	PSI_WRITE_MASKED_32(SERDES, L0_L1_REF_CLK_SEL, 0x00000081U, 0x00000001U),
	PSI_WRITE_MASKED_32(SERDES, L0_L2_REF_CLK_SEL, 0x00000082U, 0x00000002U),

	// Enable Spread Spectrum
	PSI_WRITE_MASKED_32(SERDES, L2_TM_PLL_DIG_37, 0x00000010U, 0x00000010U),
	PSI_WRITE_MASKED_32(SERDES, L2_PLL_SS_STEPS_0_LSB, 0x000000FFU, 0x00000038U),
	PSI_WRITE_MASKED_32(SERDES, L2_PLL_SS_STEPS_1_MSB, 0x00000007U, 0x00000003U),
	PSI_WRITE_MASKED_32(SERDES, L0_PLL_SS_STEPS_0_LSB, 0x000000FFU, 0x00000058U),
	PSI_WRITE_MASKED_32(SERDES, L0_PLL_SS_STEPS_1_MSB, 0x00000007U, 0x00000003U),
	PSI_WRITE_MASKED_32(SERDES, L1_PLL_SS_STEPS_0_LSB, 0x000000FFU, 0x00000058U),
	PSI_WRITE_MASKED_32(SERDES, L1_PLL_SS_STEPS_1_MSB, 0x00000007U, 0x00000003U),
	PSI_WRITE_MASKED_32(SERDES, L0_PLL_SS_STEP_SIZE_0_LSB, 0x000000FFU, 0x0000007CU),
	PSI_WRITE_MASKED_32(SERDES, L0_PLL_SS_STEP_SIZE_1, 0x000000FFU, 0x00000033U),
	PSI_WRITE_MASKED_32(SERDES, L0_PLL_SS_STEP_SIZE_2, 0x000000FFU, 0x00000002U),
	PSI_WRITE_MASKED_32(SERDES, L0_PLL_SS_STEP_SIZE_3_MSB, 0x00000033U, 0x00000030U),
	PSI_WRITE_MASKED_32(SERDES, L1_PLL_SS_STEP_SIZE_0_LSB, 0x000000FFU, 0x0000007CU),
	PSI_WRITE_MASKED_32(SERDES, L1_PLL_SS_STEP_SIZE_1, 0x000000FFU, 0x00000033U),
	PSI_WRITE_MASKED_32(SERDES, L1_PLL_SS_STEP_SIZE_2, 0x000000FFU, 0x00000002U),
	PSI_WRITE_MASKED_32(SERDES, L1_PLL_SS_STEP_SIZE_3_MSB, 0x00000033U, 0x00000030U),
	PSI_WRITE_MASKED_32(SERDES, L2_PLL_SS_STEP_SIZE_0_LSB, 0x000000FFU, 0x000000F4U),
	PSI_WRITE_MASKED_32(SERDES, L2_PLL_SS_STEP_SIZE_1, 0x000000FFU, 0x00000031U),
	PSI_WRITE_MASKED_32(SERDES, L2_PLL_SS_STEP_SIZE_2, 0x000000FFU, 0x00000002U),
	PSI_WRITE_MASKED_32(SERDES, L2_PLL_SS_STEP_SIZE_3_MSB, 0x00000033U, 0x00000030U),
	PSI_WRITE_MASKED_32(SERDES, L2_TM_DIG_6, 0x00000003U, 0x00000003U),
	PSI_WRITE_MASKED_32(SERDES, L2_TX_DIG_TM_61, 0x00000003U, 0x00000003U),
	// Enable chicken bit for pcie and usb
	PSI_WRITE_MASKED_32(SERDES, L2_TM_AUX_0, 0x00000020U, 0x00000020U),
	// Enable Eye Surf
	PSI_WRITE_MASKED_32(SERDES, L0_TM_DIG_8, 0x00000010U, 0x00000010U),
	PSI_WRITE_MASKED_32(SERDES, L1_TM_DIG_8, 0x00000010U, 0x00000010U),
	PSI_WRITE_MASKED_32(SERDES, L2_TM_DIG_8, 0x00000010U, 0x00000010U),
	PSI_WRITE_MASKED_32(SERDES, L3_TM_DIG_8, 0x00000010U, 0x00000010U),

	// ILL settings for gain and lock
	PSI_WRITE_MASKED_32(SERDES, L0_TM_ILL13, 0x00000007U, 0x00000007U),
	PSI_WRITE_MASKED_32(SERDES, L1_TM_ILL13, 0x00000007U, 0x00000007U),
	PSI_WRITE_MASKED_32(SERDES, L2_TM_MISC2, 0x00000080U, 0x00000080U),
	PSI_WRITE_MASKED_32(SERDES, L2_TM_IQ_ILL1, 0x000000FFU, 0x0000001AU),
	PSI_WRITE_MASKED_32(SERDES, L2_TM_IQ_ILL2, 0x000000FFU, 0x0000001AU),
	PSI_WRITE_MASKED_32(SERDES, L2_TM_ILL12, 0x000000FFU, 0x00000010U),
	PSI_WRITE_MASKED_32(SERDES, L2_TM_E_ILL1, 0x000000FFU, 0x000000FEU),
	PSI_WRITE_MASKED_32(SERDES, L2_TM_E_ILL2, 0x000000FFU, 0x00000000U),
	PSI_WRITE_MASKED_32(SERDES, L2_TM_IQ_ILL3, 0x000000FFU, 0x0000001AU),
	PSI_WRITE_MASKED_32(SERDES, L2_TM_E_ILL3, 0x000000FFU, 0x00000000U),
	PSI_WRITE_MASKED_32(SERDES, L2_TM_ILL8, 0x000000FFU, 0x000000FFU),
	PSI_WRITE_MASKED_32(SERDES, L2_TM_IQ_ILL8, 0x000000FFU, 0x000000F7U),
	PSI_WRITE_MASKED_32(SERDES, L2_TM_IQ_ILL9, 0x00000001U, 0x00000001U),
	PSI_WRITE_MASKED_32(SERDES, L2_TM_E_ILL8, 0x000000FFU, 0x000000F7U),
	PSI_WRITE_MASKED_32(SERDES, L2_TM_E_ILL9, 0x00000001U, 0x00000001U),
	PSI_WRITE_MASKED_32(SERDES, L2_TM_ILL13, 0x00000007U, 0x00000007U),
	PSI_WRITE_MASKED_32(SERDES, L2_TM_ILL13, 0x00000007U, 0x00000007U),

	// Symbol lock and wait
	PSI_WRITE_MASKED_32(SERDES, L0_TM_DIG_10, 0x0000000FU, 0x00000001U),
	PSI_WRITE_MASKED_32(SERDES, L1_TM_DIG_10, 0x0000000FU, 0x00000001U),
	PSI_WRITE_MASKED_32(SERDES, L2_TM_DIG_10, 0x0000000FU, 0x00000001U),
	PSI_WRITE_MASKED_32(SERDES, L3_TM_DIG_10, 0x0000000FU, 0x00000001U),

	// SIOU setting for bypass control, HSRC-DIG
	PSI_WRITE_MASKED_32(SERDES, L0_TM_RST_DLY, 0x000000FFU, 0x000000FFU),
	PSI_WRITE_MASKED_32(SERDES, L0_TM_ANA_BYP_15, 0x00000040U, 0x00000040U),
	PSI_WRITE_MASKED_32(SERDES, L0_TM_ANA_BYP_12, 0x00000040U, 0x00000040U),
	PSI_WRITE_MASKED_32(SERDES, L1_TM_RST_DLY, 0x000000FFU, 0x000000FFU),
	PSI_WRITE_MASKED_32(SERDES, L1_TM_ANA_BYP_15, 0x00000040U, 0x00000040U),
	PSI_WRITE_MASKED_32(SERDES, L1_TM_ANA_BYP_12, 0x00000040U, 0x00000040U),
	PSI_WRITE_MASKED_32(SERDES, L2_TM_RST_DLY, 0x000000FFU, 0x000000FFU),
	PSI_WRITE_MASKED_32(SERDES, L2_TM_ANA_BYP_15, 0x00000040U, 0x00000040U),
	PSI_WRITE_MASKED_32(SERDES, L2_TM_ANA_BYP_12, 0x00000040U, 0x00000040U),
	PSI_WRITE_MASKED_32(SERDES, L3_TM_RST_DLY, 0x000000FFU, 0x000000FFU),
	PSI_WRITE_MASKED_32(SERDES, L3_TM_ANA_BYP_15, 0x00000040U, 0x00000040U),
	PSI_WRITE_MASKED_32(SERDES, L3_TM_ANA_BYP_12, 0x00000040U, 0x00000040U),

	// Disable FPL/FFL
	PSI_WRITE_MASKED_32(SERDES, L0_TM_MISC3, 0x00000003U, 0x00000000U),
	PSI_WRITE_MASKED_32(SERDES, L1_TM_MISC3, 0x00000003U, 0x00000000U),
	PSI_WRITE_MASKED_32(SERDES, L2_TM_MISC3, 0x00000003U, 0x00000000U),
	PSI_WRITE_MASKED_32(SERDES, L3_TM_MISC3, 0x00000003U, 0x00000000U),

	// Disable Dynamic offset calibration
	PSI_WRITE_MASKED_32(SERDES, L0_TM_EQ11, 0x00000010U, 0x00000010U),
	PSI_WRITE_MASKED_32(SERDES, L1_TM_EQ11, 0x00000010U, 0x00000010U),
	PSI_WRITE_MASKED_32(SERDES, L2_TM_EQ11, 0x00000010U, 0x00000010U),
	PSI_WRITE_MASKED_32(SERDES, L3_TM_EQ11, 0x00000010U, 0x00000010U),

	// SERDES ILL calibration (L2)
	PSI_WRITE_MASKED_32(SERDES, L2_TM_IQ_ILL8, 0x000000FFU, 0xF3),
	PSI_WRITE_MASKED_32(SERDES, L2_TM_E_ILL8, 0x000000FFU, 0xF3),
	PSI_WRITE_MASKED_32(SERDES, L2_TM_ILL12, 0x000000FFU, 0x20),
	PSI_WRITE_MASKED_32(SERDES, L2_TM_E_ILL1, 0x000000FFU, 0x37),

	// disable ECO for PCIE
	PSI_WRITE_MASKED_32(SERDES, ICM_CFG0, 0x00000077U, 0x00000044U),
	PSI_WRITE_MASKED_32(SERDES, ICM_CFG1, 0x00000007U, 0x00000003U),

	// checking PLL lock
	PSI_WRITE_MASKED_32(SERDES, L0_TXPMD_TM_45, 0x00000037U, 0x00000037U),
	PSI_WRITE_MASKED_32(SERDES, L1_TXPMD_TM_45, 0x00000037U, 0x00000037U),
	PSI_WRITE_MASKED_32(SERDES, L1_TX_ANA_TM_118, 0x00000001U, 0x00000001U),

	// CDR and RX Equalization Settings
	PSI_WRITE_MASKED_32(SERDES, L1_TXPMD_TM_48, 0x0000001FU, 0x00000000U),
	PSI_WRITE_MASKED_32(SERDES, L0_TXPMD_TM_48, 0x0000001FU, 0x00000000U),
	PSI_WRITE_MASKED_32(SERDES, L1_TX_ANA_TM_18, 0x000000FFU, 0x00000000U),
	PSI_WRITE_MASKED_32(SERDES, L0_TX_ANA_TM_18, 0x000000FFU, 0x00000000U),

	PSI_END_PROGRAM
};

__attribute__((__section__(".hwregs")))
static PSI_IWord const resetout_init[] = {
	// take USB0 out of reset
	PSI_FAR_WRITE_MASKED_32(CRL_APB, RST_LPD_TOP, 0x00000400U, 0x0U),
	PSI_SET_REGISTER_BANK(USB30_REGS),
	PSI_WRITE_MASKED_32(USB3_REGS, FPD_POWER_PRSNT, 0x00000001U, 0x1U),
	PSI_WRITE_MASKED_32(USB3_REGS, FPD_PIPE_CLK, 0x00000001U, 0x0U),
	PSI_FAR_WRITE_MASKED_32(CRL_APB, RST_LPD_TOP, 0x00000140U, 0x0U),
	// take GEM3 out of reset
	PSI_FAR_WRITE_MASKED_32(CRL_APB, RST_LPD_IOU0, 0x00000008U, 0x0U),
	// take DP out of reset
	PSI_FAR_WRITE_MASKED_32(CRF_APB, RST_FPD_TOP, 0x00010000U, 0x0U),
	PSI_FAR_WRITE_MASKED_32(DP, PHY_RESET, 0x00000002U, 0x0U),
	PSI_FAR_WRITE_MASKED_32(DP, TX_PHY_POWER_DOWN, 0x0000000FU, 0x00000000U),

	// usb3 global configs
	PSI_SET_REGISTER_BANK(USB30_XHCI),
	PSI_WRITE_MASKED_32(USB3_XHCI, GUSB2PHYCFG, 0x00023FFFU, 0x00022457U),
	PSI_WRITE_MASKED_32(USB3_XHCI, GFLADJ, 0x003FFF00U, 0x00000000U),
	PSI_WRITE_MASKED_32(USB3_XHCI, GUCTL1, 0x00000600U, 0x00000600U),
	PSI_WRITE_MASKED_32(USB3_XHCI, GUCTL, 0x00004000U, 0x00004000U),
	PSI_SET_REGISTER_BANK(SERDES),

	// poll for L1 and L2 PLL lock status
	PSI_POLL_MASKED_32(SERDES, L1_PLL_STATUS_READ_1, 0x00000010U),
	PSI_POLL_MASKED_32(SERDES, L2_PLL_STATUS_READ_1, 0x00000010U),

	PSI_END_PROGRAM
};

void resetInRunInitProgram(void) {
	psi_RunRegisterProgram(resetin_init);
}

void serdesRunInitProgram(void) {
	psi_RunRegisterProgram(serdes_init);
}


void resetOutRunInitProgram(void) {
	psi_RunRegisterProgram(resetout_init);
}

#if 0
static int mask_pollOnValue(u32 add, u32 mask, u32 value);

void mask_delay(u32 delay);

/**
 * CRL_APB Base Address
 */
#define CRL_APB_BASEADDR      0XFF5E0000U
#define CRL_APB_RST_LPD_IOU0    ((CRL_APB_BASEADDR) + 0X00000230U)
#define CRL_APB_RST_LPD_IOU1    ((CRL_APB_BASEADDR) + 0X00000234U)
#define CRL_APB_RST_LPD_IOU2    ((CRL_APB_BASEADDR) + 0X00000238U)
#define CRL_APB_RST_LPD_TOP    ((CRL_APB_BASEADDR) + 0X0000023CU)
#define CRL_APB_IOU_SWITCH_CTRL    ((CRL_APB_BASEADDR) + 0X0000009CU)
//static int serdes_rst_seq (u32 pllsel, u32 lane3_protocol, u32 lane3_rate, u32 lane2_protocol, u32 lane2_rate, u32 lane1_protocol, u32 lane1_rate, u32 lane0_protocol, u32 lane0_rate);
//static int serdes_illcalib_pcie_gen1 (u32 pllsel, u32 lane3_protocol, u32 lane3_rate, u32 lane2_protocol, u32 lane2_rate, u32 lane1_protocol, u32 lane1_rate, u32 lane0_protocol, u32 lane0_rate, u32 gen2_calib);

/**
 * CRF_APB Base Address
 */
#define CRF_APB_BASEADDR      0XFD1A0000U

#define CRF_APB_RST_FPD_TOP    ((CRF_APB_BASEADDR) + 0X00000100U)
#define CRF_APB_GPU_REF_CTRL    ((CRF_APB_BASEADDR) + 0X00000084U)
#define CRF_APB_RST_DDR_SS    ((CRF_APB_BASEADDR) + 0X00000108U)
#define PSU_MASK_POLL_TIME 1100000

/**
 *  * Register: CRF_APB_DPLL_CTRL
 */
#define CRF_APB_DPLL_CTRL    ((CRF_APB_BASEADDR) + 0X0000002C)


#define CRF_APB_DPLL_CTRL_DIV2_SHIFT   16
#define CRF_APB_DPLL_CTRL_DIV2_WIDTH   1

#define CRF_APB_DPLL_CTRL_FBDIV_SHIFT   8
#define CRF_APB_DPLL_CTRL_FBDIV_WIDTH   7

#define CRF_APB_DPLL_CTRL_BYPASS_SHIFT   3
#define CRF_APB_DPLL_CTRL_BYPASS_WIDTH   1

#define CRF_APB_DPLL_CTRL_RESET_SHIFT   0
#define CRF_APB_DPLL_CTRL_RESET_WIDTH   1

/**
 *  * Register: CRF_APB_DPLL_CFG
 */
#define CRF_APB_DPLL_CFG    ((CRF_APB_BASEADDR) + 0X00000030)

#define CRF_APB_DPLL_CFG_LOCK_DLY_SHIFT   25
#define CRF_APB_DPLL_CFG_LOCK_DLY_WIDTH   7

#define CRF_APB_DPLL_CFG_LOCK_CNT_SHIFT   13
#define CRF_APB_DPLL_CFG_LOCK_CNT_WIDTH   10

#define CRF_APB_DPLL_CFG_LFHF_SHIFT   10
#define CRF_APB_DPLL_CFG_LFHF_WIDTH   2

#define CRF_APB_DPLL_CFG_CP_SHIFT   5
#define CRF_APB_DPLL_CFG_CP_WIDTH   4

#define CRF_APB_DPLL_CFG_RES_SHIFT   0
#define CRF_APB_DPLL_CFG_RES_WIDTH   4

/**
 * Register: CRF_APB_PLL_STATUS
 */
#define CRF_APB_PLL_STATUS    ((CRF_APB_BASEADDR) + 0X00000044)

unsigned long psu_serdes_init_data(void)
{
	/*
	* SERDES INITIALIZATION
	*/
	/*
	* GT REFERENCE CLOCK SOURCE SELECTION
	*/
	/*
	* Register : PLL_REF_SEL0 @ 0XFD410000

	* PLL0 Reference Selection. 0x0 - 5MHz, 0x1 - 9.6MHz, 0x2 - 10MHz, 0x3 - 1
	* 2MHz, 0x4 - 13MHz, 0x5 - 19.2MHz, 0x6 - 20MHz, 0x7 - 24MHz, 0x8 - 26MHz,
	*  0x9 - 27MHz, 0xA - 38.4MHz, 0xB - 40MHz, 0xC - 52MHz, 0xD - 100MHz, 0xE
	*  - 108MHz, 0xF - 125MHz, 0x10 - 135MHz, 0x11 - 150 MHz. 0x12 to 0x1F - R
	* eserved
	*  PSU_SERDES_PLL_REF_SEL0_PLLREFSEL0                          0x9

	* PLL0 Reference Selection Register
	* (OFFSET, MASK, VALUE)      (0XFD410000, 0x0000001FU ,0x00000009U)
	*/
	PSU_Mask_Write(SERDES_PLL_REF_SEL0_OFFSET, 0x0000001FU, 0x00000009U);
	/*##################################################################### */

	/*
	* Register : PLL_REF_SEL1 @ 0XFD410004

	* PLL1 Reference Selection. 0x0 - 5MHz, 0x1 - 9.6MHz, 0x2 - 10MHz, 0x3 - 1
	* 2MHz, 0x4 - 13MHz, 0x5 - 19.2MHz, 0x6 - 20MHz, 0x7 - 24MHz, 0x8 - 26MHz,
	*  0x9 - 27MHz, 0xA - 38.4MHz, 0xB - 40MHz, 0xC - 52MHz, 0xD - 100MHz, 0xE
	*  - 108MHz, 0xF - 125MHz, 0x10 - 135MHz, 0x11 - 150 MHz. 0x12 to 0x1F - R
	* eserved
	*  PSU_SERDES_PLL_REF_SEL1_PLLREFSEL1                          0x9

	* PLL1 Reference Selection Register
	* (OFFSET, MASK, VALUE)      (0XFD410004, 0x0000001FU ,0x00000009U)
	*/
	PSU_Mask_Write(SERDES_PLL_REF_SEL1_OFFSET, 0x0000001FU, 0x00000009U);
	/*##################################################################### */

	/*
	* Register : PLL_REF_SEL2 @ 0XFD410008

	* PLL2 Reference Selection. 0x0 - 5MHz, 0x1 - 9.6MHz, 0x2 - 10MHz, 0x3 - 1
	* 2MHz, 0x4 - 13MHz, 0x5 - 19.2MHz, 0x6 - 20MHz, 0x7 - 24MHz, 0x8 - 26MHz,
	*  0x9 - 27MHz, 0xA - 38.4MHz, 0xB - 40MHz, 0xC - 52MHz, 0xD - 100MHz, 0xE
	*  - 108MHz, 0xF - 125MHz, 0x10 - 135MHz, 0x11 - 150 MHz. 0x12 to 0x1F - R
	* eserved
	*  PSU_SERDES_PLL_REF_SEL2_PLLREFSEL2                          0x8

	* PLL2 Reference Selection Register
	* (OFFSET, MASK, VALUE)      (0XFD410008, 0x0000001FU ,0x00000008U)
	*/
	PSU_Mask_Write(SERDES_PLL_REF_SEL2_OFFSET, 0x0000001FU, 0x00000008U);
	/*##################################################################### */

	* GT REFERENCE CLOCK FREQUENCY SELECTION
	*/
	/*
	* Register : L0_L0_REF_CLK_SEL @ 0XFD402860

	* Sel of lane 0 ref clock local mux. Set to 1 to select lane 0 slicer outp
	* ut. Set to 0 to select lane0 ref clock mux output.
	*  PSU_SERDES_L0_L0_REF_CLK_SEL_L0_REF_CLK_LCL_SEL             0x1

	* Lane0 Ref Clock Selection Register
	* (OFFSET, MASK, VALUE)      (0XFD402860, 0x00000080U ,0x00000080U)
	*/
	PSU_Mask_Write(SERDES_L0_L0_REF_CLK_SEL_OFFSET,
	               0x00000080U, 0x00000080U);
	/*##################################################################### */

	/*
	* Register : L0_L1_REF_CLK_SEL @ 0XFD402864

	* Sel of lane 1 ref clock local mux. Set to 1 to select lane 1 slicer outp
	* ut. Set to 0 to select lane1 ref clock mux output.
	*  PSU_SERDES_L0_L1_REF_CLK_SEL_L1_REF_CLK_LCL_SEL             0x0

	* Bit 0 of lane 1 ref clock mux one hot sel. Set to 1 to select lane 0 sli
	* cer output from ref clock network
	*  PSU_SERDES_L0_L1_REF_CLK_SEL_L1_REF_CLK_SEL_0               0x1

	* Lane1 Ref Clock Selection Register
	* (OFFSET, MASK, VALUE)      (0XFD402864, 0x00000081U ,0x00000001U)
	*/
	PSU_Mask_Write(SERDES_L0_L1_REF_CLK_SEL_OFFSET,
	               0x00000081U, 0x00000001U);
	/*##################################################################### */

	/*
	* Register : L0_L2_REF_CLK_SEL @ 0XFD402868

	* Sel of lane 2 ref clock local mux. Set to 1 to select lane 1 slicer outp
	* ut. Set to 0 to select lane2 ref clock mux output.
	*  PSU_SERDES_L0_L2_REF_CLK_SEL_L2_REF_CLK_LCL_SEL             0x0

	* Bit 1 of lane 2 ref clock mux one hot sel. Set to 1 to select lane 1 sli
	* cer output from ref clock network
	*  PSU_SERDES_L0_L2_REF_CLK_SEL_L2_REF_CLK_SEL_1               0x1

	* Lane2 Ref Clock Selection Register
	* (OFFSET, MASK, VALUE)      (0XFD402868, 0x00000082U ,0x00000002U)
	*/
	PSU_Mask_Write(SERDES_L0_L2_REF_CLK_SEL_OFFSET,
	               0x00000082U, 0x00000002U);
	/*##################################################################### */

	/*
	* ENABLE SPREAD SPECTRUM
	*/
	/*
	* Register : L2_TM_PLL_DIG_37 @ 0XFD40A094

	* Enable/Disable coarse code satureation limiting logic
	*  PSU_SERDES_L2_TM_PLL_DIG_37_TM_ENABLE_COARSE_SATURATION     0x1

	* Test mode register 37
	* (OFFSET, MASK, VALUE)      (0XFD40A094, 0x00000010U ,0x00000010U)
	*/
	PSU_Mask_Write(SERDES_L2_TM_PLL_DIG_37_OFFSET,
	               0x00000010U, 0x00000010U);
	/*##################################################################### */

	/*
	* Register : L2_PLL_SS_STEPS_0_LSB @ 0XFD40A368

	* Spread Spectrum No of Steps [7:0]
	*  PSU_SERDES_L2_PLL_SS_STEPS_0_LSB_SS_NUM_OF_STEPS_0_LSB      0x38

	* Spread Spectrum No of Steps bits 7:0
	* (OFFSET, MASK, VALUE)      (0XFD40A368, 0x000000FFU ,0x00000038U)
	*/
	PSU_Mask_Write(SERDES_L2_PLL_SS_STEPS_0_LSB_OFFSET,
	               0x000000FFU, 0x00000038U);
	/*##################################################################### */

	/*
	* Register : L2_PLL_SS_STEPS_1_MSB @ 0XFD40A36C

	* Spread Spectrum No of Steps [10:8]
	*  PSU_SERDES_L2_PLL_SS_STEPS_1_MSB_SS_NUM_OF_STEPS_1_MSB      0x03

	* Spread Spectrum No of Steps bits 10:8
	* (OFFSET, MASK, VALUE)      (0XFD40A36C, 0x00000007U ,0x00000003U)
	*/
	PSU_Mask_Write(SERDES_L2_PLL_SS_STEPS_1_MSB_OFFSET,
	               0x00000007U, 0x00000003U);
	/*##################################################################### */

	/*
	* Register : L0_PLL_SS_STEPS_0_LSB @ 0XFD402368

	* Spread Spectrum No of Steps [7:0]
	*  PSU_SERDES_L0_PLL_SS_STEPS_0_LSB_SS_NUM_OF_STEPS_0_LSB      0x58

	* Spread Spectrum No of Steps bits 7:0
	* (OFFSET, MASK, VALUE)      (0XFD402368, 0x000000FFU ,0x00000058U)
	*/
	PSU_Mask_Write(SERDES_L0_PLL_SS_STEPS_0_LSB_OFFSET,
	               0x000000FFU, 0x00000058U);
	/*##################################################################### */

	/*
	* Register : L0_PLL_SS_STEPS_1_MSB @ 0XFD40236C

	* Spread Spectrum No of Steps [10:8]
	*  PSU_SERDES_L0_PLL_SS_STEPS_1_MSB_SS_NUM_OF_STEPS_1_MSB      0x3

	* Spread Spectrum No of Steps bits 10:8
	* (OFFSET, MASK, VALUE)      (0XFD40236C, 0x00000007U ,0x00000003U)
	*/
	PSU_Mask_Write(SERDES_L0_PLL_SS_STEPS_1_MSB_OFFSET,
	               0x00000007U, 0x00000003U);
	/*##################################################################### */

	/*
	* Register : L1_PLL_SS_STEPS_0_LSB @ 0XFD406368

	* Spread Spectrum No of Steps [7:0]
	*  PSU_SERDES_L1_PLL_SS_STEPS_0_LSB_SS_NUM_OF_STEPS_0_LSB      0x58

	* Spread Spectrum No of Steps bits 7:0
	* (OFFSET, MASK, VALUE)      (0XFD406368, 0x000000FFU ,0x00000058U)
	*/
	PSU_Mask_Write(SERDES_L1_PLL_SS_STEPS_0_LSB_OFFSET,
	               0x000000FFU, 0x00000058U);
	/*##################################################################### */

	/*
	* Register : L1_PLL_SS_STEPS_1_MSB @ 0XFD40636C

	* Spread Spectrum No of Steps [10:8]
	*  PSU_SERDES_L1_PLL_SS_STEPS_1_MSB_SS_NUM_OF_STEPS_1_MSB      0x3

	* Spread Spectrum No of Steps bits 10:8
	* (OFFSET, MASK, VALUE)      (0XFD40636C, 0x00000007U ,0x00000003U)
	*/
	PSU_Mask_Write(SERDES_L1_PLL_SS_STEPS_1_MSB_OFFSET,
	               0x00000007U, 0x00000003U);
	/*##################################################################### */

	/*
	* Register : L0_PLL_SS_STEP_SIZE_0_LSB @ 0XFD402370

	* Step Size for Spread Spectrum [7:0]
	*  PSU_SERDES_L0_PLL_SS_STEP_SIZE_0_LSB_SS_STEP_SIZE_0_LSB     0x7C

	* Step Size for Spread Spectrum LSB
	* (OFFSET, MASK, VALUE)      (0XFD402370, 0x000000FFU ,0x0000007CU)
	*/
	PSU_Mask_Write(SERDES_L0_PLL_SS_STEP_SIZE_0_LSB_OFFSET,
	               0x000000FFU, 0x0000007CU);
	/*##################################################################### */

	/*
	* Register : L0_PLL_SS_STEP_SIZE_1 @ 0XFD402374

	* Step Size for Spread Spectrum [15:8]
	*  PSU_SERDES_L0_PLL_SS_STEP_SIZE_1_SS_STEP_SIZE_1             0x33

	* Step Size for Spread Spectrum 1
	* (OFFSET, MASK, VALUE)      (0XFD402374, 0x000000FFU ,0x00000033U)
	*/
	PSU_Mask_Write(SERDES_L0_PLL_SS_STEP_SIZE_1_OFFSET,
	               0x000000FFU, 0x00000033U);
	/*##################################################################### */
	/*
	* Register : L0_PLL_SS_STEP_SIZE_2 @ 0XFD402378

	* Step Size for Spread Spectrum [23:16]
	*  PSU_SERDES_L0_PLL_SS_STEP_SIZE_2_SS_STEP_SIZE_LSB_2             0x2

	* Step Size for Spread Spectrum 2
	* (OFFSET, MASK, VALUE)      (0XFD402378, 0x000000FFU ,0x00000002U)
	*/
	PSU_Mask_Write(SERDES_L0_PLL_SS_STEP_SIZE_2_OFFSET,
	               0x000000FFU, 0x00000002U);
	/*##################################################################### */

	/*
	* Register : L0_PLL_SS_STEP_SIZE_3_MSB @ 0XFD40237C

	* Step Size for Spread Spectrum [25:24]
	*  PSU_SERDES_L0_PLL_SS_STEP_SIZE_3_MSB_SS_STEP_SIZE_3_MSB     0x0

	* Enable/Disable test mode force on SS step size
	*  PSU_SERDES_L0_PLL_SS_STEP_SIZE_3_MSB_FORCE_SS_STEP_SIZE     0x1

	* Enable/Disable test mode force on SS no of steps
	*  PSU_SERDES_L0_PLL_SS_STEP_SIZE_3_MSB_FORCE_SS_NUM_OF_STEPS  0x1

	* Enable force on enable Spread Spectrum
	* (OFFSET, MASK, VALUE)      (0XFD40237C, 0x00000033U ,0x00000030U)
	*/
	PSU_Mask_Write(SERDES_L0_PLL_SS_STEP_SIZE_3_MSB_OFFSET,
	               0x00000033U, 0x00000030U);
	/*##################################################################### */

	/*
	* Register : L1_PLL_SS_STEP_SIZE_0_LSB @ 0XFD406370

	* Step Size for Spread Spectrum [7:0]
	*  PSU_SERDES_L1_PLL_SS_STEP_SIZE_0_LSB_SS_STEP_SIZE_0_LSB     0x7C

	* Step Size for Spread Spectrum LSB
	* (OFFSET, MASK, VALUE)      (0XFD406370, 0x000000FFU ,0x0000007CU)
	*/
	PSU_Mask_Write(SERDES_L1_PLL_SS_STEP_SIZE_0_LSB_OFFSET,
	               0x000000FFU, 0x0000007CU);
	/*##################################################################### */

	/*
	* Register : L1_PLL_SS_STEP_SIZE_1 @ 0XFD406374

	* Step Size for Spread Spectrum [15:8]
	*  PSU_SERDES_L1_PLL_SS_STEP_SIZE_1_SS_STEP_SIZE_1             0x33

	* Step Size for Spread Spectrum 1
	* (OFFSET, MASK, VALUE)      (0XFD406374, 0x000000FFU ,0x00000033U)
	*/
	PSU_Mask_Write(SERDES_L1_PLL_SS_STEP_SIZE_1_OFFSET,
	               0x000000FFU, 0x00000033U);
	/*##################################################################### */

	/*
	* Register : L1_PLL_SS_STEP_SIZE_2 @ 0XFD406378

	* Step Size for Spread Spectrum [23:16]
	*  PSU_SERDES_L1_PLL_SS_STEP_SIZE_2_SS_STEP_SIZE_2             0x2

	* Step Size for Spread Spectrum 2
	* (OFFSET, MASK, VALUE)      (0XFD406378, 0x000000FFU ,0x00000002U)
	*/
	PSU_Mask_Write(SERDES_L1_PLL_SS_STEP_SIZE_2_OFFSET,
	               0x000000FFU, 0x00000002U);
	/*##################################################################### */

	/*
	* Register : L1_PLL_SS_STEP_SIZE_3_MSB @ 0XFD40637C

	* Step Size for Spread Spectrum [25:24]
	*  PSU_SERDES_L1_PLL_SS_STEP_SIZE_3_MSB_SS_STEP_SIZE_3_MSB     0x0

	* Enable/Disable test mode force on SS step size
	*  PSU_SERDES_L1_PLL_SS_STEP_SIZE_3_MSB_FORCE_SS_STEP_SIZE     0x1

	* Enable/Disable test mode force on SS no of steps
	*  PSU_SERDES_L1_PLL_SS_STEP_SIZE_3_MSB_FORCE_SS_NUM_OF_STEPS  0x1

	* Enable force on enable Spread Spectrum
	* (OFFSET, MASK, VALUE)      (0XFD40637C, 0x00000033U ,0x00000030U)
	*/
	PSU_Mask_Write(SERDES_L1_PLL_SS_STEP_SIZE_3_MSB_OFFSET,
	               0x00000033U, 0x00000030U);
	/*##################################################################### */

	/*
	* Register : L2_PLL_SS_STEP_SIZE_0_LSB @ 0XFD40A370

	* Step Size for Spread Spectrum [7:0]
	*  PSU_SERDES_L2_PLL_SS_STEP_SIZE_0_LSB_SS_STEP_SIZE_0_LSB     0xF4

	* Step Size for Spread Spectrum LSB
	* (OFFSET, MASK, VALUE)      (0XFD40A370, 0x000000FFU ,0x000000F4U)
	*/
	PSU_Mask_Write(SERDES_L2_PLL_SS_STEP_SIZE_0_LSB_OFFSET,
	               0x000000FFU, 0x000000F4U);
	/*##################################################################### */

	/*
	* Register : L2_PLL_SS_STEP_SIZE_1 @ 0XFD40A374

	* Step Size for Spread Spectrum [15:8]
	*  PSU_SERDES_L2_PLL_SS_STEP_SIZE_1_SS_STEP_SIZE_1             0x31

	* Step Size for Spread Spectrum 1
	* (OFFSET, MASK, VALUE)      (0XFD40A374, 0x000000FFU ,0x00000031U)
	*/
	PSU_Mask_Write(SERDES_L2_PLL_SS_STEP_SIZE_1_OFFSET,
	               0x000000FFU, 0x00000031U);
	/*##################################################################### */

	/*
	* Register : L2_PLL_SS_STEP_SIZE_2 @ 0XFD40A378

	* Step Size for Spread Spectrum [23:16]
	*  PSU_SERDES_L2_PLL_SS_STEP_SIZE_2_SS_STEP_SIZE_2             0x2

	* Step Size for Spread Spectrum 2
	* (OFFSET, MASK, VALUE)      (0XFD40A378, 0x000000FFU ,0x00000002U)
	*/
	PSU_Mask_Write(SERDES_L2_PLL_SS_STEP_SIZE_2_OFFSET,
	               0x000000FFU, 0x00000002U);
	/*##################################################################### */

	/*
	* Register : L2_PLL_SS_STEP_SIZE_3_MSB @ 0XFD40A37C

	* Step Size for Spread Spectrum [25:24]
	*  PSU_SERDES_L2_PLL_SS_STEP_SIZE_3_MSB_SS_STEP_SIZE_3_MSB     0x0

	* Enable/Disable test mode force on SS step size
	*  PSU_SERDES_L2_PLL_SS_STEP_SIZE_3_MSB_FORCE_SS_STEP_SIZE     0x1

	* Enable/Disable test mode force on SS no of steps
	*  PSU_SERDES_L2_PLL_SS_STEP_SIZE_3_MSB_FORCE_SS_NUM_OF_STEPS  0x1

	* Enable force on enable Spread Spectrum
	* (OFFSET, MASK, VALUE)      (0XFD40A37C, 0x00000033U ,0x00000030U)
	*/
	PSU_Mask_Write(SERDES_L2_PLL_SS_STEP_SIZE_3_MSB_OFFSET,
	               0x00000033U, 0x00000030U);
	/*##################################################################### */

	/*
	* Register : L2_TM_DIG_6 @ 0XFD40906C

	* Bypass Descrambler
	*  PSU_SERDES_L2_TM_DIG_6_BYPASS_DESCRAM                       0x1

	* Enable Bypass for <1> TM_DIG_CTRL_6
	*  PSU_SERDES_L2_TM_DIG_6_FORCE_BYPASS_DESCRAM                 0x1

	* Data path test modes in decoder and descram
	* (OFFSET, MASK, VALUE)      (0XFD40906C, 0x00000003U ,0x00000003U)
	*/
	PSU_Mask_Write(SERDES_L2_TM_DIG_6_OFFSET, 0x00000003U, 0x00000003U);
	/*##################################################################### */

	/*
	* Register : L2_TX_DIG_TM_61 @ 0XFD4080F4

	* Bypass scrambler signal
	*  PSU_SERDES_L2_TX_DIG_TM_61_BYPASS_SCRAM                     0x1

	* Enable/disable scrambler bypass signal
	*  PSU_SERDES_L2_TX_DIG_TM_61_FORCE_BYPASS_SCRAM               0x1

	* MPHY PLL Gear and bypass scrambler
	* (OFFSET, MASK, VALUE)      (0XFD4080F4, 0x00000003U ,0x00000003U)
	*/
	PSU_Mask_Write(SERDES_L2_TX_DIG_TM_61_OFFSET,
	               0x00000003U, 0x00000003U);
	/*##################################################################### */

	/*
	* ENABLE CHICKEN BIT FOR PCIE AND USB
	*/
	/*
	* Register : L2_TM_AUX_0 @ 0XFD4090CC

	* Spare- not used
	*  PSU_SERDES_L2_TM_AUX_0_BIT_2                                1

	* Spare registers
	* (OFFSET, MASK, VALUE)      (0XFD4090CC, 0x00000020U ,0x00000020U)
	*/
	PSU_Mask_Write(SERDES_L2_TM_AUX_0_OFFSET, 0x00000020U, 0x00000020U);
	/*##################################################################### */

	/*
	* ENABLING EYE SURF
	*/
	/*
	* Register : L0_TM_DIG_8 @ 0XFD401074

	* Enable Eye Surf
	*  PSU_SERDES_L0_TM_DIG_8_EYESURF_ENABLE                       0x1

	* Test modes for Elastic buffer and enabling Eye Surf
	* (OFFSET, MASK, VALUE)      (0XFD401074, 0x00000010U ,0x00000010U)
	*/
	PSU_Mask_Write(SERDES_L0_TM_DIG_8_OFFSET, 0x00000010U, 0x00000010U);
	/*##################################################################### */

	/*
	* Register : L1_TM_DIG_8 @ 0XFD405074

	* Enable Eye Surf
	*  PSU_SERDES_L1_TM_DIG_8_EYESURF_ENABLE                       0x1

	* Test modes for Elastic buffer and enabling Eye Surf
	* (OFFSET, MASK, VALUE)      (0XFD405074, 0x00000010U ,0x00000010U)
	*/
	PSU_Mask_Write(SERDES_L1_TM_DIG_8_OFFSET, 0x00000010U, 0x00000010U);
	/*##################################################################### */

	/*
	* Register : L2_TM_DIG_8 @ 0XFD409074

	* Enable Eye Surf
	*  PSU_SERDES_L2_TM_DIG_8_EYESURF_ENABLE                       0x1

	* Test modes for Elastic buffer and enabling Eye Surf
	* (OFFSET, MASK, VALUE)      (0XFD409074, 0x00000010U ,0x00000010U)
	*/
	PSU_Mask_Write(SERDES_L2_TM_DIG_8_OFFSET, 0x00000010U, 0x00000010U);
	/*##################################################################### */

	/*
	* Register : L3_TM_DIG_8 @ 0XFD40D074

	* Enable Eye Surf
	*  PSU_SERDES_L3_TM_DIG_8_EYESURF_ENABLE                       0x1

	* Test modes for Elastic buffer and enabling Eye Surf
	* (OFFSET, MASK, VALUE)      (0XFD40D074, 0x00000010U ,0x00000010U)
	*/
	PSU_Mask_Write(SERDES_L3_TM_DIG_8_OFFSET, 0x00000010U, 0x00000010U);
	/*##################################################################### */
	/*
	* ILL SETTINGS FOR GAIN AND LOCK SETTINGS
	*/
	/*
	* Register : L0_TM_ILL13 @ 0XFD401994

	* ILL cal idle val refcnt
	*  PSU_SERDES_L0_TM_ILL13_ILL_CAL_IDLE_VAL_REFCNT              0x7

	* ill cal idle value count
	* (OFFSET, MASK, VALUE)      (0XFD401994, 0x00000007U ,0x00000007U)
	*/
	PSU_Mask_Write(SERDES_L0_TM_ILL13_OFFSET, 0x00000007U, 0x00000007U);
	/*##################################################################### */

	/*
	* Register : L1_TM_ILL13 @ 0XFD405994

	* ILL cal idle val refcnt
	*  PSU_SERDES_L1_TM_ILL13_ILL_CAL_IDLE_VAL_REFCNT              0x7

	* ill cal idle value count
	* (OFFSET, MASK, VALUE)      (0XFD405994, 0x00000007U ,0x00000007U)
	*/
	PSU_Mask_Write(SERDES_L1_TM_ILL13_OFFSET, 0x00000007U, 0x00000007U);
	/*##################################################################### */
	/*
	* Register : L2_TM_MISC2 @ 0XFD40989C

	* ILL calib counts BYPASSED with calcode bits
	*  PSU_SERDES_L2_TM_MISC2_ILL_CAL_BYPASS_COUNTS                0x1

	* sampler cal
	* (OFFSET, MASK, VALUE)      (0XFD40989C, 0x00000080U ,0x00000080U)
	*/
	PSU_Mask_Write(SERDES_L2_TM_MISC2_OFFSET, 0x00000080U, 0x00000080U);
	/*##################################################################### */

	/*
	* Register : L2_TM_IQ_ILL1 @ 0XFD4098F8

	* IQ ILL F0 CALCODE bypass value. MPHY : G1a, PCIE : Gen 1, SATA : Gen1 ,
	* USB3 : SS
	*  PSU_SERDES_L2_TM_IQ_ILL1_ILL_BYPASS_IQ_CALCODE_F0           0x1A

	* iqpi cal code
	* (OFFSET, MASK, VALUE)      (0XFD4098F8, 0x000000FFU ,0x0000001AU)
	*/
	PSU_Mask_Write(SERDES_L2_TM_IQ_ILL1_OFFSET,
	               0x000000FFU, 0x0000001AU);
	/*##################################################################### */

	/*
	* Register : L2_TM_IQ_ILL2 @ 0XFD4098FC

	* IQ ILL F1 CALCODE bypass value. MPHY : G1b, PCIE : Gen2, SATA: Gen2
	*  PSU_SERDES_L2_TM_IQ_ILL2_ILL_BYPASS_IQ_CALCODE_F1           0x1A

	* iqpi cal code
	* (OFFSET, MASK, VALUE)      (0XFD4098FC, 0x000000FFU ,0x0000001AU)
	*/
	PSU_Mask_Write(SERDES_L2_TM_IQ_ILL2_OFFSET,
	               0x000000FFU, 0x0000001AU);
	/*##################################################################### */

	/*
	* Register : L2_TM_ILL12 @ 0XFD409990

	* G1A pll ctr bypass value
	*  PSU_SERDES_L2_TM_ILL12_G1A_PLL_CTR_BYP_VAL                  0x10

	* ill pll counter values
	* (OFFSET, MASK, VALUE)      (0XFD409990, 0x000000FFU ,0x00000010U)
	*/
	PSU_Mask_Write(SERDES_L2_TM_ILL12_OFFSET, 0x000000FFU, 0x00000010U);
	/*##################################################################### */

	/*
	* Register : L2_TM_E_ILL1 @ 0XFD409924

	* E ILL F0 CALCODE bypass value. MPHY : G1a, PCIE : Gen 1, SATA : Gen1 , U
	* SB3 : SS
	*  PSU_SERDES_L2_TM_E_ILL1_ILL_BYPASS_E_CALCODE_F0             0xFE

	* epi cal code
	* (OFFSET, MASK, VALUE)      (0XFD409924, 0x000000FFU ,0x000000FEU)
	*/
	PSU_Mask_Write(SERDES_L2_TM_E_ILL1_OFFSET, 0x000000FFU, 0x000000FEU);
	/*##################################################################### */

	/*
	* Register : L2_TM_E_ILL2 @ 0XFD409928

	* E ILL F1 CALCODE bypass value. MPHY : G1b, PCIE : Gen2, SATA: Gen2
	*  PSU_SERDES_L2_TM_E_ILL2_ILL_BYPASS_E_CALCODE_F1             0x0

	* epi cal code
	* (OFFSET, MASK, VALUE)      (0XFD409928, 0x000000FFU ,0x00000000U)
	*/
	PSU_Mask_Write(SERDES_L2_TM_E_ILL2_OFFSET, 0x000000FFU, 0x00000000U);
	/*##################################################################### */

	/*
	* Register : L2_TM_IQ_ILL3 @ 0XFD409900

	* IQ ILL F2CALCODE bypass value. MPHY : G2a, SATA : Gen3
	*  PSU_SERDES_L2_TM_IQ_ILL3_ILL_BYPASS_IQ_CALCODE_F2           0x1A

	* iqpi cal code
	* (OFFSET, MASK, VALUE)      (0XFD409900, 0x000000FFU ,0x0000001AU)
	*/
	PSU_Mask_Write(SERDES_L2_TM_IQ_ILL3_OFFSET,
	               0x000000FFU, 0x0000001AU);
	/*##################################################################### */

	/*
	* Register : L2_TM_E_ILL3 @ 0XFD40992C

	* E ILL F2CALCODE bypass value. MPHY : G2a, SATA : Gen3
	*  PSU_SERDES_L2_TM_E_ILL3_ILL_BYPASS_E_CALCODE_F2             0x0

	* epi cal code
	* (OFFSET, MASK, VALUE)      (0XFD40992C, 0x000000FFU ,0x00000000U)
	*/
	PSU_Mask_Write(SERDES_L2_TM_E_ILL3_OFFSET, 0x000000FFU, 0x00000000U);
	/*##################################################################### */

	/*
	* Register : L2_TM_ILL8 @ 0XFD409980

	* ILL calibration code change wait time
	*  PSU_SERDES_L2_TM_ILL8_ILL_CAL_ITER_WAIT                     0xFF

	* ILL cal routine control
	* (OFFSET, MASK, VALUE)      (0XFD409980, 0x000000FFU ,0x000000FFU)
	*/
	PSU_Mask_Write(SERDES_L2_TM_ILL8_OFFSET, 0x000000FFU, 0x000000FFU);
	/*##################################################################### */

	/*
	* Register : L2_TM_IQ_ILL8 @ 0XFD409914

	* IQ ILL polytrim bypass value
	*  PSU_SERDES_L2_TM_IQ_ILL8_ILL_BYPASS_IQ_POLYTRIM_VAL         0xF7

	* iqpi polytrim
	* (OFFSET, MASK, VALUE)      (0XFD409914, 0x000000FFU ,0x000000F7U)
	*/
	PSU_Mask_Write(SERDES_L2_TM_IQ_ILL8_OFFSET,
	               0x000000FFU, 0x000000F7U);
	/*##################################################################### */

	/*
	* Register : L2_TM_IQ_ILL9 @ 0XFD409918

	* bypass IQ polytrim
	*  PSU_SERDES_L2_TM_IQ_ILL9_ILL_BYPASS_IQ_POLYTIM              0x1

	* enables for lf,constant gm trim and polytirm
	* (OFFSET, MASK, VALUE)      (0XFD409918, 0x00000001U ,0x00000001U)
	*/
	PSU_Mask_Write(SERDES_L2_TM_IQ_ILL9_OFFSET,
	               0x00000001U, 0x00000001U);
	/*##################################################################### */

	/*
	* Register : L2_TM_E_ILL8 @ 0XFD409940

	* E ILL polytrim bypass value
	*  PSU_SERDES_L2_TM_E_ILL8_ILL_BYPASS_E_POLYTRIM_VAL           0xF7

	* epi polytrim
	* (OFFSET, MASK, VALUE)      (0XFD409940, 0x000000FFU ,0x000000F7U)
	*/
	PSU_Mask_Write(SERDES_L2_TM_E_ILL8_OFFSET, 0x000000FFU, 0x000000F7U);
	/*##################################################################### */

	/*
	* Register : L2_TM_E_ILL9 @ 0XFD409944

	* bypass E polytrim
	*  PSU_SERDES_L2_TM_E_ILL9_ILL_BYPASS_E_POLYTIM                0x1

	* enables for lf,constant gm trim and polytirm
	* (OFFSET, MASK, VALUE)      (0XFD409944, 0x00000001U ,0x00000001U)
	*/
	PSU_Mask_Write(SERDES_L2_TM_E_ILL9_OFFSET, 0x00000001U, 0x00000001U);
	/*##################################################################### */

	/*
	* Register : L2_TM_ILL13 @ 0XFD409994

	* ILL cal idle val refcnt
	*  PSU_SERDES_L2_TM_ILL13_ILL_CAL_IDLE_VAL_REFCNT              0x7

	* ill cal idle value count
	* (OFFSET, MASK, VALUE)      (0XFD409994, 0x00000007U ,0x00000007U)
	*/
	PSU_Mask_Write(SERDES_L2_TM_ILL13_OFFSET, 0x00000007U, 0x00000007U);
	/*##################################################################### */

	/*
	* Register : L3_TM_ILL13 @ 0XFD40D994

	* ILL cal idle val refcnt
	*  PSU_SERDES_L3_TM_ILL13_ILL_CAL_IDLE_VAL_REFCNT              0x7

	* ill cal idle value count
	* (OFFSET, MASK, VALUE)      (0XFD40D994, 0x00000007U ,0x00000007U)
	*/
	PSU_Mask_Write(SERDES_L3_TM_ILL13_OFFSET, 0x00000007U, 0x00000007U);
	/*##################################################################### */
	/*
	* SYMBOL LOCK AND WAIT
	*/
	/*
	* Register : L0_TM_DIG_10 @ 0XFD40107C

	* CDR lock wait time. (1-16 us). cdr_lock_wait_time = 4'b xxxx + 4'b 0001
	*  PSU_SERDES_L0_TM_DIG_10_CDR_BIT_LOCK_TIME                   0x1

	* test control for changing cdr lock wait time
	* (OFFSET, MASK, VALUE)      (0XFD40107C, 0x0000000FU ,0x00000001U)
	*/
	PSU_Mask_Write(SERDES_L0_TM_DIG_10_OFFSET, 0x0000000FU, 0x00000001U);
	/*##################################################################### */

	/*
	* Register : L1_TM_DIG_10 @ 0XFD40507C

	* CDR lock wait time. (1-16 us). cdr_lock_wait_time = 4'b xxxx + 4'b 0001
	*  PSU_SERDES_L1_TM_DIG_10_CDR_BIT_LOCK_TIME                   0x1

	* test control for changing cdr lock wait time
	* (OFFSET, MASK, VALUE)      (0XFD40507C, 0x0000000FU ,0x00000001U)
	*/
	PSU_Mask_Write(SERDES_L1_TM_DIG_10_OFFSET, 0x0000000FU, 0x00000001U);
	/*##################################################################### */

	/*
	* Register : L2_TM_DIG_10 @ 0XFD40907C

	* CDR lock wait time. (1-16 us). cdr_lock_wait_time = 4'b xxxx + 4'b 0001
	*  PSU_SERDES_L2_TM_DIG_10_CDR_BIT_LOCK_TIME                   0x1

	* test control for changing cdr lock wait time
	* (OFFSET, MASK, VALUE)      (0XFD40907C, 0x0000000FU ,0x00000001U)
	*/
	PSU_Mask_Write(SERDES_L2_TM_DIG_10_OFFSET, 0x0000000FU, 0x00000001U);
	/*##################################################################### */

	/*
	* Register : L3_TM_DIG_10 @ 0XFD40D07C

	* CDR lock wait time. (1-16 us). cdr_lock_wait_time = 4'b xxxx + 4'b 0001
	*  PSU_SERDES_L3_TM_DIG_10_CDR_BIT_LOCK_TIME                   0x1

	* test control for changing cdr lock wait time
	* (OFFSET, MASK, VALUE)      (0XFD40D07C, 0x0000000FU ,0x00000001U)
	*/
	PSU_Mask_Write(SERDES_L3_TM_DIG_10_OFFSET, 0x0000000FU, 0x00000001U);
	/*##################################################################### */
	/*
	* SIOU SETTINGS FOR BYPASS CONTROL,HSRX-DIG
	*/
	/*
	* Register : L0_TM_RST_DLY @ 0XFD4019A4

	* Delay apb reset by specified amount
	*  PSU_SERDES_L0_TM_RST_DLY_APB_RST_DLY                        0xFF

	* reset delay for apb reset w.r.t pso of hsrx
	* (OFFSET, MASK, VALUE)      (0XFD4019A4, 0x000000FFU ,0x000000FFU)
	*/
	PSU_Mask_Write(SERDES_L0_TM_RST_DLY_OFFSET,
	               0x000000FFU, 0x000000FFU);
	/*##################################################################### */

	/*
	* Register : L0_TM_ANA_BYP_15 @ 0XFD401038

	* Enable Bypass for <7> of TM_ANA_BYPS_15
	*  PSU_SERDES_L0_TM_ANA_BYP_15_FORCE_UPHY_ENABLE_LOW_LEAKAGE   0x1

	* Bypass control for pcs-pma interface. EQ supplies, main master supply an
	* d ps for samp c2c
	* (OFFSET, MASK, VALUE)      (0XFD401038, 0x00000040U ,0x00000040U)
	*/
	PSU_Mask_Write(SERDES_L0_TM_ANA_BYP_15_OFFSET,
	               0x00000040U, 0x00000040U);
	/*##################################################################### */

	/*
	* Register : L0_TM_ANA_BYP_12 @ 0XFD40102C

	* Enable Bypass for <7> of TM_ANA_BYPS_12
	*  PSU_SERDES_L0_TM_ANA_BYP_12_FORCE_UPHY_PSO_HSRXDIG          0x1

	* Bypass control for pcs-pma interface. Hsrx supply, hsrx des, and cdr ena
	* ble controls
	* (OFFSET, MASK, VALUE)      (0XFD40102C, 0x00000040U ,0x00000040U)
	*/
	PSU_Mask_Write(SERDES_L0_TM_ANA_BYP_12_OFFSET,
	               0x00000040U, 0x00000040U);
	/*##################################################################### */

	/*
	* Register : L1_TM_RST_DLY @ 0XFD4059A4

	* Delay apb reset by specified amount
	*  PSU_SERDES_L1_TM_RST_DLY_APB_RST_DLY                        0xFF

	* reset delay for apb reset w.r.t pso of hsrx
	* (OFFSET, MASK, VALUE)      (0XFD4059A4, 0x000000FFU ,0x000000FFU)
	*/
	PSU_Mask_Write(SERDES_L1_TM_RST_DLY_OFFSET,
	               0x000000FFU, 0x000000FFU);
	/*##################################################################### */

	/*
	* Register : L1_TM_ANA_BYP_15 @ 0XFD405038

	* Enable Bypass for <7> of TM_ANA_BYPS_15
	*  PSU_SERDES_L1_TM_ANA_BYP_15_FORCE_UPHY_ENABLE_LOW_LEAKAGE   0x1

	* Bypass control for pcs-pma interface. EQ supplies, main master supply an
	* d ps for samp c2c
	* (OFFSET, MASK, VALUE)      (0XFD405038, 0x00000040U ,0x00000040U)
	*/
	PSU_Mask_Write(SERDES_L1_TM_ANA_BYP_15_OFFSET,
	               0x00000040U, 0x00000040U);
	/*##################################################################### */

	/*
	* Register : L1_TM_ANA_BYP_12 @ 0XFD40502C

	* Enable Bypass for <7> of TM_ANA_BYPS_12
	*  PSU_SERDES_L1_TM_ANA_BYP_12_FORCE_UPHY_PSO_HSRXDIG          0x1

	* Bypass control for pcs-pma interface. Hsrx supply, hsrx des, and cdr ena
	* ble controls
	* (OFFSET, MASK, VALUE)      (0XFD40502C, 0x00000040U ,0x00000040U)
	*/
	PSU_Mask_Write(SERDES_L1_TM_ANA_BYP_12_OFFSET,
	               0x00000040U, 0x00000040U);
	/*##################################################################### */

	/*
	* Register : L2_TM_RST_DLY @ 0XFD4099A4

	* Delay apb reset by specified amount
	*  PSU_SERDES_L2_TM_RST_DLY_APB_RST_DLY                        0xFF

	* reset delay for apb reset w.r.t pso of hsrx
	* (OFFSET, MASK, VALUE)      (0XFD4099A4, 0x000000FFU ,0x000000FFU)
	*/
	PSU_Mask_Write(SERDES_L2_TM_RST_DLY_OFFSET,
	               0x000000FFU, 0x000000FFU);
	/*##################################################################### */

	/*
	* Register : L2_TM_ANA_BYP_15 @ 0XFD409038

	* Enable Bypass for <7> of TM_ANA_BYPS_15
	*  PSU_SERDES_L2_TM_ANA_BYP_15_FORCE_UPHY_ENABLE_LOW_LEAKAGE   0x1

	* Bypass control for pcs-pma interface. EQ supplies, main master supply an
	* d ps for samp c2c
	* (OFFSET, MASK, VALUE)      (0XFD409038, 0x00000040U ,0x00000040U)
	*/
	PSU_Mask_Write(SERDES_L2_TM_ANA_BYP_15_OFFSET,
	               0x00000040U, 0x00000040U);
	/*##################################################################### */

	/*
	* Register : L2_TM_ANA_BYP_12 @ 0XFD40902C

	* Enable Bypass for <7> of TM_ANA_BYPS_12
	*  PSU_SERDES_L2_TM_ANA_BYP_12_FORCE_UPHY_PSO_HSRXDIG          0x1

	* Bypass control for pcs-pma interface. Hsrx supply, hsrx des, and cdr ena
	* ble controls
	* (OFFSET, MASK, VALUE)      (0XFD40902C, 0x00000040U ,0x00000040U)
	*/
	PSU_Mask_Write(SERDES_L2_TM_ANA_BYP_12_OFFSET,
	               0x00000040U, 0x00000040U);
	/*##################################################################### */

	/*
	* Register : L3_TM_RST_DLY @ 0XFD40D9A4

	* Delay apb reset by specified amount
	*  PSU_SERDES_L3_TM_RST_DLY_APB_RST_DLY                        0xFF

	* reset delay for apb reset w.r.t pso of hsrx
	* (OFFSET, MASK, VALUE)      (0XFD40D9A4, 0x000000FFU ,0x000000FFU)
	*/
	PSU_Mask_Write(SERDES_L3_TM_RST_DLY_OFFSET,
	               0x000000FFU, 0x000000FFU);
	/*##################################################################### */

	/*
	* Register : L3_TM_ANA_BYP_15 @ 0XFD40D038

	* Enable Bypass for <7> of TM_ANA_BYPS_15
	*  PSU_SERDES_L3_TM_ANA_BYP_15_FORCE_UPHY_ENABLE_LOW_LEAKAGE   0x1

	* Bypass control for pcs-pma interface. EQ supplies, main master supply an
	* d ps for samp c2c
	* (OFFSET, MASK, VALUE)      (0XFD40D038, 0x00000040U ,0x00000040U)
	*/
	PSU_Mask_Write(SERDES_L3_TM_ANA_BYP_15_OFFSET,
	               0x00000040U, 0x00000040U);
	/*##################################################################### */

	/*
	* Register : L3_TM_ANA_BYP_12 @ 0XFD40D02C

	* Enable Bypass for <7> of TM_ANA_BYPS_12
	*  PSU_SERDES_L3_TM_ANA_BYP_12_FORCE_UPHY_PSO_HSRXDIG          0x1

	* Bypass control for pcs-pma interface. Hsrx supply, hsrx des, and cdr ena
	* ble controls
	* (OFFSET, MASK, VALUE)      (0XFD40D02C, 0x00000040U ,0x00000040U)
	*/
	PSU_Mask_Write(SERDES_L3_TM_ANA_BYP_12_OFFSET,
	               0x00000040U, 0x00000040U);
	/*##################################################################### */

	/*
	* DISABLE FPL/FFL
	*/
	/*
	* Register : L0_TM_MISC3 @ 0XFD4019AC

	* CDR fast phase lock control
	*  PSU_SERDES_L0_TM_MISC3_CDR_EN_FPL                           0x0

	* CDR fast frequency lock control
	*  PSU_SERDES_L0_TM_MISC3_CDR_EN_FFL                           0x0

	* debug bus selection bit, cdr fast phase and freq controls
	* (OFFSET, MASK, VALUE)      (0XFD4019AC, 0x00000003U ,0x00000000U)
	*/
	PSU_Mask_Write(SERDES_L0_TM_MISC3_OFFSET, 0x00000003U, 0x00000000U);
	/*##################################################################### */

	/*
	* Register : L1_TM_MISC3 @ 0XFD4059AC

	* CDR fast phase lock control
	*  PSU_SERDES_L1_TM_MISC3_CDR_EN_FPL                           0x0

	* CDR fast frequency lock control
	*  PSU_SERDES_L1_TM_MISC3_CDR_EN_FFL                           0x0

	* debug bus selection bit, cdr fast phase and freq controls
	* (OFFSET, MASK, VALUE)      (0XFD4059AC, 0x00000003U ,0x00000000U)
	*/
	PSU_Mask_Write(SERDES_L1_TM_MISC3_OFFSET, 0x00000003U, 0x00000000U);
	/*##################################################################### */

	/*
	* Register : L2_TM_MISC3 @ 0XFD4099AC

	* CDR fast phase lock control
	*  PSU_SERDES_L2_TM_MISC3_CDR_EN_FPL                           0x0

	* CDR fast frequency lock control
	*  PSU_SERDES_L2_TM_MISC3_CDR_EN_FFL                           0x0

	* debug bus selection bit, cdr fast phase and freq controls
	* (OFFSET, MASK, VALUE)      (0XFD4099AC, 0x00000003U ,0x00000000U)
	*/
	PSU_Mask_Write(SERDES_L2_TM_MISC3_OFFSET, 0x00000003U, 0x00000000U);
	/*##################################################################### */

	/*
	* Register : L3_TM_MISC3 @ 0XFD40D9AC

	* CDR fast phase lock control
	*  PSU_SERDES_L3_TM_MISC3_CDR_EN_FPL                           0x0

	* CDR fast frequency lock control
	*  PSU_SERDES_L3_TM_MISC3_CDR_EN_FFL                           0x0

	* debug bus selection bit, cdr fast phase and freq controls
	* (OFFSET, MASK, VALUE)      (0XFD40D9AC, 0x00000003U ,0x00000000U)
	*/
	PSU_Mask_Write(SERDES_L3_TM_MISC3_OFFSET, 0x00000003U, 0x00000000U);
	/*##################################################################### */
	/*
	* DISABLE DYNAMIC OFFSET CALIBRATION
	*/
	/*
	* Register : L0_TM_EQ11 @ 0XFD401978

	* Force EQ offset correction algo off if not forced on
	*  PSU_SERDES_L0_TM_EQ11_FORCE_EQ_OFFS_OFF                     0x1

	* eq dynamic offset correction
	* (OFFSET, MASK, VALUE)      (0XFD401978, 0x00000010U ,0x00000010U)
	*/
	PSU_Mask_Write(SERDES_L0_TM_EQ11_OFFSET, 0x00000010U, 0x00000010U);
	/*##################################################################### */

	/*
	* Register : L1_TM_EQ11 @ 0XFD405978

	* Force EQ offset correction algo off if not forced on
	*  PSU_SERDES_L1_TM_EQ11_FORCE_EQ_OFFS_OFF                     0x1

	* eq dynamic offset correction
	* (OFFSET, MASK, VALUE)      (0XFD405978, 0x00000010U ,0x00000010U)
	*/
	PSU_Mask_Write(SERDES_L1_TM_EQ11_OFFSET, 0x00000010U, 0x00000010U);
	/*##################################################################### */

	/*
	* Register : L2_TM_EQ11 @ 0XFD409978

	* Force EQ offset correction algo off if not forced on
	*  PSU_SERDES_L2_TM_EQ11_FORCE_EQ_OFFS_OFF                     0x1

	* eq dynamic offset correction
	* (OFFSET, MASK, VALUE)      (0XFD409978, 0x00000010U ,0x00000010U)
	*/
	PSU_Mask_Write(SERDES_L2_TM_EQ11_OFFSET, 0x00000010U, 0x00000010U);
	/*##################################################################### */

	/*
	* Register : L3_TM_EQ11 @ 0XFD40D978

	* Force EQ offset correction algo off if not forced on
	*  PSU_SERDES_L3_TM_EQ11_FORCE_EQ_OFFS_OFF                     0x1

	* eq dynamic offset correction
	* (OFFSET, MASK, VALUE)      (0XFD40D978, 0x00000010U ,0x00000010U)
	*/
	PSU_Mask_Write(SERDES_L3_TM_EQ11_OFFSET, 0x00000010U, 0x00000010U);
	/*##################################################################### */
	/*
	* SERDES ILL CALIB
	*/
	//serdes_illcalib(0,0,3,0,4,0,4,0);
	PSU_Mask_Write(SERDES_L2_TM_IQ_ILL8_OFFSET, 0x000000FFU, 0xF3);
	PSU_Mask_Write(SERDES_L2_TM_E_ILL8_OFFSET, 0x000000FFU, 0xF3);
	PSU_Mask_Write(SERDES_L2_TM_ILL12_OFFSET, 0x000000FFU, 0x20);
	PSU_Mask_Write(SERDES_L2_TM_E_ILL1_OFFSET, 0x000000FFU, 0x37);
//	HW_REG_WRITE1(SERDES, L2_TM_IQ_ILL8, 0xF3);
//	HW_REG_WRITE1(SERDES, L2_TM_E_ILL8, 0xF3);
//	HW_REG_WRITE1(SERDES, L2_TM_ILL12, 0x20);
//	HW_REG_WRITE1(SERDES, L2_TM_E_ILL1, 0x37);


	/*##################################################################### */

	/*
	* DISABLE ECO FOR PCIE
	*/
	/*
	* GT LANE SETTINGS
	*/
	/*
	* Register : ICM_CFG0 @ 0XFD410010

	* Controls UPHY Lane 0 protocol configuration. 0 - PowerDown, 1 - PCIe .0,
	*  2 - Sata0, 3 - USB0, 4 - DP.1, 5 - SGMII0, 6 - Unused, 7 - Unused
	*  PSU_SERDES_ICM_CFG0_L0_ICM_CFG                              4

	* Controls UPHY Lane 1 protocol configuration. 0 - PowerDown, 1 - PCIe.1,
	* 2 - Sata1, 3 - USB0, 4 - DP.0, 5 - SGMII1, 6 - Unused, 7 - Unused
	*  PSU_SERDES_ICM_CFG0_L1_ICM_CFG                              4

	* ICM Configuration Register 0
	* (OFFSET, MASK, VALUE)      (0XFD410010, 0x00000077U ,0x00000044U)
	*/
	PSU_Mask_Write(SERDES_ICM_CFG0_OFFSET, 0x00000077U, 0x00000044U);
	/*##################################################################### */

	/*
	* Register : ICM_CFG1 @ 0XFD410014

	* Controls UPHY Lane 2 protocol configuration. 0 - PowerDown, 1 - PCIe.1,
	* 2 - Sata0, 3 - USB0, 4 - DP.1, 5 - SGMII2, 6 - Unused, 7 - Unused
	*  PSU_SERDES_ICM_CFG1_L2_ICM_CFG                              3

	* ICM Configuration Register 1
	* (OFFSET, MASK, VALUE)      (0XFD410014, 0x00000007U ,0x00000003U)
	*/
	PSU_Mask_Write(SERDES_ICM_CFG1_OFFSET, 0x00000007U, 0x00000003U);
	/*##################################################################### */

	/*
	* CHECKING PLL LOCK
	*/
	/*
	* ENABLE SERIAL DATA MUX DEEMPH
	*/
	/*
	* Register : L0_TXPMD_TM_45 @ 0XFD400CB4

	* Enable/disable DP post2 path
	*  PSU_SERDES_L0_TXPMD_TM_45_DP_TM_TX_DP_ENABLE_POST2_PATH     0x1

	* Override enable/disable of DP post2 path
	*  PSU_SERDES_L0_TXPMD_TM_45_DP_TM_TX_OVRD_DP_ENABLE_POST2_PATH 0x1

	* Override enable/disable of DP post1 path
	*  PSU_SERDES_L0_TXPMD_TM_45_DP_TM_TX_OVRD_DP_ENABLE_POST1_PATH 0x1

	* Enable/disable DP main path
	*  PSU_SERDES_L0_TXPMD_TM_45_DP_TM_TX_DP_ENABLE_MAIN_PATH      0x1

	* Override enable/disable of DP main path
	*  PSU_SERDES_L0_TXPMD_TM_45_DP_TM_TX_OVRD_DP_ENABLE_MAIN_PATH 0x1

	* Post or pre or main DP path selection
	* (OFFSET, MASK, VALUE)      (0XFD400CB4, 0x00000037U ,0x00000037U)
	*/
	PSU_Mask_Write(SERDES_L0_TXPMD_TM_45_OFFSET,
	               0x00000037U, 0x00000037U);
	/*##################################################################### */

	/*
	* Register : L1_TXPMD_TM_45 @ 0XFD404CB4

	* Enable/disable DP post2 path
	*  PSU_SERDES_L1_TXPMD_TM_45_DP_TM_TX_DP_ENABLE_POST2_PATH     0x1

	* Override enable/disable of DP post2 path
	*  PSU_SERDES_L1_TXPMD_TM_45_DP_TM_TX_OVRD_DP_ENABLE_POST2_PATH 0x1

	* Override enable/disable of DP post1 path
	*  PSU_SERDES_L1_TXPMD_TM_45_DP_TM_TX_OVRD_DP_ENABLE_POST1_PATH 0x1

	* Enable/disable DP main path
	*  PSU_SERDES_L1_TXPMD_TM_45_DP_TM_TX_DP_ENABLE_MAIN_PATH      0x1

	* Override enable/disable of DP main path
	*  PSU_SERDES_L1_TXPMD_TM_45_DP_TM_TX_OVRD_DP_ENABLE_MAIN_PATH 0x1

	* Post or pre or main DP path selection
	* (OFFSET, MASK, VALUE)      (0XFD404CB4, 0x00000037U ,0x00000037U)
	*/
	PSU_Mask_Write(SERDES_L1_TXPMD_TM_45_OFFSET,
	               0x00000037U, 0x00000037U);
	/*##################################################################### */

	/*
	* Register : L0_TX_ANA_TM_118 @ 0XFD4001D8

	* Test register force for enabling/disablign TX deemphasis bits <17:0>
	*  PSU_SERDES_L0_TX_ANA_TM_118_FORCE_TX_DEEMPH_17_0            0x1

	* Enable Override of TX deemphasis
	* (OFFSET, MASK, VALUE)      (0XFD4001D8, 0x00000001U ,0x00000001U)
	*/
	PSU_Mask_Write(SERDES_L0_TX_ANA_TM_118_OFFSET,
	               0x00000001U, 0x00000001U);
	/*##################################################################### */

	/*
	* Register : L1_TX_ANA_TM_118 @ 0XFD4041D8

	* Test register force for enabling/disablign TX deemphasis bits <17:0>
	*  PSU_SERDES_L1_TX_ANA_TM_118_FORCE_TX_DEEMPH_17_0            0x1

	* Enable Override of TX deemphasis
	* (OFFSET, MASK, VALUE)      (0XFD4041D8, 0x00000001U ,0x00000001U)
	*/
	PSU_Mask_Write(SERDES_L1_TX_ANA_TM_118_OFFSET,
	               0x00000001U, 0x00000001U);
	/*##################################################################### */

	/*
	* CDR AND RX EQUALIZATION SETTINGS
	*/
	/*
	* GEM SERDES SETTINGS
	*/
	/*
	* ENABLE PRE EMPHAIS AND VOLTAGE SWING
	*/
	/*
	* Register : L1_TXPMD_TM_48 @ 0XFD404CC0

	* Margining factor value
	*  PSU_SERDES_L1_TXPMD_TM_48_TM_RESULTANT_MARGINING_FACTOR     0

	* Margining factor
	* (OFFSET, MASK, VALUE)      (0XFD404CC0, 0x0000001FU ,0x00000000U)
	*/
	PSU_Mask_Write(SERDES_L1_TXPMD_TM_48_OFFSET,
	               0x0000001FU, 0x00000000U);
	/*##################################################################### */

	/*
	* Register : L0_TXPMD_TM_48 @ 0XFD400CC0

	* Margining factor value
	*  PSU_SERDES_L0_TXPMD_TM_48_TM_RESULTANT_MARGINING_FACTOR     0

	* Margining factor
	* (OFFSET, MASK, VALUE)      (0XFD400CC0, 0x0000001FU ,0x00000000U)
	*/
	PSU_Mask_Write(SERDES_L0_TXPMD_TM_48_OFFSET,
	               0x0000001FU, 0x00000000U);
	/*##################################################################### */

	/*
	* Register : L1_TX_ANA_TM_18 @ 0XFD404048

	* pipe_TX_Deemph. 0: -6dB de-emphasis, 1: -3.5dB de-emphasis, 2 : No de-em
	* phasis, Others: reserved
	*  PSU_SERDES_L1_TX_ANA_TM_18_PIPE_TX_DEEMPH_7_0               0

	* Override for PIPE TX de-emphasis
	* (OFFSET, MASK, VALUE)      (0XFD404048, 0x000000FFU ,0x00000000U)
	*/
	PSU_Mask_Write(SERDES_L1_TX_ANA_TM_18_OFFSET,
	               0x000000FFU, 0x00000000U);
	/*##################################################################### */

	/*
	* Register : L0_TX_ANA_TM_18 @ 0XFD400048

	* pipe_TX_Deemph. 0: -6dB de-emphasis, 1: -3.5dB de-emphasis, 2 : No de-em
	* phasis, Others: reserved
	*  PSU_SERDES_L0_TX_ANA_TM_18_PIPE_TX_DEEMPH_7_0               0

	* Override for PIPE TX de-emphasis
	* (OFFSET, MASK, VALUE)      (0XFD400048, 0x000000FFU ,0x00000000U)
	*/
	PSU_Mask_Write(SERDES_L0_TX_ANA_TM_18_OFFSET,
	               0x000000FFU, 0x00000000U);
	/*##################################################################### */

	return 1;
}

void psu_resetin_init_data(void)
{
	/*
	* PUTTING SERDES PERIPHERAL IN RESET
	*/
	/*
	* PUTTING USB0 IN RESET
	*/
	/*
	* Register : RST_LPD_TOP @ 0XFF5E023C

	* USB 0 reset for control registers
	*  PSU_CRL_APB_RST_LPD_TOP_USB0_APB_RESET                      0X1

	* USB 0 sleep circuit reset
	*  PSU_CRL_APB_RST_LPD_TOP_USB0_HIBERRESET                     0X1

	* USB 0 reset
	*  PSU_CRL_APB_RST_LPD_TOP_USB0_CORERESET                      0X1

	* Software control register for the LPD block.
	* (OFFSET, MASK, VALUE)      (0XFF5E023C, 0x00000540U ,0x00000540U)
	*/
	HW_REG_RMW1(CRL_APB, RST_LPD_TOP, 0x00000540U, 0x00000540U);
	/*##################################################################### */

	/*
	* PUTTING GEM0 IN RESET
	*/
	/*
	* Register : RST_LPD_IOU0 @ 0XFF5E0230

	* GEM 3 reset
	*  PSU_CRL_APB_RST_LPD_IOU0_GEM3_RESET                         0X1

	* Software controlled reset for the GEMs
	* (OFFSET, MASK, VALUE)      (0XFF5E0230, 0x00000008U ,0x00000008U)
	*/
	HW_REG_RMW1(CRL_APB, RST_LPD_IOU0, 0x00000008U, 0x00000008U);
	/*##################################################################### */

	/*
	* PUTTING DP IN RESET
	*/
	/*
	* Register : DP_TX_PHY_POWER_DOWN @ 0XFD4A0238

	* Two bits per lane. When set to 11, moves the GT to power down mode. When
	*  set to 00, GT will be in active state. bits [1:0] - lane0 Bits [3:2] -
	* lane 1
	*  PSU_DP_DP_TX_PHY_POWER_DOWN_POWER_DWN                       0XA

	* Control PHY Power down
	* (OFFSET, MASK, VALUE)      (0XFD4A0238, 0x0000000FU ,0x0000000AU)
	*/
	HW_REG_RMW1(DP, TX_PHY_POWER_DOWN, 0x0000000FU, 0x0000000AU);
	/*##################################################################### */

	/*
	* Register : DP_PHY_RESET @ 0XFD4A0200

	* Set to '1' to hold the GT in reset. Clear to release.
	*  PSU_DP_DP_PHY_RESET_GT_RESET                                0X1

	* Reset the transmitter PHY.
	* (OFFSET, MASK, VALUE)      (0XFD4A0200, 0x00000002U ,0x00000002U)
	*/
	HW_REG_RMW1(DP, PHY_RESET, 0x00000002U, 0x00000002U);
	/*##################################################################### */

	/*
	* Register : RST_FPD_TOP @ 0XFD1A0100

	* Display Port block level reset (includes DPDMA)
	*  PSU_CRF_APB_RST_FPD_TOP_DP_RESET                            0X1

	* FPD Block level software controlled reset
	* (OFFSET, MASK, VALUE)      (0XFD1A0100, 0x00010000U ,0x00010000U)
	*/
	HW_REG_RMW1(CRF_APB, RST_FPD_TOP, 0x00010000U, 0x00010000U);
	/*##################################################################### */
}

unsigned long psu_resetout_init_data(void)
{
	/*
	* TAKING SERDES PERIPHERAL OUT OF RESET RESET
	*/
	/*
	* PUTTING USB0 IN RESET
	*/
	/*
	* Register : RST_LPD_TOP @ 0XFF5E023C

	* USB 0 reset for control registers
	*  PSU_CRL_APB_RST_LPD_TOP_USB0_APB_RESET                      0X0

	* Software control register for the LPD block.
	* (OFFSET, MASK, VALUE)      (0XFF5E023C, 0x00000400U ,0x00000000U)
	*/
	PSU_Mask_Write(CRL_APB_RST_LPD_TOP_OFFSET, 0x00000400U, 0x00000000U);
	/*##################################################################### */

	/*
	* USB0 PIPE POWER PRESENT
	*/
	/*
	* Register : fpd_power_prsnt @ 0XFF9D0080

	* This bit is used to choose between PIPE power present and 1'b1
	*  PSU_USB3_0_FPD_POWER_PRSNT_OPTION                           0X1

	* fpd_power_prsnt
	* (OFFSET, MASK, VALUE)      (0XFF9D0080, 0x00000001U ,0x00000001U)
	*/
	PSU_Mask_Write(USB3_0_FPD_POWER_PRSNT_OFFSET,
								 0x00000001U, 0x00000001U);
	/*##################################################################### */

	/*
	* Register : fpd_pipe_clk @ 0XFF9D007C

	* This bit is used to choose between PIPE clock coming from SerDes and the
	*  suspend clk
	*  PSU_USB3_0_FPD_PIPE_CLK_OPTION                              0x0

	* fpd_pipe_clk
	* (OFFSET, MASK, VALUE)      (0XFF9D007C, 0x00000001U ,0x00000000U)
	*/
	PSU_Mask_Write(USB3_0_FPD_PIPE_CLK_OFFSET, 0x00000001U, 0x00000000U);
	/*##################################################################### */

	/*
	* HIBERREST
	*/
	/*
	* Register : RST_LPD_TOP @ 0XFF5E023C

	* USB 0 sleep circuit reset
	*  PSU_CRL_APB_RST_LPD_TOP_USB0_HIBERRESET                     0X0

	* USB 0 reset
	*  PSU_CRL_APB_RST_LPD_TOP_USB0_CORERESET                      0X0

	* Software control register for the LPD block.
	* (OFFSET, MASK, VALUE)      (0XFF5E023C, 0x00000140U ,0x00000000U)
	*/
	PSU_Mask_Write(CRL_APB_RST_LPD_TOP_OFFSET, 0x00000140U, 0x00000000U);
	/*##################################################################### */

	/*
	* PUTTING GEM0 IN RESET
	*/
	/*
	* Register : RST_LPD_IOU0 @ 0XFF5E0230

	* GEM 3 reset
	*  PSU_CRL_APB_RST_LPD_IOU0_GEM3_RESET                         0X0

	* Software controlled reset for the GEMs
	* (OFFSET, MASK, VALUE)      (0XFF5E0230, 0x00000008U ,0x00000000U)
	*/
	PSU_Mask_Write(CRL_APB_RST_LPD_IOU0_OFFSET,
								 0x00000008U, 0x00000000U);
	/*##################################################################### */

	/*
	* PUTTING DP IN RESET
	*/
	/*
	* Register : RST_FPD_TOP @ 0XFD1A0100

	* Display Port block level reset (includes DPDMA)
	*  PSU_CRF_APB_RST_FPD_TOP_DP_RESET                            0X0

	* FPD Block level software controlled reset
	* (OFFSET, MASK, VALUE)      (0XFD1A0100, 0x00010000U ,0x00000000U)
	*/
	PSU_Mask_Write(CRF_APB_RST_FPD_TOP_OFFSET, 0x00010000U, 0x00000000U);
	/*##################################################################### */

	/*
	* Register : DP_PHY_RESET @ 0XFD4A0200

	* Set to '1' to hold the GT in reset. Clear to release.
	*  PSU_DP_DP_PHY_RESET_GT_RESET                                0X0

	* Reset the transmitter PHY.
	* (OFFSET, MASK, VALUE)      (0XFD4A0200, 0x00000002U ,0x00000000U)
	*/
	PSU_Mask_Write(DP_DP_PHY_RESET_OFFSET, 0x00000002U, 0x00000000U);
	/*##################################################################### */

	/*
	* Register : DP_TX_PHY_POWER_DOWN @ 0XFD4A0238

	* Two bits per lane. When set to 11, moves the GT to power down mode. When
	*  set to 00, GT will be in active state. bits [1:0] - lane0 Bits [3:2] -
	* lane 1
	*  PSU_DP_DP_TX_PHY_POWER_DOWN_POWER_DWN                       0X0

	* Control PHY Power down
	* (OFFSET, MASK, VALUE)      (0XFD4A0238, 0x0000000FU ,0x00000000U)
	*/
	PSU_Mask_Write(DP_DP_TX_PHY_POWER_DOWN_OFFSET,
								 0x0000000FU, 0x00000000U);
	/*##################################################################### */

	/*
	* USB0 GFLADJ
	*/
	/*
	* Register : GUSB2PHYCFG @ 0XFE20C200

	* USB 2.0 Turnaround Time (USBTrdTim) Sets the turnaround time in PHY cloc
	* ks. Specifies the response time for a MAC request to the Packet FIFO Con
	* troller (PFC) to fetch data from the DFIFO (SPRAM). The following are th
	* e required values for the minimum SoC bus frequency of 60 MHz. USB turna
	* round time is a critical certification criteria when using long cables a
	* nd five hub levels. The required values for this field: - 4'h5: When the
	*  MAC interface is 16-bit UTMI+. - 4'h9: When the MAC interface is 8-bit
	* UTMI+/ULPI. If SoC bus clock is less than 60 MHz, and USB turnaround tim
	* e is not critical, this field can be set to a larger value. Note: This f
	* ield is valid only in device mode.
	*  PSU_USB3_0_XHCI_GUSB2PHYCFG_USBTRDTIM                       0x9

	* Transceiver Delay: Enables a delay between the assertion of the UTMI/ULP
	* I Transceiver Select signal (for HS) and the assertion of the TxValid si
	* gnal during a HS Chirp. When this bit is set to 1, a delay (of approxima
	* tely 2.5 us) is introduced from the time when the Transceiver Select is
	* set to 2'b00 (HS) to the time the TxValid is driven to 0 for sending the
	*  chirp-K. This delay is required for some UTMI/ULPI PHYs. Note: - If you
	*  enable the hibernation feature when the device core comes out of power-
	* off, you must re-initialize this bit with the appropriate value because
	* the core does not save and restore this bit value during hibernation. -
	* This bit is valid only in device mode.
	*  PSU_USB3_0_XHCI_GUSB2PHYCFG_XCVRDLY                         0x0

	* Enable utmi_sleep_n and utmi_l1_suspend_n (EnblSlpM) The application use
	* s this bit to control utmi_sleep_n and utmi_l1_suspend_n assertion to th
	* e PHY in the L1 state. - 1'b0: utmi_sleep_n and utmi_l1_suspend_n assert
	* ion from the core is not transferred to the external PHY. - 1'b1: utmi_s
	* leep_n and utmi_l1_suspend_n assertion from the core is transferred to t
	* he external PHY. Note: This bit must be set high for Port0 if PHY is use
	* d. Note: In Device mode - Before issuing any device endpoint command whe
	* n operating in 2.0 speeds, disable this bit and enable it after the comm
	* and completes. Without disabling this bit, if a command is issued when t
	* he device is in L1 state and if mac2_clk (utmi_clk/ulpi_clk) is gated of
	* f, the command will not get completed.
	*  PSU_USB3_0_XHCI_GUSB2PHYCFG_ENBLSLPM                        0x0

	* USB 2.0 High-Speed PHY or USB 1.1 Full-Speed Serial Transceiver Select T
	* he application uses this bit to select a high-speed PHY or a full-speed
	* transceiver. - 1'b0: USB 2.0 high-speed UTMI+ or ULPI PHY. This bit is a
	* lways 0, with Write Only access. - 1'b1: USB 1.1 full-speed serial trans
	* ceiver. This bit is always 1, with Write Only access. If both interface
	* types are selected in coreConsultant (that is, parameters' values are no
	* t zero), the application uses this bit to select the active interface is
	*  active, with Read-Write bit access. Note: USB 1.1 full-serial transceiv
	* er is not supported. This bit always reads as 1'b0.
	*  PSU_USB3_0_XHCI_GUSB2PHYCFG_PHYSEL                          0x0

	* Suspend USB2.0 HS/FS/LS PHY (SusPHY) When set, USB2.0 PHY enters Suspend
	*  mode if Suspend conditions are valid. For DRD/OTG configurations, it is
	*  recommended that this bit is set to 0 during coreConsultant configurati
	* on. If it is set to 1, then the application must clear this bit after po
	* wer-on reset. Application needs to set it to 1 after the core initializa
	* tion completes. For all other configurations, this bit can be set to 1 d
	* uring core configuration. Note: - In host mode, on reset, this bit is se
	* t to 1. Software can override this bit after reset. - In device mode, be
	* fore issuing any device endpoint command when operating in 2.0 speeds, d
	* isable this bit and enable it after the command completes. If you issue
	* a command without disabling this bit when the device is in L2 state and
	* if mac2_clk (utmi_clk/ulpi_clk) is gated off, the command will not get c
	* ompleted.
	*  PSU_USB3_0_XHCI_GUSB2PHYCFG_SUSPENDUSB20                    0x1

	* Full-Speed Serial Interface Select (FSIntf) The application uses this bi
	* t to select a unidirectional or bidirectional USB 1.1 full-speed serial
	* transceiver interface. - 1'b0: 6-pin unidirectional full-speed serial in
	* terface. This bit is set to 0 with Read Only access. - 1'b1: 3-pin bidir
	* ectional full-speed serial interface. This bit is set to 0 with Read Onl
	* y access. Note: USB 1.1 full-speed serial interface is not supported. Th
	* is bit always reads as 1'b0.
	*  PSU_USB3_0_XHCI_GUSB2PHYCFG_FSINTF                          0x0

	* ULPI or UTMI+ Select (ULPI_UTMI_Sel) The application uses this bit to se
	* lect a UTMI+ or ULPI Interface. - 1'b0: UTMI+ Interface - 1'b1: ULPI Int
	* erface This bit is writable only if UTMI+ and ULPI is specified for High
	* -Speed PHY Interface(s) in coreConsultant configuration (DWC_USB3_HSPHY_
	* INTERFACE = 3). Otherwise, this bit is read-only and the value depends o
	* n the interface selected through DWC_USB3_HSPHY_INTERFACE.
	*  PSU_USB3_0_XHCI_GUSB2PHYCFG_ULPI_UTMI_SEL                   0x1

	* PHY Interface (PHYIf) If UTMI+ is selected, the application uses this bi
	* t to configure the core to support a UTMI+ PHY with an 8- or 16-bit inte
	* rface. - 1'b0: 8 bits - 1'b1: 16 bits ULPI Mode: 1'b0 Note: - All the en
	* abled 2.0 ports must have the same clock frequency as Port0 clock freque
	* ncy (utmi_clk[0]). - The UTMI 8-bit and 16-bit modes cannot be used toge
	* ther for different ports at the same time (that is, all the ports must b
	* e in 8-bit mode, or all of them must be in 16-bit mode, at a time). - If
	*  any of the USB 2.0 ports is selected as ULPI port for operation, then a
	* ll the USB 2.0 ports must be operating at 60 MHz.
	*  PSU_USB3_0_XHCI_GUSB2PHYCFG_PHYIF                           0x0

	* HS/FS Timeout Calibration (TOutCal) The number of PHY clocks, as indicat
	* ed by the application in this field, is multiplied by a bit-time factor;
	*  this factor is added to the high-speed/full-speed interpacket timeout d
	* uration in the core to account for additional delays introduced by the P
	* HY. This may be required, since the delay introduced by the PHY in gener
	* ating the linestate condition may vary among PHYs. The USB standard time
	* out value for high-speed operation is 736 to 816 (inclusive) bit times.
	* The USB standard timeout value for full-speed operation is 16 to 18 (inc
	* lusive) bit times. The application must program this field based on the
	* speed of connection. The number of bit times added per PHY clock are: Hi
	* gh-speed operation: - One 30-MHz PHY clock = 16 bit times - One 60-MHz P
	* HY clock = 8 bit times Full-speed operation: - One 30-MHz PHY clock = 0.
	* 4 bit times - One 60-MHz PHY clock = 0.2 bit times - One 48-MHz PHY cloc
	* k = 0.25 bit times
	*  PSU_USB3_0_XHCI_GUSB2PHYCFG_TOUTCAL                         0x7

	* ULPI External VBUS Drive (ULPIExtVbusDrv) Selects supply source to drive
	*  5V on VBUS, in the ULPI PHY. - 1'b0: PHY drives VBUS with internal char
	* ge pump (default). - 1'b1: PHY drives VBUS with an external supply. (Onl
	* y when RTL parameter DWC_USB3_HSPHY_INTERFACE = 2 or 3)
	*  PSU_USB3_0_XHCI_GUSB2PHYCFG_ULPIEXTVBUSDRV                  0x1

	* Global USB2 PHY Configuration Register The application must program this
	*  register before starting any transactions on either the SoC bus or the
	* USB. In Device-only configurations, only one register is needed. In Host
	*  mode, per-port registers are implemented.
	* (OFFSET, MASK, VALUE)      (0XFE20C200, 0x00023FFFU ,0x00022457U)
	*/
	PSU_Mask_Write(USB3_0_XHCI_GUSB2PHYCFG_OFFSET,
								 0x00023FFFU, 0x00022457U);
	/*##################################################################### */

	/*
	* Register : GFLADJ @ 0XFE20C630

	* This field indicates the frame length adjustment to be applied when SOF/
	* ITP counter is running on the ref_clk. This register value is used to ad
	* just the ITP interval when GCTL[SOFITPSYNC] is set to '1'; SOF and ITP i
	* nterval when GLADJ.GFLADJ_REFCLK_LPM_SEL is set to '1'. This field must
	* be programmed to a non-zero value only if GFLADJ_REFCLK_LPM_SEL is set t
	* o '1' or GCTL.SOFITPSYNC is set to '1'. The value is derived as follows:
	*  FLADJ_REF_CLK_FLADJ=((125000/ref_clk_period_integer)-(125000/ref_clk_pe
	* riod)) * ref_clk_period where - the ref_clk_period_integer is the intege
	* r value of the ref_clk period got by truncating the decimal (fractional)
	*  value that is programmed in the GUCTL.REF_CLK_PERIOD field. - the ref_c
	* lk_period is the ref_clk period including the fractional value. Examples
	* : If the ref_clk is 24 MHz then - GUCTL.REF_CLK_PERIOD = 41 - GFLADJ.GLA
	* DJ_REFCLK_FLADJ = ((125000/41)-(125000/41.6666))*41.6666 = 2032 (ignorin
	* g the fractional value) If the ref_clk is 48 MHz then - GUCTL.REF_CLK_PE
	* RIOD = 20 - GFLADJ.GLADJ_REFCLK_FLADJ = ((125000/20)-(125000/20.8333))*2
	* 0.8333 = 5208 (ignoring the fractional value)
	*  PSU_USB3_0_XHCI_GFLADJ_GFLADJ_REFCLK_FLADJ                  0x0

	* Global Frame Length Adjustment Register This register provides options f
	* or the software to control the core behavior with respect to SOF (Start
	* of Frame) and ITP (Isochronous Timestamp Packet) timers and frame timer
	* functionality. It provides an option to override the fladj_30mhz_reg sid
	* eband signal. In addition, it enables running SOF or ITP frame timer cou
	* nters completely from the ref_clk. This facilitates hardware LPM in host
	*  mode with the SOF or ITP counters being run from the ref_clk signal.
	* (OFFSET, MASK, VALUE)      (0XFE20C630, 0x003FFF00U ,0x00000000U)
	*/
	PSU_Mask_Write(USB3_0_XHCI_GFLADJ_OFFSET, 0x003FFF00U, 0x00000000U);
	/*##################################################################### */

	/*
	* Register : GUCTL1 @ 0XFE20C11C

	* When this bit is set to '0', termsel, xcvrsel will become 0 during end o
	* f resume while the opmode will become 0 once controller completes end of
	*  resume and enters U0 state (2 separate commandswill be issued). When th
	* is bit is set to '1', all the termsel, xcvrsel, opmode becomes 0 during
	* end of resume itself (only 1 command will be issued)
	*  PSU_USB3_0_XHCI_GUCTL1_RESUME_TERMSEL_XCVRSEL_UNIFY         0x1

	* Reserved
	*  PSU_USB3_0_XHCI_GUCTL1_RESERVED_9                           0x1

	* Global User Control Register 1
	* (OFFSET, MASK, VALUE)      (0XFE20C11C, 0x00000600U ,0x00000600U)
	*/
	PSU_Mask_Write(USB3_0_XHCI_GUCTL1_OFFSET, 0x00000600U, 0x00000600U);
	/*##################################################################### */

	/*
	* Register : GUCTL @ 0XFE20C12C

	* Host IN Auto Retry (USBHstInAutoRetryEn) When set, this field enables th
	* e Auto Retry feature. For IN transfers (non-isochronous) that encounter
	* data packets with CRC errors or internal overrun scenarios, the auto ret
	* ry feature causes the Host core to reply to the device with a non-termin
	* ating retry ACK (that is, an ACK transaction packet with Retry = 1 and N
	* umP != 0). If the Auto Retry feature is disabled (default), the core wil
	* l respond with a terminating retry ACK (that is, an ACK transaction pack
	* et with Retry = 1 and NumP = 0). - 1'b0: Auto Retry Disabled - 1'b1: Aut
	* o Retry Enabled Note: This bit is also applicable to the device mode.
	*  PSU_USB3_0_XHCI_GUCTL_USBHSTINAUTORETRYEN                   0x1

	* Global User Control Register: This register provides a few options for t
	* he software to control the core behavior in the Host mode. Most of the o
	* ptions are used to improve host inter-operability with different devices
	* .
	* (OFFSET, MASK, VALUE)      (0XFE20C12C, 0x00004000U ,0x00004000U)
	*/
	PSU_Mask_Write(USB3_0_XHCI_GUCTL_OFFSET, 0x00004000U, 0x00004000U);
	/*##################################################################### */

	/*
	* UPDATING TWO PCIE REGISTERS DEFAULT VALUES, AS THESE REGISTERS HAVE INCO
	* RRECT RESET VALUES IN SILICON.
	*/
	/*
	* Register : ATTR_25 @ 0XFD480064

	* If TRUE Completion Timeout Disable is supported. This is required to be
	* TRUE for Endpoint and either setting allowed for Root ports. Drives Devi
	* ce Capability 2 [4]; EP=0x0001; RP=0x0001
	*  PSU_PCIE_ATTRIB_ATTR_25_ATTR_CPL_TIMEOUT_DISABLE_SUPPORTED  0X1

	* ATTR_25
	* (OFFSET, MASK, VALUE)      (0XFD480064, 0x00000200U ,0x00000200U)
	*/
	PSU_Mask_Write(PCIE_ATTRIB_ATTR_25_OFFSET, 0x00000200U, 0x00000200U);
	/*##################################################################### */

	/*
	* CHECK PLL LOCK FOR LANE1
	*/
	/*
	* Register : L1_PLL_STATUS_READ_1 @ 0XFD4063E4

	* Status Read value of PLL Lock
	*  PSU_SERDES_L1_PLL_STATUS_READ_1_PLL_LOCK_STATUS_READ        1
	* (OFFSET, MASK, VALUE)      (0XFD4063E4, 0x00000010U ,0x00000010U)
	*/
	mask_poll(SERDES_L1_PLL_STATUS_READ_1_OFFSET, 0x00000010U);

	/*##################################################################### */

	/*
	* CHECK PLL LOCK FOR LANE2
	*/
	/*
	* Register : L2_PLL_STATUS_READ_1 @ 0XFD40A3E4

	* Status Read value of PLL Lock
	*  PSU_SERDES_L2_PLL_STATUS_READ_1_PLL_LOCK_STATUS_READ        1
	* (OFFSET, MASK, VALUE)      (0XFD40A3E4, 0x00000010U ,0x00000010U)
	*/
	mask_poll(SERDES_L2_PLL_STATUS_READ_1_OFFSET, 0x00000010U);

	/*##################################################################### */

	/*
	* SATA AHCI VENDOR SETTING
	*/

	return 1;
}

#endif
