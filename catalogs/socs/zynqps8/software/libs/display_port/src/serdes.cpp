#include "core/core.h"
#include "zynqps8/display_port/display.hpp"
#include "zynqps8/display_port/aux.hpp"
#include "zynqps8/display_port/serdes.hpp"
#include "core/math.h"
#include "utils/busy_sleep.h"
#include "dbg/print.h"
#include "platform/reg_access.h"
#include "platform/memory_map.h"
#include "platform/registers/dp.h"
#include "platform/registers/dp_dpcd.h"
#include "platform/registers/serdes.h"


namespace DisplayPort::Display::Serdes {
static void SetVoltageSwing(Connection* display) {
	/**
	 *Voltage Swing Value table
	 */
	static const uint8_t vs[4][4] = {
			{ 0x2a, 0x27, 0x24, 0x20 },
			{ 0x27, 0x23, 0x20, 0xff },
			{ 0x24, 0x20, 0xff, 0xff },
			{ 0xff, 0xff, 0xff, 0xff },
	};

	uint8_t const peLevelRx = display->preEmphasis;
	uint8_t const vsLevelRx = display->voltageSwing;
	uint8_t const swing = vs[peLevelRx][vsLevelRx];
	for(int i=0;i < display->numLanes;++i) {
		switch(i) {
			case 0: HW_REG_WRITE1(SERDES, L0_TXPMD_TM_48, swing); break;
			case 1: HW_REG_WRITE1(SERDES, L1_TXPMD_TM_48, swing); break;
			default: break;
		}
	}
}

static void SetPreEmphasis(Connection* display) {
	/**
	 * PreEmphasis Value table
	 */
	static const uint8_t pe[4][4] = {
			{ 0x02, 0x02, 0x02, 0x02 },
			{ 0x01, 0x01, 0x01, 0xff },
			{ 0x00, 0x00, 0xff, 0xff },
			{ 0xff, 0xff, 0xff, 0xff },
	};
	uint8_t const peLevelRx = display->preEmphasis;
	uint8_t const vsLevelRx = display->voltageSwing;
	uint8_t const preEmp = pe[peLevelRx][vsLevelRx];
	for(int i=0;i < display->numLanes;++i) {
		switch(i) {
			case 0: HW_REG_WRITE1(SERDES, L0_TX_ANA_TM_18, preEmp); break;
			case 1: HW_REG_WRITE1(SERDES, L1_TX_ANA_TM_18, preEmp); break;
			default: break;
		}
	}
}


bool CheckLanesAligned(Connection const* display) {
	return HW_REG_DECODE_FIELD(DP_DPCD, LANE_ALIGN_STATUS_UPDATED, INTERLANE_ALIGN_DONE, display->laneAlignStatus);
}

bool CheckLanesClockRecovery(Connection const* display) {
	switch(display->numLanes) {
		case 2:
			if(!HW_REG_DECODE_FIELD(DP_DPCD, LANE_0_1_STATUS, LANE1_CR_DONE, display->lane_0_1Status)) return false;
			[[fallthrough]];
		case 1:
			if(HW_REG_DECODE_FIELD(DP_DPCD, LANE_0_1_STATUS, LANE0_CR_DONE, display->lane_0_1Status)) return true;
		default:
		case 0:
			return false;
	}
}

bool CheckLanesChannelEqualisation(Connection const* display) {
	switch(display->numLanes) {
		case 2:
			if(!HW_REG_DECODE_FIELD(DP_DPCD, LANE_0_1_STATUS, LANE1_CHANNEL_EQ_DONE, display->lane_0_1Status)) return false;
			[[fallthrough]];
		case 1:
			if(HW_REG_DECODE_FIELD(DP_DPCD, LANE_0_1_STATUS, LANE0_CHANNEL_EQ_DONE, display->lane_0_1Status)) break;
		default:
		case 0:
			return false;
	}

	switch(display->numLanes) {
		case 2:
			if(!HW_REG_DECODE_FIELD(DP_DPCD, LANE_0_1_STATUS, LANE1_SYMBOL_LOCKED, display->lane_0_1Status)) return false;
			[[fallthrough]];
		case 1:
			if(HW_REG_DECODE_FIELD(DP_DPCD, LANE_0_1_STATUS, LANE0_SYMBOL_LOCKED, display->lane_0_1Status))
				return CheckLanesAligned(display);
		default:
		case 0:
			return false;
	}
}


LinkStatus CheckLinkStatus(Connection const* display) {
	int status = LS_OKAY;
	if(!Serdes::CheckLanesClockRecovery(display)) {
		debug_print("CheckLanesClockRecovery failed\n");
		status |= LS_CLOCK_RECOVERY_FAILED;
	}
	if(!Serdes::CheckLanesChannelEqualisation(display)) {
		debug_print("CheckLanesChannelEqualisation failed\n");
		status |= LS_CHANNEL_EQUALISATION_FAILED;
	}
	if(!Serdes::CheckLanesAligned(display)) {
		debug_print("CheckLanesAligned failed\n");
		status |= LS_LANES_ALIGNED_FAILED;
	}
	return (LinkStatus)status;
}

void ResetPhy(Connection* display) {
	HW_REG_CLR_BIT1(DP, TRANSMITTER_ENABLE, TX_EN);

	HW_REG_WRITE1(DP, PHY_RESET,
						 HW_REG_ENCODE_FIELD(DP, PHY_RESET, EN_8B_10B, 1) |
								 HW_REG_ENCODE_FIELD(DP, PHY_RESET, PHY_RESET, 1 ) );

	Utils_BusyMicroSleep(100);

	HW_REG_WRITE1(DP, PHY_RESET,
						 HW_REG_ENCODE_FIELD(DP, PHY_RESET, EN_8B_10B, 1) |
								 HW_REG_ENCODE_FIELD(DP, PHY_RESET, PHY_RESET, 0) );

	Serdes::StallForPhyReady(display);

	HW_REG_SET_BIT1(DP, TRANSMITTER_ENABLE, TX_EN);
}

void UpdateLaneStatusAdjReqs(Connection* display) {
	uint8_t sinkData[8];
	if(AuxRead(display, DP_DPCD_SINK_COUNT_OFFSET, 8, sinkData)) {
		display->lane_0_1Status = sinkData[2];
		display->laneAlignStatus = sinkData[4];

		uint8_t voltageSwingAdjustRequired[2] = {};
		uint8_t preEmphasisAdjustRequired[2] = {};
		voltageSwingAdjustRequired[0] = HW_REG_DECODE_FIELD(DP_DPCD, ADJUST_REQUEST_LANE0_1, VOLTAGE_SWING_LANE0, sinkData[6]);
		voltageSwingAdjustRequired[1] = HW_REG_DECODE_FIELD(DP_DPCD, ADJUST_REQUEST_LANE0_1, VOLTAGE_SWING_LANE1, sinkData[6]);
		preEmphasisAdjustRequired[0] = HW_REG_DECODE_FIELD(DP_DPCD, ADJUST_REQUEST_LANE0_1, PRE_EMPHASIS_LANE0, sinkData[6]);
		preEmphasisAdjustRequired[1] = HW_REG_DECODE_FIELD(DP_DPCD, ADJUST_REQUEST_LANE0_1, PRE_EMPHASIS_LANE1, sinkData[6]);

		for(int i =0;i < display->numLanes;++i) {
			display->voltageSwing = Math_Max_U8(display->voltageSwing, voltageSwingAdjustRequired[i]);
			display->preEmphasis = Math_Max_U8(display->preEmphasis, preEmphasisAdjustRequired[i]);
		}
		display->voltageSwing = Math_Min_U8(display->voltageSwing, Connection::MaxVoltageSwing);
		display->preEmphasis = Math_Min_U8(display->preEmphasis, Connection::MaxPreEmphasis);
		if(display->preEmphasis > (4 - display->voltageSwing)) {
			display->preEmphasis = 4 - display->voltageSwing;
		}
	} else {
		debug_print("DP AuxRead @ DP_DPCD_SINK_COUNT failed\n");
	}
}


bool SetVoltageSwingAndPreEmphasis(Connection* display) {
	uint8_t data = 0;
	if(display->voltageSwing >= Connection::MaxVoltageSwing) {
		data |= DP_DPCD_TRAINING_LANE0_SET_MAX_SWING_REACHED;
	}
	if(display->preEmphasis >= Connection::MaxPreEmphasis) {
		data |= DP_DPCD_TRAINING_LANE0_SET_MAX_PRE_EMPHASIS_REACHED;
	}
	data |= HW_REG_ENCODE_FIELD(DP_DPCD, TRAINING_LANE0_SET, VOLTAGE_SWING, display->voltageSwing);
	data |= HW_REG_ENCODE_FIELD(DP_DPCD, TRAINING_LANE0_SET, PRE_EMPHASIS, display->preEmphasis);
	uint8_t auxData[4] = { data, data, data, data };

	SetVoltageSwing(display);
	SetPreEmphasis(display);

	if(AuxWrite(display, DP_DPCD_TRAINING_LANE0_SET_OFFSET, 4, auxData)) {
		return true;
	} else {
		debug_printf("DP AuxWrite @ DP_DPCD_TRAINING_LANE0_SET_OFFSET failed\n");
		return false;
	}
}


void StallForPhyReady(Connection *display) {
	uint32_t timeout = 100;

	uint8_t const readyMask = display->numLanes == 1 ?
														HW_REG_ENCODE_FIELD(DP, PHY_STATUS, RESET_LANES_0_1, 0x1) |
																HW_REG_ENCODE_FIELD(DP, PHY_STATUS, PLL_LOCKED , 0x1)
																									 :
														HW_REG_ENCODE_FIELD(DP, PHY_STATUS, RESET_LANES_0_1, 0x3) |
																HW_REG_ENCODE_FIELD(DP, PHY_STATUS, PLL_LOCKED , 0x1);
	do {
		if((HW_REG_READ1(DP, PHY_STATUS) & readyMask) == readyMask) return;
		Utils_BusyMicroSleep(20);
		timeout--;
	} while(timeout >= 0);

	debug_print("StallForPhyRead timeout");
}

bool SetDownSpread(Connection* display, bool enable) {
	if(display->supportsDownSpread == false) {
		return false;
	}

	if(display->downSpreadEnabled == enable) {
		return true;
	}

	uint8_t downSpreadCtrlReg = (display->msaTimingParIgnored) ?
															HW_REG_ENCODE_FIELD(DP_DPCD, DOWNSPREAD_CTRL, MSA_TIMING_PAR_IGNORED, 1) : 0;

	if(enable) {
		downSpreadCtrlReg |= HW_REG_ENCODE_FIELD(DP_DPCD, DOWNSPREAD_CTRL, SPREAD_AMP, 1);
	}

	if(!AuxWrite(display, DP_DPCD_DOWNSPREAD_CTRL_OFFSET, 1, &downSpreadCtrlReg)) {
		debug_print("DP AuxWrite @ DP_DPCD_DOWNSPREAD_CTRL failed\n");
		return false;

	}

	HW_REG_WRITE1(DP, DOWNSPREAD_CTRL, enable);
	display->downSpreadEnabled = enable;

	return true;
}

bool SetEnhancedFrameMode(Connection* display, bool enable) {
	if(display->supportsEnhancedFrame == false) {
		return false;
	}

	if(display->enhancedFrameEnabled == enable) {
		return true;
	}

	uint8_t laneCountSetReg = HW_REG_ENCODE_FIELD(DP_DPCD, LANE_COUNT_SET, LANE_COUNT, display->numLanes);

	if(enable) {
		laneCountSetReg |= HW_REG_ENCODE_FIELD(DP_DPCD, LANE_COUNT_SET, ENHANCED_FRAME_EN, 1);
	}

	if(!AuxWrite(display, DP_DPCD_LANE_COUNT_SET_OFFSET, 1, &laneCountSetReg)) {
		debug_print("DP AuxWrite @ DP_DPCD_LANE_COUNT_SET failed\n");
		return false;

	}

	HW_REG_WRITE1(DP, ENHANCED_FRAME_EN, enable);
	display->enhancedFrameEnabled = enable;

	return true;
}

void SetLinkRate(Connection* display, LinkRate rate) {
	auto const linkRate = (uint8_t)rate;
	HW_REG_CLR_BIT1(DP, TRANSMITTER_ENABLE, TX_EN);

	switch (rate) {
		case LinkRate::Rate_1_62Gbps:
			HW_REG_WRITE1(DP, PHY_CLOCK_SELECT,
								 HW_REG_ENCODE_ENUM(DP, PHY_CLOCK_SELECT, SEL, LINK_1_62GBS));
			break;
		case LinkRate::Rate_2_7Gbps:
			HW_REG_WRITE1(DP, PHY_CLOCK_SELECT,
								 HW_REG_ENCODE_ENUM(DP, PHY_CLOCK_SELECT, SEL, LINK_2_7GBS));
			break;
		case LinkRate::Rate_5_4Gbps:
			HW_REG_WRITE1(DP, PHY_CLOCK_SELECT,
								 HW_REG_ENCODE_ENUM(DP, PHY_CLOCK_SELECT, SEL, LINK_5_4GBS));
			break;
	}

	HW_REG_WRITE1(DP, LINK_BW_SET, (uint32_t)rate);

	HW_REG_SET_BIT1(DP, TRANSMITTER_ENABLE, TX_EN);

	StallForPhyReady(display);

	if(!AuxWrite(display,DP_DPCD_LINK_BW_SET_OFFSET, 1, &linkRate)) {
		debug_print("DP AuxWrite @ DP_DPCD_LINK_BW_SET failed\n");
		return;
	}

	display->linkRate = rate;

	HW_REG_WRITE1(DP, LINK_BW_SET, linkRate);

}

void SetLaneCount(Connection* display, unsigned int count) {
	uint8_t laneCountSetReg = HW_REG_ENCODE_FIELD(DP_DPCD, LANE_COUNT_SET, LANE_COUNT, count);

	if(display->enhancedFrameEnabled) {
		laneCountSetReg |= HW_REG_ENCODE_FIELD(DP_DPCD, LANE_COUNT_SET, ENHANCED_FRAME_EN, 1);
	}

	if(!AuxWrite(display, DP_DPCD_LANE_COUNT_SET_OFFSET, 1, &laneCountSetReg)) {
		debug_print("DP AuxWrite @ DP_DPCD_LANE_COUNT_SET failed\n");
		return;
	}

	HW_REG_WRITE1(DP, LANE_COUNT_SET, count);
	display->numLanes = count;

}

} // end namespace