#include "core/core.h"
#include "zynqps8/display_port/display.hpp"
#include "zynqps8/display_port/aux.hpp"
#include "zynqps8/display_port/serdes.hpp"
#include "zynqps8/display_port/train.hpp"
#include "zynqps8/display_port/eedid.hpp"
#include "platform/reg_access.h"
#include "platform/memory_map.h"
#include "platform/registers/dp.h"
#include "platform/registers/dpdma.h"
#include "platform/registers/dp_dpcd.h"
#include "platform/registers/eedid.h"
#include "platform/registers/crf_apb.h"

#include "utils/busy_sleep.h"
#include "utils/boot_psi.h"
#include "dbg/print.h"
#include "core/math.h"
#include "dbg/assert.h"
#include "platform/zynqmp/pll_helper.h"
#include "core/bitops.hpp"

namespace DisplayPort::Display {

#if 0
static void DumpDPCD_0_16(Connection* display) {
	uint8_t dpcd[16];
	debug_print("DumpDPCD_0_16: \n");
	if(AuxRead(display, DP_DPCD_REV_OFFSET, 16, dpcd)){

		switch(dpcd[DP_DPCD_REV_OFFSET]) {
			case DP_DPCD_REV_MJR_DP_REV_1_0: debug_print("DP 1.0\n"); break;
			case DP_DPCD_REV_MJR_DP_REV_1_1: debug_print("DP 1.1\n"); break;
			case DP_DPCD_REV_MJR_DP_REV_1_2: debug_print("DP 1.2\n"); break;
			default: debug_printf("DP REV %d\n",dpcd[DP_DPCD_REV_OFFSET]);
		}

		switch(dpcd[DP_DPCD_MAX_LINK_RATE_OFFSET]) {
			case DP_DPCD_MAX_LINK_RATE_RATE_1_62GBPS: debug_print("1.62Gbps * "); break;
			case DP_DPCD_MAX_LINK_RATE_RATE_2_70GBPS: debug_print("2.70Gbps * "); break;
			case DP_DPCD_MAX_LINK_RATE_RATE_5_40GBPS: debug_print("5.40Gbps * "); break;
		}
		switch (HW_REG_DECODE_FIELD(DP_DPCD, MAX_LANE_COUNT, MAX_LANE_COUNT, dpcd[DP_DPCD_MAX_LANE_COUNT_OFFSET])) {
			case 1: debug_print("1 lane\n"); break;
			case 2: debug_print("2 lanes\n"); break;
			case 3: debug_print("3 lanes\n"); break;
			case 4: debug_print("4 lanes\n"); break;
		}

		debug_print("Lane caps: ");
		if(HW_REG_DECODE_FIELD(DP_DPCD, MAX_LANE_COUNT, ENHANCED_FRAME_SUPPORT, dpcd[DP_DPCD_MAX_LANE_COUNT_OFFSET])) {
			debug_print("Enhanced Frame ");
		}
		if(HW_REG_DECODE_FIELD(DP_DPCD, MAX_LANE_COUNT, TPS3_SUPPORT, dpcd[DP_DPCD_MAX_LANE_COUNT_OFFSET])) {
			debug_print("Training Pattern 3 ");
		}
		if(HW_REG_DECODE_FIELD(DP_DPCD, MAX_DOWNSPREAD, MAX_DOWNSPREAD, dpcd[DP_DPCD_MAX_DOWNSPREAD_OFFSET])) {
			debug_print("Downspread ");
		}
		debug_printf("\nDelayRate (microseconds) %d\n", dpcd[DP_DPCD_TRAIN_AUX_RD_INTERVAL_OFFSET] ? 4000 * dpcd[DP_DPCD_TRAIN_AUX_RD_INTERVAL_OFFSET] : 400);
		if(HW_REG_DECODE_FIELD(DP_DPCD, MAIN_LINK_CHANNEL_CODING, ANSI_8B_10B, dpcd[DP_DPCD_MAIN_LINK_CHANNEL_CODING_OFFSET])) {
			debug_print("Supports ANSI_8B_10B main link coding\n");
		}
		debug_print("Rx Port 0: ");
		debug_printf("buffer size %d ", HW_REG_DECODE_FIELD(DP_DPCD, RX_PORT0_CAP_1, BUFFER_SIZE, dpcd[DP_DPCD_RX_PORT0_CAP_1_OFFSET]));

		if(HW_REG_DECODE_FIELD(DP_DPCD, RX_PORT0_CAP_0, LOCAL_EDID_PRESENT, dpcd[DP_DPCD_RX_PORT0_CAP_0_OFFSET])) {
			debug_print("EDID ");
		}
		if(HW_REG_DECODE_FIELD(DP_DPCD, RX_PORT0_CAP_0, ASSOCIATED_TO_PRECEDING_PORT, dpcd[DP_DPCD_RX_PORT0_CAP_0_OFFSET])) {
			debug_print("is secondary port ");
		}
		debug_print("\n");
		debug_print("Rx Port 1: ");
		debug_printf("buffer size %d ", HW_REG_DECODE_FIELD(DP_DPCD, RX_PORT1_CAP_1, BUFFER_SIZE, dpcd[DP_DPCD_RX_PORT1_CAP_1_OFFSET]));

		if(HW_REG_DECODE_FIELD(DP_DPCD, RX_PORT1_CAP_0, LOCAL_EDID_PRESENT, dpcd[DP_DPCD_RX_PORT1_CAP_0_OFFSET])) {
			debug_print("EDID ");
		}
		if(HW_REG_DECODE_FIELD(DP_DPCD, RX_PORT1_CAP_0, ASSOCIATED_TO_PRECEDING_PORT, dpcd[DP_DPCD_RX_PORT1_CAP_0_OFFSET])) {
			debug_print("is secondary port ");
		}
		debug_print("\n");
		debug_print("I2C Speeds Supported: ");
		uint8_t i2cSpeed = HW_REG_DECODE_FIELD(DP_DPCD, I2C_SPEED_CTL_CAP, I2C_SPEED, dpcd[DP_DPCD_I2C_SPEED_CTL_CAP_OFFSET]);
		if(i2cSpeed == DP_DPCD_I2C_SPEED_CTL_CAP_I2C_SPEED_NOT_SUPPORTED) {
			debug_print("Not Supported");
		} else {
			if(i2cSpeed & DP_DPCD_I2C_SPEED_CTL_CAP_I2C_SPEED_1KBPS) debug_print("1Kbps ");
			if(i2cSpeed & DP_DPCD_I2C_SPEED_CTL_CAP_I2C_SPEED_5KBPS) debug_print("5Kbps ");
			if(i2cSpeed & DP_DPCD_I2C_SPEED_CTL_CAP_I2C_SPEED_10KBPS) debug_print("10Kbps ");
			if(i2cSpeed & DP_DPCD_I2C_SPEED_CTL_CAP_I2C_SPEED_100KBPS) debug_print("100Kbps ");
			if(i2cSpeed & DP_DPCD_I2C_SPEED_CTL_CAP_I2C_SPEED_400KBPS) debug_print("400Kbps ");
			if(i2cSpeed & DP_DPCD_I2C_SPEED_CTL_CAP_I2C_SPEED_1MBPS) debug_print("1Mbps ");
		}
		debug_print("\n");

		debug_print("Train Aux Read Interval: ");
		switch (HW_REG_DECODE_FIELD(DP_DPCD, TRAIN_AUX_RD_INTERVAL, INTERVAL, dpcd[DP_DPCD_TRAIN_AUX_RD_INTERVAL_OFFSET])) {
			case DP_DPCD_TRAIN_AUX_RD_INTERVAL_INTERVAL_4MS: debug_print("4MS Supported\n"); break;
			case DP_DPCD_TRAIN_AUX_RD_INTERVAL_INTERVAL_8MS: debug_print("8MS\n"); break;
			case DP_DPCD_TRAIN_AUX_RD_INTERVAL_INTERVAL_12MS: debug_print("12MS\n"); break;
			case DP_DPCD_TRAIN_AUX_RD_INTERVAL_INTERVAL_16MS: debug_print("16MS\n"); break;
			default: debug_printf("%d\n", HW_REG_DECODE_FIELD(DP_DPCD, TRAIN_AUX_RD_INTERVAL, INTERVAL, dpcd[DP_DPCD_TRAIN_AUX_RD_INTERVAL_OFFSET])); break;
		}
	} else {
		debug_print("DP AuxRead @ DP_DPCD_REV failed\n");
	}
}
static void DumpDPCD_256_272(Connection* display) {
	uint8_t dpcd[16];
	debug_print("DumpDPCD_256_272: \n");

	if(AuxRead(display, DP_DPCD_LINK_BW_SET_OFFSET, 16, dpcd)) {
		switch (dpcd[0]) {
			case DP_DPCD_LINK_BW_SET_BANDWIDTH_1_62GBPS: debug_print("1.62Gbps * "); break;
			case DP_DPCD_LINK_BW_SET_BANDWIDTH_2_70GBPS: debug_print("2.70Gbps * "); break;
			case DP_DPCD_LINK_BW_SET_BANDWIDTH_5_40GBPS: debug_print("5.40Gbps * "); break;
		}
		switch (HW_REG_DECODE_FIELD(DP_DPCD, LANE_COUNT_SET, LANE_COUNT, dpcd[1])) {
			case 1: debug_print("1 lane\n"); break;
			case 2: debug_print("2 lanes\n"); break;
			case 3: debug_print("3 lanes\n"); break;
			case 4: debug_print("4 lanes\n"); break;
		}
		switch (HW_REG_DECODE_FIELD(DP_DPCD, TRAINING_PATTERN_SET, PATTERN_SELECT, dpcd[2])) {
			case DP_DPCD_TRAINING_PATTERN_SET_PATTERN_SELECT_DISABLED: debug_print("No Training Pattern Set\n"); break;
			case DP_DPCD_TRAINING_PATTERN_SET_PATTERN_SELECT_PATTERN_1: debug_print("Training Pattern 1 Set\n"); break;
			case DP_DPCD_TRAINING_PATTERN_SET_PATTERN_SELECT_PATTERN_2: debug_print("Training Pattern 2 Set\n"); break;
			case DP_DPCD_TRAINING_PATTERN_SET_PATTERN_SELECT_PATTERN_3: debug_print("Training Pattern 3 Set\n"); break;
		}
		switch (HW_REG_DECODE_FIELD(DP_DPCD, TRAINING_PATTERN_SET, LINK_QUAL_PATTERN, dpcd[2])) {
			case DP_DPCD_TRAINING_PATTERN_SET_LINK_QUAL_PATTERN_NOT_TRANSMITTED: debug_print("No Link Quality Pattern Set\n"); break;
			case DP_DPCD_TRAINING_PATTERN_SET_LINK_QUAL_PATTERN_D10_2_PATTERN: debug_print("Link Quality D10_2 Pattern Set\n"); break;
			case DP_DPCD_TRAINING_PATTERN_SET_LINK_QUAL_PATTERN_SYMBOL_ERROR_RATE_PATTERN: debug_print("Link Quality Symbol Error Rate Patter Set\n"); break;
			case DP_DPCD_TRAINING_PATTERN_SET_LINK_QUAL_PATTERN_PRBS7_PATTERN: debug_print("Link Quality PRBS7 Pattern Set\n"); break;
		}
		if(HW_REG_DECODE_FIELD(DP_DPCD, TRAINING_PATTERN_SET, SCRAMBLING_DISABLED, dpcd[2])) {
			debug_print("Link Pattern scrambling disabled\n");
		}
	}
}
#endif
void SetPixelClock(uint64_t FreqHz)
{
//Input Frequency for the PLL with precision upto two decimals
#define XAVBUF_INPUT_REF_CLK		33333333.33333
//#define XAVBUF_INPUT_REF_CLK 27000000.0
//Frequency of VCO before divider to meet jitter requirement
//#define XAVBUF_PLL_OUT_FREQ		1'485'325'000ULL
//#define XAVBUF_PLL_OUT_FREQ		1'450'000'000ULL
#define XAVBUF_PLL_OUT_FREQ	1'433'333'333ULL

	uint64_t const ExtDivider =  (XAVBUF_PLL_OUT_FREQ / FreqHz);
	uint32_t const ExtDivider0 = (ExtDivider > 63) ? 63 : ExtDivider;
	uint32_t const ExtDivider1 = (ExtDivider > 63) ? ExtDivider / 63 : 1;
	//debug_printf("FreqHz %llu ExtDivider %llu ExtDivider0 %lu ExtDivider1 %lu\n", FreqHz, ExtDivider, ExtDivider0, ExtDivider1);

	// Calculate integer and fractional parts
	uint64_t const Vco = FreqHz * (uint64_t)(ExtDivider1 * ExtDivider0 * 2);
	double const VcoF = (double)Vco / XAVBUF_INPUT_REF_CLK;
	auto const FracIntegerFBDIV = (uint32_t)VcoF;
	auto const Fractional = (uint32_t)((VcoF - (float)FracIntegerFBDIV) * 65536.0);
	//debug_printf("Vco %llu VcoF %f FracF %f FracIntegerFBDIV %u Fractional %u\n", Vco, VcoF, Fractional / 65536.0, FracIntegerFBDIV, Fractional );

	hw_ZynqmpPllHelper const cfg = hw_GetZynqmpPllHelper(FracIntegerFBDIV);
	PSI_IWord const SetVPLL[] = {
		PSI_SET_REGISTER_BANK(CRF_APB),
		PSI_WRITE_32(CRF_APB, VPLL_CTRL,
								 HW_REG_ENCODE_FIELD(CRF_APB, VPLL_CTRL, BYPASS, 1) |
								 HW_REG_ENCODE_FIELD(CRF_APB, VPLL_CTRL, RESET, 1)),
		PSI_WRITE_MASKED_32(CRF_APB, VPLL_CTRL,
												HW_REG_FIELD_MASK(CRF_APB, VPLL_CTRL, POST_SRC) |
												HW_REG_FIELD_MASK(CRF_APB, VPLL_CTRL, PRE_SRC) |
												HW_REG_FIELD_MASK(CRF_APB, VPLL_CTRL, FBDIV) |
												HW_REG_FIELD_MASK(CRF_APB, VPLL_CTRL, DIV2),
											 	HW_REG_ENCODE_ENUM(CRF_APB, VPLL_CTRL, POST_SRC, PS_REF_CLK) |
											 	HW_REG_ENCODE_ENUM(CRF_APB, VPLL_CTRL, PRE_SRC, PS_REF_CLK) |
											 	HW_REG_ENCODE_FIELD(CRF_APB, VPLL_CTRL, FBDIV, FracIntegerFBDIV) |
											 	HW_REG_ENCODE_FIELD(CRF_APB, VPLL_CTRL, DIV2, 1 ) |
											 	0),
		PSI_WRITE_32(CRF_APB, VPLL_CFG,
								 HW_REG_ENCODE_FIELD(CRF_APB, VPLL_CFG, LOCK_DLY, (uint32_t)cfg.lock_dly) |
								 HW_REG_ENCODE_FIELD(CRF_APB, VPLL_CFG, LOCK_CNT, (uint32_t)cfg.lock_cnt) |
								 HW_REG_ENCODE_FIELD(CRF_APB, VPLL_CFG, LFHF, (uint32_t)cfg.lfhf) |
								 HW_REG_ENCODE_FIELD(CRF_APB, VPLL_CFG, CP, (uint32_t)cfg.cp) |
								 HW_REG_ENCODE_FIELD(CRF_APB, VPLL_CFG, RES, (uint32_t)cfg.res) |
								 0),
		PSI_WRITE_32(CRF_APB, VPLL_FRAC_CFG,
								 HW_REG_ENCODE_FIELD(CRF_APB, VPLL_FRAC_CFG, ENABLED, (Fractional != 0)) |
								 HW_REG_ENCODE_FIELD(CRF_APB, VPLL_FRAC_CFG, DATA, Fractional) |
								 0),
		PSI_WRITE_MASKED_32(CRF_APB, VPLL_CTRL,
												HW_REG_FIELD_MASK(CRF_APB, VPLL_CTRL, RESET),
												HW_REG_ENCODE_FIELD(CRF_APB, VPLL_CTRL, RESET, 1)),
		PSI_DELAY_US(20),
		PSI_WRITE_MASKED_32(CRF_APB, VPLL_CTRL,
												HW_REG_FIELD_MASK(CRF_APB, VPLL_CTRL, RESET),
												HW_REG_ENCODE_FIELD(CRF_APB, VPLL_CTRL, RESET, 0)),

		PSI_POLL_MASKED_32(CRF_APB, PLL_STATUS, CRF_APB_PLL_STATUS_VPLL_LOCK),

		PSI_WRITE_MASKED_32(CRF_APB, VPLL_CTRL,
												HW_REG_FIELD_MASK(CRF_APB, VPLL_CTRL, BYPASS),
												HW_REG_ENCODE_FIELD(CRF_APB, VPLL_CTRL, BYPASS, 0)),

		PSI_POLL_MASKED_32(CRF_APB, PLL_STATUS, CRF_APB_PLL_STATUS_VPLL_STABLE),
		PSI_DELAY_US(20),

		PSI_WRITE_MASKED_32(CRF_APB, VPLL_TO_LPD_CTRL,
												HW_REG_FIELD_MASK(CRF_APB, VPLL_TO_LPD_CTRL, DIVISOR0),
												HW_REG_ENCODE_FIELD(CRF_APB, VPLL_TO_LPD_CTRL, DIVISOR0, 1)),

		PSI_WRITE_32(CRF_APB, DP_VIDEO_REF_CTRL,
								 HW_REG_ENCODE_FIELD(CRF_APB, DP_VIDEO_REF_CTRL, CLKACT, 0) | 0),
		PSI_DELAY_US(20),

		PSI_WRITE_MASKED_32(CRF_APB, DP_VIDEO_REF_CTRL,
								 HW_REG_FIELD_MASK(CRF_APB, DP_VIDEO_REF_CTRL,  CLKACT),
								 HW_REG_ENCODE_FIELD(CRF_APB, DP_VIDEO_REF_CTRL, CLKACT, 0)),

		PSI_WRITE_MASKED_32(CRF_APB, DP_VIDEO_REF_CTRL,
								HW_REG_FIELD_MASK(CRF_APB, DP_VIDEO_REF_CTRL, DIVISOR1) |
								HW_REG_FIELD_MASK(CRF_APB, DP_VIDEO_REF_CTRL, DIVISOR0) |
								HW_REG_FIELD_MASK(CRF_APB, DP_VIDEO_REF_CTRL, SRCSEL),
								HW_REG_ENCODE_FIELD(CRF_APB, DP_VIDEO_REF_CTRL, DIVISOR1, ExtDivider1) |
								HW_REG_ENCODE_FIELD(CRF_APB, DP_VIDEO_REF_CTRL, DIVISOR0, ExtDivider0) |
								HW_REG_ENCODE_FIELD(CRF_APB, DP_VIDEO_REF_CTRL, SRCSEL, CRF_APB_DP_VIDEO_REF_CTRL_SRCSEL_VPLL) |
								0),

		PSI_WRITE_MASKED_32(CRF_APB, DP_VIDEO_REF_CTRL,
												HW_REG_FIELD_MASK(CRF_APB, DP_VIDEO_REF_CTRL,  CLKACT),
												HW_REG_ENCODE_FIELD(CRF_APB, DP_VIDEO_REF_CTRL, CLKACT, 1)),

		PSI_END_PROGRAM,
	};

	psi_RunRegisterProgram(SetVPLL);
}

void UpdateMonitorLink(Connection *display) {
	uint8_t dpcd[16];
	if (AuxRead(display, DP_DPCD_REV_OFFSET, 16, dpcd)) {
		display->linkRate = (LinkRate) Math_Min_U8((uint8_t) display->linkRate, dpcd[DP_DPCD_MAX_LINK_RATE_OFFSET]);
		display->numLanes = Math_Min_U8(display->numLanes,
																		HW_REG_DECODE_FIELD(DP_DPCD, MAX_LANE_COUNT, MAX_LANE_COUNT,
																												dpcd[DP_DPCD_MAX_LANE_COUNT_OFFSET]));
		display->supportsEnhancedFrame =
				HW_REG_DECODE_FIELD(DP_DPCD, MAX_LANE_COUNT, ENHANCED_FRAME_SUPPORT, dpcd[DP_DPCD_MAX_LANE_COUNT_OFFSET]);
		display->supportsTrainingPattern3 =
				HW_REG_DECODE_FIELD(DP_DPCD, MAX_LANE_COUNT, TPS3_SUPPORT, dpcd[DP_DPCD_MAX_LANE_COUNT_OFFSET]);
		display->supportsDownSpread =
				HW_REG_DECODE_FIELD(DP_DPCD, MAX_DOWNSPREAD, MAX_DOWNSPREAD, dpcd[DP_DPCD_MAX_DOWNSPREAD_OFFSET]);

		display->delayRateUS =
				dpcd[DP_DPCD_TRAIN_AUX_RD_INTERVAL_OFFSET] ? 4000 * dpcd[DP_DPCD_TRAIN_AUX_RD_INTERVAL_OFFSET] : 400;
		Serdes::UpdateLaneStatusAdjReqs(display);

		if (!Serdes::SetVoltageSwingAndPreEmphasis(display)) {
			debug_print("SetVoltageSwingAndPreEmphasis failed\n");
			return;
		}

	} else {
		debug_print("DP AuxRead @ DP_DPCD_REV failed\n");
	}
}

void Init(Connection *link) {
	memset(link, 0, sizeof(Connection));
	link->numLanes = 2;
	link->linkRate = LinkRate::Rate_5_4Gbps;

	HW_REG_SET_BIT1(DP, SOFTWARE_RESET, SOFT_RST);
	HW_REG_CLR_BIT1(DP, TRANSMITTER_ENABLE, TX_EN);
	HW_REG_WRITE1(DP, PHY_RESET,
						 HW_REG_ENCODE_FIELD(DP, PHY_RESET, EN_8B_10B, 1) |
						 HW_REG_ENCODE_FIELD(DP, PHY_RESET, PHY_RESET, 1));

	// 100Mhz FPD APB clock
	HW_REG_WRITE1(DP, AUX_CLOCK_DIVIDER,
						 HW_REG_ENCODE_FIELD(DP, AUX_CLOCK_DIVIDER, AUX_SIGNAL_WIDTH_FILTER, 50) |
						 HW_REG_ENCODE_FIELD(DP, AUX_CLOCK_DIVIDER, CLK_DIV, 100)
	);
	HW_REG_WRITE1(DP, PHY_CLOCK_SELECT, DP_PHY_CLOCK_SELECT_SEL_LINK_5_4GBS);

	HW_REG_WRITE1(DP, PHY_RESET,
						 HW_REG_ENCODE_FIELD(DP, PHY_RESET, EN_8B_10B, 1) |
						 HW_REG_ENCODE_FIELD(DP, PHY_RESET, PHY_RESET, 0));

	// wait for phy
	Serdes::StallForPhyReady(link);

	HW_REG_SET_BIT1(DP, MAIN_STREAM_ENABLE, MS_ENABLE);
	HW_REG_SET_BIT1(DP, TRANSMITTER_ENABLE, TX_EN);

	// TODO add 0x600 to dpcd registers
	uint8_t wakeUpData = 0x1;
	// done twice to allow for sleepy heads to get out of bed
	AuxRead(link, 0x600, 1, &wakeUpData);
	Utils_BusyMilliSleep(20);
	AuxRead(link, 0x600, 1, &wakeUpData);
	Utils_BusyMilliSleep(20);


	HW_REG_SET_BIT1(DP, INT_EN, HPD_PULSE_DET);
	HW_REG_SET_BIT1(DP, INT_EN, HPD_EVENT);
	HW_REG_SET_BIT1(DP, INT_EN, HPD_IRQ);

}
void Init(Display *display) {
	memset(display, 0, sizeof(Display));
	display->pixelConfig.format = OutputPixelFormat::RGB;
	display->pixelConfig.bitsPerChannel = BitsPerPrimaryChannel::EIGHT_BITS;
	display->synchronousClock = 1;
}

void Init(DMADescriptor* dma) {
	memset(dma, 0, sizeof(DMADescriptor));
	dma->preamble = 0xA5;
	dma->ignoreDone = 1;
	dma->lastOfFrame = 1;
	dma->dscrId = 0;
	dma->nextDescriptorAddress = (uint32_t)(((uintptr_all_t)dma) & 0xFFFF'FFFF);
	dma->nextDescriptorAddressExt = (uint16_t)((((uintptr_all_t)dma) >> 32ULL) & 0xFFFF);

	// check bitfield match HW format
	assert( (*((uint32_t*)dma) & 0xFF) == 0xA5);
}

void Init(Mixer *mixer) {
	memset(mixer, 0, sizeof(Mixer));

	static const ColourTransformMatrix identityMatrix = {
			{0x0000, 0x0000, 0x0000},
			{0x1000, 0x0000, 0x0000,
			 0x0000, 0x1000, 0x0000,
			 0x0000, 0x0000, 0x1000},
			{0x0000, 0x0000, 0x0000}
	};

	mixer->videoPlane.source = DisplayVideoPlane::Source::DISABLED;
	mixer->gfxPlane.source = DisplayGfxPlane::Source::DISABLED;

	memcpy(&mixer->videoPlane.toRGBTransform, &identityMatrix, sizeof(ColourTransformMatrix));
	memcpy(&mixer->gfxPlane.toRGBTransform, &identityMatrix, sizeof(ColourTransformMatrix));
	memcpy(&mixer->outRgb2YCrCbMatrix, &identityMatrix, sizeof(ColourTransformMatrix));
}

bool IsDisplayConnected(Connection *link) {
	uint32_t timeout = 10;
	debug_print("Searching for display");
	do {
		debug_print(".");
		bool const hpdState = HW_REG_GET_BIT1(DP, INTERRUPT_SIGNAL_STATE, HPD_STATE);
		bool const hpdEvent = HW_REG_GET_BIT1(DP, INT_STATUS, HPD_EVENT);
		bool const hpdPulseDetected = HW_REG_GET_BIT1(DP, INT_STATUS, HPD_PULSE_DET);
		uint32_t hpdPulseDuration = 0;
		if (hpdPulseDetected) {
			hpdPulseDuration = HW_REG_READ1(DP, HPD_DURATION);
		}
		if (hpdState && hpdEvent) {
			debug_print(" found\n");
			return true;
		}
		if (hpdState && hpdPulseDuration >= 250) {
			debug_print(" found\n");
			return true;
		}

		Utils_BusyMilliSleep(25);
		timeout--;
	} while (timeout > 0);
	debug_print(" NO DISPLAY\n");

	return false;
}
void EnableMainLink(bool enable) {
	HW_REG_WRITE1(DP, FORCE_SCRAMBLER_RESET, 1);
	Utils_BusyMilliSleep(25);
	HW_REG_WRITE1(DP, FORCE_SCRAMBLER_RESET, 0);

	HW_REG_WRITE1(DP, MAIN_STREAM_ENABLE, enable);
}

bool Connect(Connection *link) {
	link->connected = false;

	if (!IsDisplayConnected(link)) {
		return false;
	}

	UpdateMonitorLink(link);

	if (link->supportsEnhancedFrame) {
		if (!Serdes::SetEnhancedFrameMode(link, true)) {
			debug_print("DP SetEnhancedFrameMode supported but failed\n");
		}
	}
	if (link->supportsDownSpread) {
		if (!Serdes::SetDownSpread(link, false)) {
			debug_print("DP SetDownSpread supported but failed\n");
		}
	}


	Serdes::SetLinkRate(link, link->linkRate);
	Serdes::SetLaneCount(link, link->numLanes);

	Serdes::ResetPhy(link);

	EnableMainLink(false);

	if (!TrainLink(link)) {
		debug_print("DP TrainLink failed\n");
		return false;
	}

	link->connected = true;

/*	debug_printf("DP Training passed: ");
	switch (link->linkRate) {
		case LinkRate::Rate_1_62Gbps: debug_print("1.62Gbps * ");
			break;
		case LinkRate::Rate_2_7Gbps: debug_print("2.70Gbps * ");
			break;
		case LinkRate::Rate_5_4Gbps: debug_print("5.40Gbps * ");
			break;
	}
	switch (link->numLanes) {
		case 1: debug_print("1 lane\n");
			break;
		case 2: debug_print("2 lanes\n");
			break;
		case 3: debug_print("3 lanes\n");
			break;
		case 4: debug_print("4 lanes\n");
			break;
	}
*/
	return true;
}

void SetDisplay(Connection *link, Display *display, Mixer *mixer) {
	uint8_t bitsPerPixel = 0;
	switch (display->pixelConfig.bitsPerChannel) {
		case BitsPerPrimaryChannel::SIX_BITS: bitsPerPixel = 6;
			break;
		case BitsPerPrimaryChannel::EIGHT_BITS: bitsPerPixel = 8;
			break;
		case BitsPerPrimaryChannel::TEN_BITS: bitsPerPixel = 10;
			break;
		case BitsPerPrimaryChannel::TWELVE_BITS: bitsPerPixel = 12;
			break;
		case BitsPerPrimaryChannel::SIXTEEN_BITS: bitsPerPixel = 16;
			break;
	}
	if (!display->pixelConfig.yOnly) {
		switch (display->pixelConfig.format) {
			case OutputPixelFormat::OTHER:
			case OutputPixelFormat::YCBCR_444:
			case OutputPixelFormat::RGB: bitsPerPixel = 3 * bitsPerPixel;
				break;
			case OutputPixelFormat::YCBCR_422: bitsPerPixel = 2 * bitsPerPixel;
				break;
		}
	}
	assert(bitsPerPixel > 0);

	uint32_t const wordsPerLine = ((display->videoTiming.width * bitsPerPixel) + 15) / 16;
	uint32_t const userDataCountPerLane = wordsPerLine +
														((wordsPerLine % link->numLanes) != 0 ? (wordsPerLine % link->numLanes) : 0) -
														link->numLanes;
	//debug_printf("wordsPerLine %d bitsPerPixel %d userDataCountPerLane %d\n", wordsPerLine, bitsPerPixel, userDataCountPerLane);

	uint32_t const transferUnitSize = 64;
	uint32_t const pixelClockKHz = (uint32_t)(((float)display->videoTiming.vTotal * (float)display->videoTiming.hTotal * display->videoTiming.frameRateHz) / 1000.0f);

	uint32_t const videoBandwidth = (pixelClockKHz * bitsPerPixel) / 8;
	uint32_t const linkBandwidth = link->numLanes * (int) link->linkRate * 27000;
	double const bw = (videoBandwidth / (double)linkBandwidth) * transferUnitSize;
	uint32_t const minBytesPerTransferUnit = (uint32_t)bw;
	uint32_t const fractionBytesPerTransferUnit = (uint32_t)((bw - (double)minBytesPerTransferUnit)*1024.0);

	uint32_t const mClockFactor = pixelClockKHz;
	uint32_t const nClockFactor = (int) link->linkRate * 27000;

	debug_printf("bw %f minBPTU %d fractionBPTU %d M %d N %d\n",
							 bw, minBytesPerTransferUnit, fractionBytesPerTransferUnit, mClockFactor, nClockFactor);

	uint16_t const hStart = display->videoTiming.hSyncPulseWidth + display->videoTiming.hBackPorch;
	uint16_t const vStart = display->videoTiming.vSyncPulseWidth + display->videoTiming.vBackPorch;
	uint16_t const initWait = (minBytesPerTransferUnit <= 4) ? transferUnitSize : transferUnitSize - minBytesPerTransferUnit;

//	debug_printf("TSU %ld pixelClockKHz %ld videoBandwith %ld linkBandwidth %ld initWait %d\n",
//							 transferUnitSize, pixelClockKHz, videoBandwidth, linkBandwidth, initWait);
//	debug_printf("H Addressable %d H total %d H Start %d H Sync Pulse %d H Polarity %d\n",
//							 display->videoTiming.width, display->videoTiming.hTotal, hStart, display->videoTiming.hSyncPulseWidth, display->videoTiming.hSyncPolarity);
//	debug_printf("V Addressable %d V total %d V Start %d V Sync Pulse %d V Polarity %d\n",
//							 display->videoTiming.height, display->videoTiming.vTotal, vStart, display->videoTiming.vSyncPulseWidth, display->videoTiming.vSyncPolarity);

	PSI_IWord const setMixerProgram[] = {
			PSI_SET_REGISTER_BANK(DP),

			// set the output format
			PSI_WRITE_32(DP, V_BLEND_OUTPUT_VID_FORMAT,
									 HW_REG_ENCODE_FIELD(DP, V_BLEND_OUTPUT_VID_FORMAT, EN_DOWNSAMPLE,
																			 (uint32_t) (display->pixelConfig.format == OutputPixelFormat::YCBCR_422)) |
									 HW_REG_ENCODE_FIELD(DP, V_BLEND_OUTPUT_VID_FORMAT, VID_FORMAT,
																			 (uint32_t) (display->pixelConfig.yOnly
																									 ? DP_V_BLEND_OUTPUT_VID_FORMAT_VID_FORMAT_Y_ONLY :
																									 (display->pixelConfig.format == OutputPixelFormat::RGB)
																									 ? DP_V_BLEND_OUTPUT_VID_FORMAT_VID_FORMAT_RGB :
																									 (display->pixelConfig.format == OutputPixelFormat::YCBCR_444)
																									 ? DP_V_BLEND_OUTPUT_VID_FORMAT_VID_FORMAT_YCBCR_444 :
																									 DP_V_BLEND_OUTPUT_VID_FORMAT_VID_FORMAT_YCBCR_422))),

			// set the background colour
			PSI_WRITE_32(DP, V_BLEND_BG_CLR_0, mixer->backgroundColour[0]),
			PSI_WRITE_32(DP, V_BLEND_BG_CLR_1, mixer->backgroundColour[1]),
			PSI_WRITE_32(DP, V_BLEND_BG_CLR_2, mixer->backgroundColour[2]),

			// layer setup
			PSI_WRITE_32(DP, V_BLEND_LAYER0_CONTROL,
									 HW_REG_ENCODE_FIELD(DP, V_BLEND_LAYER0_CONTROL, BYPASS,
																			 (uint32_t) (mixer->function == MixerFunction::VIDEO)) |
									 HW_REG_ENCODE_FIELD(DP, V_BLEND_LAYER0_CONTROL, RGB_MODE,
																			 (uint32_t) DisplayVideoPlane::IsFormatRGB(mixer->videoPlane.format)) |
									 HW_REG_ENCODE_FIELD(DP, V_BLEND_LAYER0_CONTROL, EN_US,
																			 (uint32_t) DisplayVideoPlane::NeedColourUpSampling(mixer->videoPlane.format))),
			PSI_WRITE_32(DP, V_BLEND_LAYER1_CONTROL,
									 HW_REG_ENCODE_FIELD(DP, V_BLEND_LAYER1_CONTROL, BYPASS,
																			 (uint32_t) (mixer->function == MixerFunction::GFX)) |
									 HW_REG_ENCODE_FIELD(DP, V_BLEND_LAYER1_CONTROL, RGB_MODE,
																			 (uint32_t) DisplayGfxPlane::IsFormatRGB(mixer->gfxPlane.format)) |
									 HW_REG_ENCODE_FIELD(DP, V_BLEND_LAYER1_CONTROL, EN_US,
																			 (uint32_t) DisplayGfxPlane::NeedColourUpSampling(mixer->gfxPlane.format))),


			// set chroma keying
			PSI_WRITE_32(DP, V_BLEND_CHROMA_KEY_ENABLE,
									 HW_REG_ENCODE_FIELD(DP, V_BLEND_CHROMA_KEY_ENABLE, M_SEL,
																			 (uint32_t) (mixer->function == MixerFunction::CHROMA_KEY_VIDEO)) |
									 HW_REG_ENCODE_FIELD(DP, V_BLEND_CHROMA_KEY_ENABLE, EN,
																			 (uint32_t) (mixer->function == MixerFunction::CHROMA_KEY_VIDEO
																					 || mixer->function == MixerFunction::CHROMA_KEY_GFX))),

			// set global alpha parameters
			PSI_WRITE_32(DP, V_BLEND_SET_GLOBAL_ALPHA_REG,
									 HW_REG_ENCODE_FIELD(DP, V_BLEND_SET_GLOBAL_ALPHA_REG, EN,
																			 (uint32_t) (mixer->function == MixerFunction::GLOBAL_PORTER_DUFF)) |
											 HW_REG_ENCODE_FIELD(DP, V_BLEND_SET_GLOBAL_ALPHA_REG, VALUE, (uint32_t) mixer->globalAlpha)),
			PSI_WRITE_32(DP, V_BLEND_CHROMA_KEY_COMP1,
									 HW_REG_ENCODE_FIELD(DP, V_BLEND_CHROMA_KEY_COMP1, MIN, (uint32_t) mixer->chromaKeyMin[0]) |
											 HW_REG_ENCODE_FIELD(DP, V_BLEND_CHROMA_KEY_COMP1, MAX, (uint32_t) mixer->chromaKeyMax[0])),
			PSI_WRITE_32(DP, V_BLEND_CHROMA_KEY_COMP2,
									 HW_REG_ENCODE_FIELD(DP, V_BLEND_CHROMA_KEY_COMP2, MIN, (uint32_t) mixer->chromaKeyMin[1]) |
											 HW_REG_ENCODE_FIELD(DP, V_BLEND_CHROMA_KEY_COMP2, MAX, (uint32_t) mixer->chromaKeyMax[1])),
			PSI_WRITE_32(DP, V_BLEND_CHROMA_KEY_COMP3,
									 HW_REG_ENCODE_FIELD(DP, V_BLEND_CHROMA_KEY_COMP3, MIN, (uint32_t) mixer->chromaKeyMin[2]) |
											 HW_REG_ENCODE_FIELD(DP, V_BLEND_CHROMA_KEY_COMP3, MAX, (uint32_t) mixer->chromaKeyMax[2])),


			// set the video planes colour transform
			PSI_WRITE_32(DP, V_BLEND_LUMA_IN1CSC_OFFSET,
									 HW_REG_ENCODE_FIELD(DP, V_BLEND_LUMA_IN1CSC_OFFSET, PRE_OFFSET,
																			 (uint32_t) mixer->videoPlane.toRGBTransform.preTranslate[0]) |
								 HW_REG_ENCODE_FIELD(DP, V_BLEND_LUMA_IN1CSC_OFFSET, POST_OFFSET,
																		 (uint32_t) mixer->videoPlane.toRGBTransform.postTranslate[0])),
			PSI_WRITE_32(DP, V_BLEND_CR_IN1CSC_OFFSET,
									 HW_REG_ENCODE_FIELD(DP, V_BLEND_CR_IN1CSC_OFFSET, PRE_OFFSET,
																			 (uint32_t) mixer->videoPlane.toRGBTransform.preTranslate[1]) |
									 HW_REG_ENCODE_FIELD(DP, V_BLEND_CR_IN1CSC_OFFSET, POST_OFFSET,
																			 (uint32_t) mixer->videoPlane.toRGBTransform.postTranslate[1])),
			PSI_WRITE_32(DP, V_BLEND_CB_IN1CSC_OFFSET,
									 HW_REG_ENCODE_FIELD(DP, V_BLEND_CB_IN1CSC_OFFSET, PRE_OFFSET,
																			 (uint32_t) mixer->videoPlane.toRGBTransform.preTranslate[2]) |
									 HW_REG_ENCODE_FIELD(DP, V_BLEND_CB_IN1CSC_OFFSET, POST_OFFSET,
																			 (uint32_t) mixer->videoPlane.toRGBTransform.postTranslate[2])),
			PSI_WRITE_32(DP, V_BLEND_IN1CSC_COEFF0, mixer->videoPlane.toRGBTransform.matrix[0]),
			PSI_WRITE_32(DP, V_BLEND_IN1CSC_COEFF1, mixer->videoPlane.toRGBTransform.matrix[1]),
			PSI_WRITE_32(DP, V_BLEND_IN1CSC_COEFF2, mixer->videoPlane.toRGBTransform.matrix[2]),
			PSI_WRITE_32(DP, V_BLEND_IN1CSC_COEFF3, mixer->videoPlane.toRGBTransform.matrix[3]),
			PSI_WRITE_32(DP, V_BLEND_IN1CSC_COEFF4, mixer->videoPlane.toRGBTransform.matrix[4]),
			PSI_WRITE_32(DP, V_BLEND_IN1CSC_COEFF5, mixer->videoPlane.toRGBTransform.matrix[5]),
			PSI_WRITE_32(DP, V_BLEND_IN1CSC_COEFF6, mixer->videoPlane.toRGBTransform.matrix[6]),
			PSI_WRITE_32(DP, V_BLEND_IN1CSC_COEFF7, mixer->videoPlane.toRGBTransform.matrix[7]),
			PSI_WRITE_32(DP, V_BLEND_IN1CSC_COEFF8, mixer->videoPlane.toRGBTransform.matrix[8]),

			// gfx colour space
			PSI_WRITE_32(DP, V_BLEND_LUMA_IN2CSC_OFFSET,
									 HW_REG_ENCODE_FIELD(DP, V_BLEND_LUMA_IN2CSC_OFFSET, PRE_OFFSET,
																			 (uint32_t) mixer->gfxPlane.toRGBTransform.preTranslate[0]) |
									 HW_REG_ENCODE_FIELD(DP, V_BLEND_LUMA_IN2CSC_OFFSET, POST_OFFSET,
																			 (uint32_t) mixer->gfxPlane.toRGBTransform.postTranslate[0])),
			PSI_WRITE_32(DP, V_BLEND_CR_IN2CSC_OFFSET,
									 HW_REG_ENCODE_FIELD(DP, V_BLEND_CR_IN2CSC_OFFSET, PRE_OFFSET,
																			 (uint32_t) mixer->gfxPlane.toRGBTransform.preTranslate[1]) |
									 HW_REG_ENCODE_FIELD(DP, V_BLEND_CR_IN2CSC_OFFSET, POST_OFFSET,
																			 (uint32_t) mixer->gfxPlane.toRGBTransform.postTranslate[1])),
			PSI_WRITE_32(DP, V_BLEND_CB_IN2CSC_OFFSET,
									 HW_REG_ENCODE_FIELD(DP, V_BLEND_CB_IN2CSC_OFFSET, PRE_OFFSET,
																			 (uint32_t) mixer->gfxPlane.toRGBTransform.preTranslate[2]) |
									 HW_REG_ENCODE_FIELD(DP, V_BLEND_CB_IN2CSC_OFFSET, POST_OFFSET,
																			 (uint32_t) mixer->gfxPlane.toRGBTransform.postTranslate[2])),
			PSI_WRITE_32(DP, V_BLEND_IN2CSC_COEFF0, mixer->gfxPlane.toRGBTransform.matrix[0]),
			PSI_WRITE_32(DP, V_BLEND_IN2CSC_COEFF1, mixer->gfxPlane.toRGBTransform.matrix[1]),
			PSI_WRITE_32(DP, V_BLEND_IN2CSC_COEFF2, mixer->gfxPlane.toRGBTransform.matrix[2]),
			PSI_WRITE_32(DP, V_BLEND_IN2CSC_COEFF3, mixer->gfxPlane.toRGBTransform.matrix[3]),
			PSI_WRITE_32(DP, V_BLEND_IN2CSC_COEFF4, mixer->gfxPlane.toRGBTransform.matrix[4]),
			PSI_WRITE_32(DP, V_BLEND_IN2CSC_COEFF5, mixer->gfxPlane.toRGBTransform.matrix[5]),
			PSI_WRITE_32(DP, V_BLEND_IN2CSC_COEFF6, mixer->gfxPlane.toRGBTransform.matrix[6]),
			PSI_WRITE_32(DP, V_BLEND_IN2CSC_COEFF7, mixer->gfxPlane.toRGBTransform.matrix[7]),
			PSI_WRITE_32(DP, V_BLEND_IN2CSC_COEFF8, mixer->gfxPlane.toRGBTransform.matrix[8]),

			// output colour transform
			PSI_WRITE_32(DP, V_BLEND_LUMA_OUTCSC_OFFSET,
									 HW_REG_ENCODE_FIELD(DP, V_BLEND_LUMA_OUTCSC_OFFSET, PRE_OFFSET,
																			 (uint32_t) mixer->outRgb2YCrCbMatrix.preTranslate[0]) |
									 HW_REG_ENCODE_FIELD(DP, V_BLEND_LUMA_OUTCSC_OFFSET, POST_OFFSET,
																			 (uint32_t) mixer->outRgb2YCrCbMatrix.postTranslate[0])),
			PSI_WRITE_32(DP, V_BLEND_CR_OUTCSC_OFFSET,
									 HW_REG_ENCODE_FIELD(DP, V_BLEND_CR_OUTCSC_OFFSET, PRE_OFFSET,
																			 (uint32_t) mixer->outRgb2YCrCbMatrix.preTranslate[1]) |
									 HW_REG_ENCODE_FIELD(DP, V_BLEND_CR_OUTCSC_OFFSET, POST_OFFSET,
																			 (uint32_t) mixer->outRgb2YCrCbMatrix.postTranslate[1])),
			PSI_WRITE_32(DP, V_BLEND_CB_OUTCSC_OFFSET,
									 HW_REG_ENCODE_FIELD(DP, V_BLEND_CB_OUTCSC_OFFSET, PRE_OFFSET,
																			 (uint32_t) mixer->outRgb2YCrCbMatrix.preTranslate[2]) |
									 HW_REG_ENCODE_FIELD(DP, V_BLEND_CB_OUTCSC_OFFSET, POST_OFFSET,
																			 (uint32_t) mixer->outRgb2YCrCbMatrix.postTranslate[2])),
			PSI_WRITE_32(DP, V_BLEND_RGB2YCBCR_COEFF0, mixer->outRgb2YCrCbMatrix.matrix[0]),
			PSI_WRITE_32(DP, V_BLEND_RGB2YCBCR_COEFF1, mixer->outRgb2YCrCbMatrix.matrix[1]),
			PSI_WRITE_32(DP, V_BLEND_RGB2YCBCR_COEFF2, mixer->outRgb2YCrCbMatrix.matrix[2]),
			PSI_WRITE_32(DP, V_BLEND_RGB2YCBCR_COEFF3, mixer->outRgb2YCrCbMatrix.matrix[3]),
			PSI_WRITE_32(DP, V_BLEND_RGB2YCBCR_COEFF4, mixer->outRgb2YCrCbMatrix.matrix[4]),
			PSI_WRITE_32(DP, V_BLEND_RGB2YCBCR_COEFF5, mixer->outRgb2YCrCbMatrix.matrix[5]),
			PSI_WRITE_32(DP, V_BLEND_RGB2YCBCR_COEFF6, mixer->outRgb2YCrCbMatrix.matrix[6]),
			PSI_WRITE_32(DP, V_BLEND_RGB2YCBCR_COEFF7, mixer->outRgb2YCrCbMatrix.matrix[7]),
			PSI_WRITE_32(DP, V_BLEND_RGB2YCBCR_COEFF8, mixer->outRgb2YCrCbMatrix.matrix[8]),

			PSI_END_PROGRAM

	};

	PSI_IWord const setMainStreamOutputProgram[] = {
			PSI_SET_REGISTER_BANK(DP),

			PSI_WRITE_32(DP, MAIN_STREAM_HTOTAL, display->videoTiming.hTotal),
			PSI_WRITE_32(DP, MAIN_STREAM_VTOTAL, display->videoTiming.vTotal),
			PSI_WRITE_32(DP, MAIN_STREAM_POLARITY,
									 HW_REG_ENCODE_FIELD(DP,
																			 MAIN_STREAM_POLARITY,
																			 HSYNC_POLARITY,
																			 (uint32_t) display->videoTiming.hSyncPolarity) |
									 HW_REG_ENCODE_FIELD(DP,
																			 MAIN_STREAM_POLARITY,
																			 VSYNC_POLARITY,
																			 (uint32_t) display->videoTiming.vSyncPolarity)),
			PSI_WRITE_32(DP, MAIN_STREAM_HSWIDTH, display->videoTiming.hSyncPulseWidth),
			PSI_WRITE_32(DP, MAIN_STREAM_VSWIDTH, display->videoTiming.vSyncPulseWidth),
			PSI_WRITE_32(DP, MAIN_STREAM_HRES, display->videoTiming.width),
			PSI_WRITE_32(DP, MAIN_STREAM_VRES, display->videoTiming.height),
			PSI_WRITE_32(DP, MAIN_STREAM_HSTART, hStart),
			PSI_WRITE_32(DP, MAIN_STREAM_VSTART, vStart),
			PSI_WRITE_32(DP, MAIN_STREAM_MISC0,
									 HW_REG_ENCODE_FIELD(DP, MAIN_STREAM_MISC0, BPC, (uint32_t) display->pixelConfig.bitsPerChannel) |
									 HW_REG_ENCODE_FIELD(DP, MAIN_STREAM_MISC0, YCBCR_COLR, (uint32_t) display->ycbcrColourimety) |
									 HW_REG_ENCODE_FIELD(DP, MAIN_STREAM_MISC0, DYNC_RANGE, (uint32_t) display->dynamicRange) |
									 HW_REG_ENCODE_FIELD(DP, MAIN_STREAM_MISC0, COMP_FORMAT, (uint32_t) display->pixelConfig.format) |
									 HW_REG_ENCODE_FIELD(DP, MAIN_STREAM_MISC0, SYNC_CLOCK, (uint32_t) display->synchronousClock)),
			PSI_WRITE_32(DP, MAIN_STREAM_MISC1,
									 HW_REG_ENCODE_FIELD(DP, MAIN_STREAM_MISC1, Y_ONLY_EN, (uint32_t) display->pixelConfig.yOnly)),
			PSI_WRITE_32(DP, MAIN_STREAM_M_VID, mClockFactor),
			PSI_WRITE_32(DP, MSA_TRANSFER_UNIT_SIZE, transferUnitSize),
			PSI_WRITE_32(DP, MAIN_STREAM_N_VID, nClockFactor),
			PSI_WRITE_32(DP, USER_DATA_COUNT_PER_LANE, userDataCountPerLane),
			PSI_WRITE_32(DP, USER_PIX_WIDTH, link->numLanes),
			PSI_WRITE_32(DP, MIN_BYTES_PER_TU, minBytesPerTransferUnit),
			PSI_WRITE_32(DP, FRAC_BYTES_PER_TU, fractionBytesPerTransferUnit),
			PSI_WRITE_32(DP, INIT_WAIT, initWait),
			PSI_END_PROGRAM
	};

	EnableMainLink( false );

	psi_RunRegisterProgram( setMixerProgram );

	Connect( link );


	SetPixelClock( pixelClockKHz * 1000 );
	Utils_BusyMilliSleep(10);

	HW_REG_WRITE1( DP, SOFTWARE_RESET, HW_REG_ENCODE_FIELD( DP, SOFTWARE_RESET, SOFT_RST, 1 ));
	Utils_BusyMilliSleep( 10 );
	HW_REG_WRITE1( DP, SOFTWARE_RESET, HW_REG_ENCODE_FIELD( DP, SOFTWARE_RESET, SOFT_RST, 0 ));

	psi_RunRegisterProgram( setMainStreamOutputProgram );

	Utils_BusyMilliSleep(10);

	SetMixerDMA( mixer );

	Utils_BusyMilliSleep(10);

	EnableMainLink( true );



}

void SetMixerDMA(Mixer* mixer) {
	bool const thirtyTwoBitAddressBus = sizeof(uintptr_all_t) == sizeof(uintptr_t);

	assert(!thirtyTwoBitAddressBus || mixer->videoPlane.simpleDescPlane0Address <= 0xFFFFFFFF);
	assert(!thirtyTwoBitAddressBus || mixer->videoPlane.simpleDescPlane1Address <= 0xFFFFFFFF);
	assert(!thirtyTwoBitAddressBus || mixer->videoPlane.simpleDescPlane2Address <= 0xFFFFFFFF);
	assert(!thirtyTwoBitAddressBus || mixer->gfxPlane.simpleDescBufferAddress <= 0xFFFFFFFF);

	HW_REG_WRITE1(DP, INT_DS, 0xFFFFFFFF);
	HW_REG_WRITE1(DP, INT_STATUS, 0xFFFFFFFF);
	HW_REG_WRITE1(DPDMA, IMR, 0xFFFFFFFF);
	HW_REG_WRITE1(DPDMA, EIMR, 0xFFFFFFFF);
	HW_REG_WRITE1(DPDMA, ISR, 0xFFFFFFFF);
	HW_REG_WRITE1(DPDMA, EISR, 0xFFFFFFFF);
	PSI_IWord const setMixerProgram[] = {
		PSI_SET_REGISTER_BANK(DP),

		// set the background colour
		PSI_WRITE_32(DP, V_BLEND_BG_CLR_0, mixer->backgroundColour[0]),
		PSI_WRITE_32(DP, V_BLEND_BG_CLR_1, mixer->backgroundColour[1]),
		PSI_WRITE_32(DP, V_BLEND_BG_CLR_2, mixer->backgroundColour[2]),

		// layer setup
		PSI_WRITE_32(DP, V_BLEND_LAYER0_CONTROL,
		             HW_REG_ENCODE_FIELD(DP, V_BLEND_LAYER0_CONTROL, BYPASS,
		                                 (uint32_t) (mixer->function == MixerFunction::VIDEO)) |
		             HW_REG_ENCODE_FIELD(DP, V_BLEND_LAYER0_CONTROL, RGB_MODE,
		                                 (uint32_t) DisplayVideoPlane::IsFormatRGB(mixer->videoPlane.format)) |
		             HW_REG_ENCODE_FIELD(DP, V_BLEND_LAYER0_CONTROL, EN_US,
		                                 (uint32_t) DisplayVideoPlane::NeedColourUpSampling(mixer->videoPlane.format))),
		PSI_WRITE_32(DP, V_BLEND_LAYER1_CONTROL,
		             HW_REG_ENCODE_FIELD(DP, V_BLEND_LAYER1_CONTROL, BYPASS,
		                                 (uint32_t) (mixer->function == MixerFunction::GFX)) |
		             HW_REG_ENCODE_FIELD(DP, V_BLEND_LAYER1_CONTROL, RGB_MODE,
		                                 (uint32_t) DisplayGfxPlane::IsFormatRGB(mixer->gfxPlane.format)) |
		             HW_REG_ENCODE_FIELD(DP, V_BLEND_LAYER1_CONTROL, EN_US,
		                                 (uint32_t) DisplayGfxPlane::NeedColourUpSampling(mixer->gfxPlane.format))),


		// set chroma keying
		PSI_WRITE_32(DP, V_BLEND_CHROMA_KEY_ENABLE,
		             HW_REG_ENCODE_FIELD(DP, V_BLEND_CHROMA_KEY_ENABLE, M_SEL,
		                                 (uint32_t) (mixer->function == MixerFunction::CHROMA_KEY_VIDEO)) |
		             HW_REG_ENCODE_FIELD(DP, V_BLEND_CHROMA_KEY_ENABLE, EN,
		                                 (uint32_t) (mixer->function == MixerFunction::CHROMA_KEY_VIDEO
		                                             || mixer->function == MixerFunction::CHROMA_KEY_GFX))),

		// set global alpha parameters
		PSI_WRITE_32(DP, V_BLEND_SET_GLOBAL_ALPHA_REG,
		             HW_REG_ENCODE_FIELD(DP, V_BLEND_SET_GLOBAL_ALPHA_REG, EN,
		                                 (uint32_t) (mixer->function == MixerFunction::GLOBAL_PORTER_DUFF)) |
		             HW_REG_ENCODE_FIELD(DP, V_BLEND_SET_GLOBAL_ALPHA_REG, VALUE, (uint32_t) mixer->globalAlpha)),
		PSI_WRITE_32(DP, V_BLEND_CHROMA_KEY_COMP1,
		             HW_REG_ENCODE_FIELD(DP, V_BLEND_CHROMA_KEY_COMP1, MIN, (uint32_t) mixer->chromaKeyMin[0]) |
		             HW_REG_ENCODE_FIELD(DP, V_BLEND_CHROMA_KEY_COMP1, MAX, (uint32_t) mixer->chromaKeyMax[0])),
		PSI_WRITE_32(DP, V_BLEND_CHROMA_KEY_COMP2,
		             HW_REG_ENCODE_FIELD(DP, V_BLEND_CHROMA_KEY_COMP2, MIN, (uint32_t) mixer->chromaKeyMin[1]) |
		             HW_REG_ENCODE_FIELD(DP, V_BLEND_CHROMA_KEY_COMP2, MAX, (uint32_t) mixer->chromaKeyMax[1])),
		PSI_WRITE_32(DP, V_BLEND_CHROMA_KEY_COMP3,
		             HW_REG_ENCODE_FIELD(DP, V_BLEND_CHROMA_KEY_COMP3, MIN, (uint32_t) mixer->chromaKeyMin[2]) |
		             HW_REG_ENCODE_FIELD(DP, V_BLEND_CHROMA_KEY_COMP3, MAX, (uint32_t) mixer->chromaKeyMax[2])),


		// set the video planes colour transform
		PSI_WRITE_32(DP, V_BLEND_LUMA_IN1CSC_OFFSET,
		             HW_REG_ENCODE_FIELD(DP, V_BLEND_LUMA_IN1CSC_OFFSET, PRE_OFFSET,
		                                 (uint32_t) mixer->videoPlane.toRGBTransform.preTranslate[0]) |
		             HW_REG_ENCODE_FIELD(DP, V_BLEND_LUMA_IN1CSC_OFFSET, POST_OFFSET,
		                                 (uint32_t) mixer->videoPlane.toRGBTransform.postTranslate[0])),
		PSI_WRITE_32(DP, V_BLEND_CR_IN1CSC_OFFSET,
		             HW_REG_ENCODE_FIELD(DP, V_BLEND_CR_IN1CSC_OFFSET, PRE_OFFSET,
		                                 (uint32_t) mixer->videoPlane.toRGBTransform.preTranslate[1]) |
		             HW_REG_ENCODE_FIELD(DP, V_BLEND_CR_IN1CSC_OFFSET, POST_OFFSET,
		                                 (uint32_t) mixer->videoPlane.toRGBTransform.postTranslate[1])),
		PSI_WRITE_32(DP, V_BLEND_CB_IN1CSC_OFFSET,
		             HW_REG_ENCODE_FIELD(DP, V_BLEND_CB_IN1CSC_OFFSET, PRE_OFFSET,
		                                 (uint32_t) mixer->videoPlane.toRGBTransform.preTranslate[2]) |
		             HW_REG_ENCODE_FIELD(DP, V_BLEND_CB_IN1CSC_OFFSET, POST_OFFSET,
		                                 (uint32_t) mixer->videoPlane.toRGBTransform.postTranslate[2])),
		PSI_WRITE_32(DP, V_BLEND_IN1CSC_COEFF0, mixer->videoPlane.toRGBTransform.matrix[0]),
		PSI_WRITE_32(DP, V_BLEND_IN1CSC_COEFF1, mixer->videoPlane.toRGBTransform.matrix[1]),
		PSI_WRITE_32(DP, V_BLEND_IN1CSC_COEFF2, mixer->videoPlane.toRGBTransform.matrix[2]),
		PSI_WRITE_32(DP, V_BLEND_IN1CSC_COEFF3, mixer->videoPlane.toRGBTransform.matrix[3]),
		PSI_WRITE_32(DP, V_BLEND_IN1CSC_COEFF4, mixer->videoPlane.toRGBTransform.matrix[4]),
		PSI_WRITE_32(DP, V_BLEND_IN1CSC_COEFF5, mixer->videoPlane.toRGBTransform.matrix[5]),
		PSI_WRITE_32(DP, V_BLEND_IN1CSC_COEFF6, mixer->videoPlane.toRGBTransform.matrix[6]),
		PSI_WRITE_32(DP, V_BLEND_IN1CSC_COEFF7, mixer->videoPlane.toRGBTransform.matrix[7]),
		PSI_WRITE_32(DP, V_BLEND_IN1CSC_COEFF8, mixer->videoPlane.toRGBTransform.matrix[8]),

		// gfx colour space
		PSI_WRITE_32(DP, V_BLEND_LUMA_IN2CSC_OFFSET,
		             HW_REG_ENCODE_FIELD(DP, V_BLEND_LUMA_IN2CSC_OFFSET, PRE_OFFSET,
		                                 (uint32_t) mixer->gfxPlane.toRGBTransform.preTranslate[0]) |
		             HW_REG_ENCODE_FIELD(DP, V_BLEND_LUMA_IN2CSC_OFFSET, POST_OFFSET,
		                                 (uint32_t) mixer->gfxPlane.toRGBTransform.postTranslate[0])),
		PSI_WRITE_32(DP, V_BLEND_CR_IN2CSC_OFFSET,
		             HW_REG_ENCODE_FIELD(DP, V_BLEND_CR_IN2CSC_OFFSET, PRE_OFFSET,
		                                 (uint32_t) mixer->gfxPlane.toRGBTransform.preTranslate[1]) |
		             HW_REG_ENCODE_FIELD(DP, V_BLEND_CR_IN2CSC_OFFSET, POST_OFFSET,
		                                 (uint32_t) mixer->gfxPlane.toRGBTransform.postTranslate[1])),
		PSI_WRITE_32(DP, V_BLEND_CB_IN2CSC_OFFSET,
		             HW_REG_ENCODE_FIELD(DP, V_BLEND_CB_IN2CSC_OFFSET, PRE_OFFSET,
		                                 (uint32_t) mixer->gfxPlane.toRGBTransform.preTranslate[2]) |
		             HW_REG_ENCODE_FIELD(DP, V_BLEND_CB_IN2CSC_OFFSET, POST_OFFSET,
		                                 (uint32_t) mixer->gfxPlane.toRGBTransform.postTranslate[2])),
		PSI_WRITE_32(DP, V_BLEND_IN2CSC_COEFF0, mixer->gfxPlane.toRGBTransform.matrix[0]),
		PSI_WRITE_32(DP, V_BLEND_IN2CSC_COEFF1, mixer->gfxPlane.toRGBTransform.matrix[1]),
		PSI_WRITE_32(DP, V_BLEND_IN2CSC_COEFF2, mixer->gfxPlane.toRGBTransform.matrix[2]),
		PSI_WRITE_32(DP, V_BLEND_IN2CSC_COEFF3, mixer->gfxPlane.toRGBTransform.matrix[3]),
		PSI_WRITE_32(DP, V_BLEND_IN2CSC_COEFF4, mixer->gfxPlane.toRGBTransform.matrix[4]),
		PSI_WRITE_32(DP, V_BLEND_IN2CSC_COEFF5, mixer->gfxPlane.toRGBTransform.matrix[5]),
		PSI_WRITE_32(DP, V_BLEND_IN2CSC_COEFF6, mixer->gfxPlane.toRGBTransform.matrix[6]),
		PSI_WRITE_32(DP, V_BLEND_IN2CSC_COEFF7, mixer->gfxPlane.toRGBTransform.matrix[7]),
		PSI_WRITE_32(DP, V_BLEND_IN2CSC_COEFF8, mixer->gfxPlane.toRGBTransform.matrix[8]),

		// output colour transform
		PSI_WRITE_32(DP, V_BLEND_LUMA_OUTCSC_OFFSET,
		             HW_REG_ENCODE_FIELD(DP, V_BLEND_LUMA_OUTCSC_OFFSET, PRE_OFFSET,
		                                 (uint32_t) mixer->outRgb2YCrCbMatrix.preTranslate[0]) |
		             HW_REG_ENCODE_FIELD(DP, V_BLEND_LUMA_OUTCSC_OFFSET, POST_OFFSET,
		                                 (uint32_t) mixer->outRgb2YCrCbMatrix.postTranslate[0])),
		PSI_WRITE_32(DP, V_BLEND_CR_OUTCSC_OFFSET,
		             HW_REG_ENCODE_FIELD(DP, V_BLEND_CR_OUTCSC_OFFSET, PRE_OFFSET,
		                                 (uint32_t) mixer->outRgb2YCrCbMatrix.preTranslate[1]) |
		             HW_REG_ENCODE_FIELD(DP, V_BLEND_CR_OUTCSC_OFFSET, POST_OFFSET,
		                                 (uint32_t) mixer->outRgb2YCrCbMatrix.postTranslate[1])),
		PSI_WRITE_32(DP, V_BLEND_CB_OUTCSC_OFFSET,
		             HW_REG_ENCODE_FIELD(DP, V_BLEND_CB_OUTCSC_OFFSET, PRE_OFFSET,
		                                 (uint32_t) mixer->outRgb2YCrCbMatrix.preTranslate[2]) |
		             HW_REG_ENCODE_FIELD(DP, V_BLEND_CB_OUTCSC_OFFSET, POST_OFFSET,
		                                 (uint32_t) mixer->outRgb2YCrCbMatrix.postTranslate[2])),
		PSI_WRITE_32(DP, V_BLEND_RGB2YCBCR_COEFF0, mixer->outRgb2YCrCbMatrix.matrix[0]),
		PSI_WRITE_32(DP, V_BLEND_RGB2YCBCR_COEFF1, mixer->outRgb2YCrCbMatrix.matrix[1]),
		PSI_WRITE_32(DP, V_BLEND_RGB2YCBCR_COEFF2, mixer->outRgb2YCrCbMatrix.matrix[2]),
		PSI_WRITE_32(DP, V_BLEND_RGB2YCBCR_COEFF3, mixer->outRgb2YCrCbMatrix.matrix[3]),
		PSI_WRITE_32(DP, V_BLEND_RGB2YCBCR_COEFF4, mixer->outRgb2YCrCbMatrix.matrix[4]),
		PSI_WRITE_32(DP, V_BLEND_RGB2YCBCR_COEFF5, mixer->outRgb2YCrCbMatrix.matrix[5]),
		PSI_WRITE_32(DP, V_BLEND_RGB2YCBCR_COEFF6, mixer->outRgb2YCrCbMatrix.matrix[6]),
		PSI_WRITE_32(DP, V_BLEND_RGB2YCBCR_COEFF7, mixer->outRgb2YCrCbMatrix.matrix[7]),
		PSI_WRITE_32(DP, V_BLEND_RGB2YCBCR_COEFF8, mixer->outRgb2YCrCbMatrix.matrix[8]),

		PSI_END_PROGRAM

	};
	psi_RunRegisterProgram(setMixerProgram);

	HW_REG_WRITE1(DP, AV_BUF_FORMAT,
							 HW_REG_ENCODE_FIELD(DP, AV_BUF_FORMAT, NL_GRAPHX_FORMAT, (uint32_t)mixer->gfxPlane.format) |
							 HW_REG_ENCODE_FIELD(DP, AV_BUF_FORMAT, NL_VID_FORMAT, (uint32_t)mixer->videoPlane.format) );

	// set the clock sources
	HW_REG_WRITE1(DP, AV_BUF_AUD_VID_CLK_SOURCE,
							 HW_REG_ENCODE_FIELD(DP, AV_BUF_AUD_VID_CLK_SOURCE, VID_TIMING_SRC, 1) |
							 HW_REG_ENCODE_FIELD(DP, AV_BUF_AUD_VID_CLK_SOURCE, AUD_CLK_SRC, 1) |
							 HW_REG_ENCODE_FIELD(DP, AV_BUF_AUD_VID_CLK_SOURCE, VID_CLK_SRC, 1));

	bool const planeEnable[4] = {
			(mixer->videoPlane.source == DisplayVideoPlane::Source::BUFFER),
			(mixer->videoPlane.source == DisplayVideoPlane::Source::BUFFER
					&& (DisplayVideoPlane::NumberOfPlanes(mixer->videoPlane.format) >= 2)),
			(mixer->videoPlane.source == DisplayVideoPlane::Source::BUFFER
					&& (DisplayVideoPlane::NumberOfPlanes(mixer->videoPlane.format) >= 3)),
			(mixer->gfxPlane.source == DisplayGfxPlane::Source::BUFFER)
	};
	HW_REG_WRITE1(DP, AV_CHBUF0, HW_REG_ENCODE_FIELD(DP, AV_CHBUF0, FLUSH, 1));
	HW_REG_WRITE1(DP, AV_CHBUF1, HW_REG_ENCODE_FIELD(DP, AV_CHBUF1, FLUSH, 1));
	HW_REG_WRITE1(DP, AV_CHBUF2, HW_REG_ENCODE_FIELD(DP, AV_CHBUF2, FLUSH, 1));
	HW_REG_WRITE1(DP, AV_CHBUF3, HW_REG_ENCODE_FIELD(DP, AV_CHBUF3, FLUSH, 1));

	// TODO work out best burst len, 1 seem to work for all modes currently
	if(planeEnable[0]) {
		HW_REG_WRITE1(DP, AV_CHBUF0,
							 HW_REG_ENCODE_FIELD(DP, AV_CHBUF0, BURST_LEN, 1) |
							 HW_REG_ENCODE_FIELD(DP, AV_CHBUF0, EN, 1));
	}
	if(planeEnable[1]) {
		HW_REG_WRITE1(DP, AV_CHBUF1,
							 HW_REG_ENCODE_FIELD(DP, AV_CHBUF1, BURST_LEN, 1) |
							 HW_REG_ENCODE_FIELD(DP, AV_CHBUF1, EN, 1));
	}
	if(planeEnable[2]) {
		HW_REG_WRITE1(DP, AV_CHBUF2,
							 HW_REG_ENCODE_FIELD(DP, AV_CHBUF2, BURST_LEN, 1) |
							 HW_REG_ENCODE_FIELD(DP, AV_CHBUF2, EN, 1));
	}
	if(planeEnable[3]) {
		HW_REG_WRITE1(DP, AV_CHBUF3,
							 HW_REG_ENCODE_FIELD(DP, AV_CHBUF3, BURST_LEN, 1) |
							 HW_REG_ENCODE_FIELD(DP, AV_CHBUF3, EN, 1));
	}

	HW_REG_WRITE1(DP, AV_BUF_VIDEO_COMP0_SCALE_FACTOR,
						 HW_REG_ENCODE_FIELD(DP, AV_BUF_VIDEO_COMP0_SCALE_FACTOR, VID_SCA_FACT0, 0x10101) );
	HW_REG_WRITE1(DP, AV_BUF_VIDEO_COMP1_SCALE_FACTOR,
						 HW_REG_ENCODE_FIELD(DP, AV_BUF_VIDEO_COMP1_SCALE_FACTOR, VID_SCA_FACT1, 0x10101));
	HW_REG_WRITE1(DP, AV_BUF_VIDEO_COMP1_SCALE_FACTOR,
						 HW_REG_ENCODE_FIELD(DP, AV_BUF_VIDEO_COMP2_SCALE_FACTOR, VID_SCA_FACT2, 0x10101) );
	HW_REG_WRITE1(DP, AV_BUF_GRAPHICS_COMP0_SCALE_FACTOR,
						 HW_REG_ENCODE_FIELD(DP, AV_BUF_GRAPHICS_COMP0_SCALE_FACTOR, GRAPHICS_SCALE_FACTOR0, 0x10101) );
	HW_REG_WRITE1(DP, AV_BUF_GRAPHICS_COMP1_SCALE_FACTOR,
						 HW_REG_ENCODE_FIELD(DP, AV_BUF_GRAPHICS_COMP1_SCALE_FACTOR, GRAPHICS_SCALE_FACTOR1, 0x10101));
	HW_REG_WRITE1(DP, AV_BUF_GRAPHICS_COMP1_SCALE_FACTOR,
						 HW_REG_ENCODE_FIELD(DP, AV_BUF_GRAPHICS_COMP2_SCALE_FACTOR, GRAPHICS_SCALE_FACTOR2, 0x10101) );


	HW_REG_WRITE1(DP, AV_BUF_SRST_REG, HW_REG_ENCODE_FIELD(DP, AV_BUF_SRST_REG, VID_RST, 1));
	Utils_BusyMilliSleep(10);
	HW_REG_WRITE1(DP, AV_BUF_SRST_REG, HW_REG_ENCODE_FIELD(DP, AV_BUF_SRST_REG, VID_RST, 0));

	HW_REG_WRITE1(DPDMA, CH0_CNTL, HW_REG_ENCODE_FIELD(DPDMA, CH0_CNTL, EN, 0));
	HW_REG_WRITE1(DPDMA, CH1_CNTL, HW_REG_ENCODE_FIELD(DPDMA, CH1_CNTL, EN, 0));
	HW_REG_WRITE1(DPDMA, CH2_CNTL, HW_REG_ENCODE_FIELD(DPDMA, CH2_CNTL, EN, 0));
	HW_REG_WRITE1(DPDMA, CH3_CNTL, HW_REG_ENCODE_FIELD(DPDMA, CH3_CNTL, EN, 0));

	if(planeEnable[0]) {
		assert(mixer->videoPlane.simpleDescPlane0Address);

		HW_REG_WRITE1(DPDMA, CH0_DSCR_STRT_ADDRE,
							 (uint32_t) (mixer->videoPlane.simpleDescPlane0Address >> 32ULL));
		HW_REG_WRITE1(DPDMA, CH0_DSCR_STRT_ADDR,
								 (uint32_t) (mixer->videoPlane.simpleDescPlane0Address & 0xFFFFFFFF));
		HW_REG_WRITE1(DPDMA, CH0_CNTL,
							 HW_REG_ENCODE_FIELD(DPDMA, CH0_CNTL, QOS_DATA_RD, 11) |
							 HW_REG_ENCODE_FIELD(DPDMA, CH0_CNTL, QOS_DSCR_RD, 11) |
							 HW_REG_ENCODE_FIELD(DPDMA, CH0_CNTL, QOS_DSCR_WR, 11) |
							 HW_REG_ENCODE_FIELD(DPDMA, CH0_CNTL, EN, 1));
		HW_REG_WRITE1(DPDMA, GBL, HW_REG_ENCODE_FIELD(DPDMA, GBL , TRG_CH0, 1));
	}

	if(planeEnable[1]) {
		assert(mixer->videoPlane.simpleDescPlane1Address);

		HW_REG_WRITE1(DPDMA, CH1_DSCR_STRT_ADDRE,
							 (uint32_t) (mixer->videoPlane.simpleDescPlane1Address >> 32ULL));
		HW_REG_WRITE1(DPDMA, CH1_DSCR_STRT_ADDR,
							 (uint32_t) (mixer->videoPlane.simpleDescPlane1Address & 0xFFFFFFFF));
		HW_REG_WRITE1(DPDMA, CH1_CNTL,
							 HW_REG_ENCODE_FIELD(DPDMA, CH1_CNTL, QOS_DATA_RD, 11) |
							 HW_REG_ENCODE_FIELD(DPDMA, CH1_CNTL, QOS_DSCR_RD, 11) |
							 HW_REG_ENCODE_FIELD(DPDMA, CH1_CNTL, QOS_DSCR_WR, 11) |
							 HW_REG_ENCODE_FIELD(DPDMA, CH1_CNTL, EN, 1));
		HW_REG_WRITE1(DPDMA, GBL, HW_REG_ENCODE_FIELD(DPDMA, GBL , TRG_CH1, 1));
	}

	if(planeEnable[2]) {
		assert(mixer->videoPlane.simpleDescPlane2Address);

		HW_REG_WRITE1(DPDMA, CH2_DSCR_STRT_ADDRE,
							 (uint32_t) (mixer->videoPlane.simpleDescPlane2Address >> 32ULL));
		HW_REG_WRITE1(DPDMA, CH2_DSCR_STRT_ADDR,
							 (uint32_t) (mixer->videoPlane.simpleDescPlane2Address & 0xFFFFFFFF));
		HW_REG_WRITE1(DPDMA, CH2_CNTL,
							 HW_REG_ENCODE_FIELD(DPDMA, CH2_CNTL, QOS_DATA_RD, 11) |
							 HW_REG_ENCODE_FIELD(DPDMA, CH2_CNTL, QOS_DSCR_RD, 11) |
							 HW_REG_ENCODE_FIELD(DPDMA, CH2_CNTL, QOS_DSCR_WR, 11) |
							 HW_REG_ENCODE_FIELD(DPDMA, CH2_CNTL, EN, 1));
		HW_REG_WRITE1(DPDMA, GBL, HW_REG_ENCODE_FIELD(DPDMA, GBL , TRG_CH2, 1));
	}

	if(planeEnable[3]) {
		assert(mixer->gfxPlane.simpleDescBufferAddress);

		HW_REG_WRITE1(DPDMA, CH3_DSCR_STRT_ADDRE,
							 (uint32_t) (mixer->gfxPlane.simpleDescBufferAddress >> 32ULL));
		HW_REG_WRITE1(DPDMA, CH3_DSCR_STRT_ADDR,
							 (uint32_t) (mixer->gfxPlane.simpleDescBufferAddress & 0xFFFFFFFF));
		HW_REG_WRITE1(DPDMA, CH3_CNTL,
							 HW_REG_ENCODE_FIELD(DPDMA, CH3_CNTL, QOS_DATA_RD, 11) |
							 HW_REG_ENCODE_FIELD(DPDMA, CH3_CNTL, QOS_DSCR_RD, 11) |
							 HW_REG_ENCODE_FIELD(DPDMA, CH3_CNTL, QOS_DSCR_WR, 11) |
							 HW_REG_ENCODE_FIELD(DPDMA, CH3_CNTL, EN, 1));
		HW_REG_WRITE1(DPDMA, GBL, HW_REG_ENCODE_FIELD(DPDMA, GBL , TRG_CH3, 1));
	}

	HW_REG_WRITE1(DP, AV_BUF_OUTPUT_AUDIO_VIDEO_SELECT,
							 HW_REG_ENCODE_FIELD(DP, AV_BUF_OUTPUT_AUDIO_VIDEO_SELECT, VID_STREAM1_SEL,
									 (uint32_t) mixer->videoPlane.source) |
							 HW_REG_ENCODE_FIELD(DP, AV_BUF_OUTPUT_AUDIO_VIDEO_SELECT, VID_STREAM2_SEL,
									 (uint32_t) mixer->gfxPlane.source) |
							 HW_REG_ENCODE_FIELD(DP, AV_BUF_OUTPUT_AUDIO_VIDEO_SELECT, AUD_STREAM1_SEL, 3) |
							 HW_REG_ENCODE_FIELD(DP, AV_BUF_OUTPUT_AUDIO_VIDEO_SELECT, AUD_STREAM2_SEL, 0));

	/*
	 * 	HW_REG_SET_BIT1(DP, INT_EN, VBLNK_START);
	for(int i = 0; i < 10;i++) {
		debug_printf("DP_INT_STATUS 0x%x\n", HW_REG_READ1(DP, INT_STATUS));
		debug_printf("DPDMA_ISR 0x%x\n", HW_REG_READ1(DPDMA, ISR));
		debug_printf("DPDMA_EISR 0x%x\n", HW_REG_READ1(DPDMA, EISR));
		HW_REG_WRITE1(DP, INT_STATUS, HW_REG_READ1(DP, INT_STATUS));
		HW_REG_WRITE1(DPDMA, ISR, HW_REG_READ1(DPDMA, ISR));
		HW_REG_WRITE1(DPDMA, EISR, HW_REG_READ1(DPDMA, EISR));
		Utils_BusyMicroSleep(100);
	}*/
}

bool CopyStandardVideoMode(StandardVideoMode videoMode, VideoTiming *videoTiming) {
	switch(videoMode) {
		case StandardVideoMode::VM_640_480_60:
			*videoTiming = VideoTiming {
				 .width = 640, .hFrontPorch = 16, .hSyncPulseWidth = 96, .hBackPorch= 48, .hTotal = 800,
				 .height = 480, .vFrontPorch= 10, .vSyncPulseWidth = 2, .vBackPorch = 33, .vTotal = 525,
				 .hSyncPolarity = true, .vSyncPolarity= true, .frameRateHz = 59.94 };
			 return true;
		case StandardVideoMode::VM_800_600_60:
			*videoTiming = VideoTiming {
					.width = 800, .hFrontPorch = 40, .hSyncPulseWidth = 128, .hBackPorch= 88, .hTotal = 1056,
					.height = 600, .vFrontPorch= 1, .vSyncPulseWidth = 4, .vBackPorch = 23, .vTotal = 628,
					.hSyncPolarity = true, .vSyncPolarity= true, .frameRateHz = 60 };
			return true;
		case StandardVideoMode::VM_1280_720_60:
			*videoTiming = VideoTiming {
					.width = 1280, .hFrontPorch = 110, .hSyncPulseWidth = 40, .hBackPorch= 220, .hTotal = 1650,
					.height = 720, .vFrontPorch= 5, .vSyncPulseWidth = 5, .vBackPorch = 20, .vTotal = 750,
					.hSyncPolarity = true, .vSyncPolarity= true, .frameRateHz = 60 };
			return true;
		case StandardVideoMode::VM_1920_1080_60:
			*videoTiming = VideoTiming {
					.width = 1920, .hFrontPorch = 88, .hSyncPulseWidth = 44, .hBackPorch= 148, .hTotal = 2200,
					.height = 1080, .vFrontPorch= 4, .vSyncPulseWidth = 5, .vBackPorch = 36, .vTotal = 1125,
					.hSyncPolarity = true, .vSyncPolarity= true, .frameRateHz = 60 };
			return true;
	}
	return false;
}

bool CopyNativeResolution(Connection *link, VideoTiming *videoTiming) {
	uint8_t edidBlock0[128];
	uint64_t header = 0x00FFFFFFFFFFFF00;
	if (DisplayPort::Display::Eedid::ReadBlock(link, 0, edidBlock0)) {
		if (memcmp(edidBlock0, &header, 8) != 0) {
			debug_print("EdidHeader block 0 not correct\n");
			return false;
		}
	} else {
		debug_print("ERROR ReadEdidBlock block 0\n");
		return false;
	}

	if (!HW_REG_DECODE_BIT(EEDID, FEATURE_SUPPORT, PTM_IS_NATIVE, edidBlock0[EEDID_FEATURE_SUPPORT_OFFSET])) {
		debug_print("Monitor doesn't have a native resolution\n");
		return false;
	}

#define DTD_REG(x) edidBlock0[EEDID_DTD_PIXEL_CLOCK_LSB_1_OFFSET + (EEDID_DTD_##x##_OFFSET - EEDID_DTD_PIXEL_CLOCK_LSB_1_OFFSET)]
	if ((DTD_REG(PIXEL_CLOCK_LSB_1) == 0 && DTD_REG(PIXEL_CLOCK_MSB_1) == 0)) {
		debug_print("Monitor doesn't have native resolution timing data\n");
		return false;
	}

	uint32_t const pixelClockKHz = ((DTD_REG(PIXEL_CLOCK_MSB_1) << 8) | DTD_REG(PIXEL_CLOCK_LSB_1)) * 10;

	uint16_t const hBlank = (HW_REG_DECODE_FIELD(EEDID, DTD_HA_HB_MSB_1, HB_MSB, DTD_REG(HA_HB_MSB_1)) << 8) +
			DTD_REG(HORIZONTAL_BLANKING_LSB_1);
	uint16_t const vBlank = (HW_REG_DECODE_FIELD(EEDID, DTD_VA_VB_MSB_1, VB_MSB, DTD_REG(VA_VB_MSB_1)) << 8) +
			DTD_REG(VERTICAL_BLANKING_LSB_1);
	uint16_t const hActive = (HW_REG_DECODE_FIELD(EEDID, DTD_HA_HB_MSB_1, HA_MSB, DTD_REG(HA_HB_MSB_1)) << 8) +
			DTD_REG(HORIZONTAL_ADDRESSABLE_LSB_1);
	uint16_t const vActive = (HW_REG_DECODE_FIELD(EEDID, DTD_VA_VB_MSB_1, VA_MSB, DTD_REG(VA_VB_MSB_1)) << 8) +
			DTD_REG(VERTICAL_ADDRESSABLE_LSB_1);

	uint8_t const hfp_hsp_vfp_vsp = DTD_REG(HFP_HSP_VFP_VSP_MSB_1);
	uint8_t const vfp_vsp_lsb = DTD_REG(VERTICAL_FRONT_PORCH_VERTICAL_SYNC_PULSE_LSB_1);

	uint16_t const hFrontPorch =
			(HW_REG_DECODE_FIELD(EEDID, DTD_HFP_HSP_VFP_VSP_MSB_1, HFP_MSB, hfp_hsp_vfp_vsp) << 8) +
					DTD_REG(HORIZONTAL_FRONT_PORCH_LSB_1);
	uint16_t const hSyncPulseWidth =
			(HW_REG_DECODE_FIELD(EEDID, DTD_HFP_HSP_VFP_VSP_MSB_1, HSP_MSB, hfp_hsp_vfp_vsp) << 8) +
					DTD_REG(HORIZONTAL_SYNC_PULSE_LSB_1);
	uint16_t const vFrontPorch =
			(HW_REG_DECODE_FIELD(EEDID, DTD_HFP_HSP_VFP_VSP_MSB_1, VFP_MSB, hfp_hsp_vfp_vsp) << 4) +
					HW_REG_DECODE_FIELD(EEDID, DTD_VERTICAL_FRONT_PORCH_VERTICAL_SYNC_PULSE_LSB_1, VFP_LSB, vfp_vsp_lsb);
	uint16_t const vSyncPulseWidth =
			(HW_REG_DECODE_FIELD(EEDID, DTD_HFP_HSP_VFP_VSP_MSB_1, VSP_MSB, hfp_hsp_vfp_vsp) << 4) +
					HW_REG_DECODE_FIELD(EEDID, DTD_VERTICAL_FRONT_PORCH_VERTICAL_SYNC_PULSE_LSB_1, VSP_LSB, vfp_vsp_lsb);

	uint16_t const hBackPorch = hBlank - (hFrontPorch + hSyncPulseWidth);
	uint16_t const vBackPorch = vBlank - (vFrontPorch + vSyncPulseWidth);
	uint16_t const hTotal = hSyncPulseWidth + hFrontPorch + hActive + hBackPorch;
	uint16_t const vTotal = vSyncPulseWidth + vFrontPorch + vActive + vBackPorch;
	bool const hSyncPol = !HW_REG_DECODE_BIT(EEDID, DTD_FLAGS_1, DIGITAL_HORIZONTAL_SYNC_IS_POSITIVE, DTD_REG(FLAGS_1));
	bool const vSyncPol =
			!HW_REG_DECODE_BIT(EEDID, DTD_FLAGS_1, SERRATIONS_OR_DIGITAL_SEPERATE_SYNC_IS_POSITIVE, DTD_REG(FLAGS_1));
#undef DTD_REG

	videoTiming->width = hActive;
	videoTiming->hFrontPorch = hFrontPorch;
	videoTiming->hSyncPulseWidth = hSyncPulseWidth;
	videoTiming->hBackPorch = hBackPorch;
	videoTiming->hTotal = hTotal;

	videoTiming->height = vActive;
	videoTiming->vFrontPorch = vFrontPorch;
	videoTiming->vSyncPulseWidth = vSyncPulseWidth;
	videoTiming->vBackPorch = vBackPorch;
	videoTiming->vTotal = vTotal;

	videoTiming->hSyncPolarity = hSyncPol;
	videoTiming->vSyncPolarity = vSyncPol;
	videoTiming->frameRateHz = (pixelClockKHz*1000) / (hTotal + vTotal);

	return true;
}

void SetVBlankInterrupt(bool enable) {
	if(enable) {
		HW_REG_SET_BIT1(DP, INT_EN, VBLNK_START);
	} else {
		HW_REG_SET_BIT1(DP, INT_DS, VBLNK_START);
	}
}

void SetCounterMatch0Interrupt(bool enable) {
	if(enable) {
		HW_REG_SET_BIT1(DP, INT_EN, PIXEL0_MATCH);
	} else {
		HW_REG_SET_BIT1(DP, INT_DS, PIXEL0_MATCH);
	}
	HW_REG_WRITE1(DP, AV_BUF_HCOUNT_VCOUNT_INT0, 0xFFFF'FFFF);

}

void SetCounterMatch1Interrupt(bool enable) {
	if(enable) {
		HW_REG_SET_BIT1(DP, INT_EN, PIXEL1_MATCH);
	} else {
		HW_REG_SET_BIT1(DP, INT_DS, PIXEL1_MATCH);
	}
	HW_REG_WRITE1(DP, AV_BUF_HCOUNT_VCOUNT_INT1, 0xFFFF'FFFF);
}


} // end namespace


