#include "core/core.h"
#include "zynqps8/display_port/train.hpp"
#include "zynqps8/display_port/display.hpp"
#include "zynqps8/display_port/serdes.hpp"
#include "zynqps8/display_port/aux.hpp"
#include "platform/reg_access.h"
#include "platform/memory_map.h"
#include "platform/registers/dp.h"
#include "platform/registers/dp_dpcd.h"
#include "utils/busy_sleep.h"
#include "dbg/print.h"
#include "dbg/assert.h"


namespace DisplayPort::Display {

enum class TrainingState {
	CLOCK_RECOVERY,
	CHANNEL_EQUALISATION,
	ADJUST_LINK_RATE,
	ADJUST_LANE_COUNT,
	FAILURE,
	SUCCESS
};

enum class TrainingPattern {
	DISABLED = HW_REG_FIELD_ENUM(DP_DPCD, TRAINING_PATTERN_SET, PATTERN_SELECT, DISABLED),
	PATTERN_1 = HW_REG_FIELD_ENUM(DP_DPCD, TRAINING_PATTERN_SET, PATTERN_SELECT, PATTERN_1),
	PATTERN_2 = HW_REG_FIELD_ENUM(DP_DPCD, TRAINING_PATTERN_SET, PATTERN_SELECT, PATTERN_2),
	PATTERN_3 = HW_REG_FIELD_ENUM(DP_DPCD, TRAINING_PATTERN_SET, PATTERN_SELECT, PATTERN_3),
};

static const uint32_t DISPLAY_PORT_ITERATION = 5;

static void SetTrainingPattern(Connection* display, TrainingPattern pattern) {
	HW_REG_WRITE1(DP, TRAINING_PATTERN_SET, (uint32_t)pattern);
	uint8_t tp = (uint8_t) pattern;

	switch (pattern) {
		case TrainingPattern::DISABLED:
			HW_REG_WRITE1(DP, SCRAMBLING_DISABLE, false);
			break;
		case TrainingPattern::PATTERN_1:
		case TrainingPattern::PATTERN_2:
		case TrainingPattern::PATTERN_3:
			tp |= HW_REG_FIELD(DP_DPCD, TRAINING_PATTERN_SET, SCRAMBLING_DISABLED);
			HW_REG_WRITE1(DP, SCRAMBLING_DISABLE, true);
			break;
	}
	if (!AuxWrite(display, DP_DPCD_TRAINING_PATTERN_SET_OFFSET, 1, &tp)) {
		debug_print("DP AuxRead @ DP_DPCD_TRAINING_PATTERN_SET failed\n");
	}

	if (!Serdes::SetVoltageSwingAndPreEmphasis(display)) {
		debug_print("Serdes::SetVoltageSwingAndPreEmphasis failed\n");
		return;
	}
}

static TrainingState TrainClockRecovery(Connection* display) {

	display->voltageSwing = 0;
	display->preEmphasis = 0;
	SetTrainingPattern(display, TrainingPattern::PATTERN_1);

	uint8_t prevVoltageSwing = 0xFF;
	uint8_t sameVoltageSwingCount = 0;

	while(true) {
		Utils_BusyMicroSleep(display->delayRateUS);
		Serdes::UpdateLaneStatusAdjReqs(display);
		if(Serdes::CheckLanesClockRecovery(display)) {
			// clock recovered
			return TrainingState::CHANNEL_EQUALISATION;
		}

		if(prevVoltageSwing == display->voltageSwing) {
			sameVoltageSwingCount++;
		} else {
			sameVoltageSwingCount = 0;
			prevVoltageSwing = display->voltageSwing;
		}
		if (sameVoltageSwingCount >= 5) {
			// clock unable to be recovered current settings so adjust link rate
			return TrainingState::ADJUST_LINK_RATE;
		}

		if (display->voltageSwing == Connection::MaxVoltageSwing) {
			// clock unable to be recovered current settings so adjust link rate
			return TrainingState::ADJUST_LINK_RATE;
		}

		Serdes::SetVoltageSwingAndPreEmphasis(display);
	}

}

static TrainingState TrainChannelEqualisation(Connection* display) {
	uint32_t iters = DISPLAY_PORT_ITERATION;
	if(display->supportsTrainingPattern3) {
		SetTrainingPattern(display, TrainingPattern::PATTERN_3);
	} else {
		SetTrainingPattern(display, TrainingPattern::PATTERN_2);
	}

	while(iters > 0) {
		Utils_BusyMicroSleep(display->delayRateUS);
		Serdes::UpdateLaneStatusAdjReqs(display);
		Serdes::SetVoltageSwingAndPreEmphasis(display);
		if(!Serdes::CheckLanesClockRecovery(display)) break;
		if(Serdes::CheckLanesChannelEqualisation(display)) return TrainingState::SUCCESS;

		iters--;
	}
	// equalisation failed at current settings so adjust the link rate
	return TrainingState::ADJUST_LINK_RATE;
}

static TrainingState AdjustLinkRate(Connection* display) {
	switch(display->linkRate) {
		default:
		case LinkRate::Rate_1_62Gbps:
			// lowest link rate, so try reducing lanes
			return TrainingState::ADJUST_LANE_COUNT;
		case LinkRate::Rate_2_7Gbps:
			// try at lower link rate
			Serdes::SetLinkRate(display, LinkRate::Rate_1_62Gbps);
			return TrainingState::CLOCK_RECOVERY;
		case LinkRate::Rate_5_4Gbps:
			// try at lower link rate
			Serdes::SetLinkRate(display, LinkRate::Rate_2_7Gbps);
			return TrainingState::CLOCK_RECOVERY;
	}
}

static TrainingState AdjustLaneCount(Connection* display) {
	if(display->numLanes == 1) {
		// can't reduce lanes anymore, nothing more to try
		return TrainingState::FAILURE;
	}
	// reduce number of lane and restart training at highest link rate
	Serdes::SetLaneCount(display, display->numLanes-1);
	Serdes::SetLinkRate(display, LinkRate::Rate_5_4Gbps);
	return TrainingState::CLOCK_RECOVERY;
}


bool TrainLink(Connection *display) {
	uint32_t timeout = 100;
	TrainingState state = TrainingState::CLOCK_RECOVERY;

	display->numLanes = 2;
	display->linkRate = LinkRate::Rate_5_4Gbps;
	Serdes::SetLaneCount(display, display->numLanes);
	Serdes::SetLinkRate(display, display->linkRate);

	do {
//		debug_printf("Train State %d\n", (int)state);
		switch(state) {
			case TrainingState::CLOCK_RECOVERY: state = TrainClockRecovery(display); break;
			case TrainingState::CHANNEL_EQUALISATION: state = TrainChannelEqualisation(display); break;
			case TrainingState::ADJUST_LINK_RATE: state = AdjustLinkRate(display); break;
			case TrainingState::ADJUST_LANE_COUNT: state = AdjustLaneCount(display); break;
			case TrainingState::FAILURE: return false;
			case TrainingState::SUCCESS: {
				// final check
				if(Serdes::CheckLinkStatus(display) != Serdes::LS_OKAY) {
					return false;
				} else {
					SetTrainingPattern(display, TrainingPattern::DISABLED);
					return true;
				}
			}
		}


		timeout--;
	} while(timeout >= 0);

	debug_print("TrainLink timeout");

	return false;
}

} // end namespace