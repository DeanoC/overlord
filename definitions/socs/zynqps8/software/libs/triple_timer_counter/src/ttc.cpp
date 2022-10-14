#include "core/core.h"
#include "dbg/print.h"
#include "dbg/assert.h"
#include "platform/reg_access.h"
#include "platform/a53/memory_map.h"
#include "platform/registers/ttc.h"
#include "zynqps8/triple_timer_counter/ttc.hpp"

#define TTC_CLK_FREQ_HZ 100000000U

namespace TripleTimerCounter {
static uint32_t NameToRegister(Name const name_) {
	switch (name_) {
		case Name::TTC_1_1:
		case Name::TTC_1_2:
		case Name::TTC_1_3:
			return TTC0_BASE_ADDR;
		case Name::TTC_2_1:
		case Name::TTC_2_2:
		case Name::TTC_2_3:
			return TTC1_BASE_ADDR;
		case Name::TTC_3_1:
		case Name::TTC_3_2:
		case Name::TTC_3_3:
			return TTC2_BASE_ADDR;
		case Name::TTC_4_1:
		case Name::TTC_4_2:
		case Name::TTC_4_3:
			return TTC3_BASE_ADDR;
	}
	return TTC0_BASE_ADDR;
}

static uint32_t NameToTimer(Name const name_) {
	switch (name_) {
		case Name::TTC_1_1:
		case Name::TTC_2_1:
		case Name::TTC_3_1:
		case Name::TTC_4_1:
			return 0;
		case Name::TTC_1_2:
		case Name::TTC_2_2:
		case Name::TTC_3_2:
		case Name::TTC_4_2:
			return 1;
		case Name::TTC_1_3:
		case Name::TTC_2_3:
		case Name::TTC_3_3:
		case Name::TTC_4_3:
			return 2;
	}
	return 0;
}
static void SetCounterControlBits(Name const name_, uint32_t bits_) {
	uint32_t const name = NameToRegister(name_);
	uint32_t const index = NameToTimer(name_) * 4;
	uint32_t counterControl = hw_RegRead( name,TTC_COUNTER_CONTROL_1_OFFSET + index);
	counterControl = counterControl | bits_;
	hw_RegWrite(name,TTC_COUNTER_CONTROL_1_OFFSET + index,counterControl);
}
static void ClearCounterControlBits(Name const name_, uint32_t bits_) {
	uint32_t const name = NameToRegister(name_);
	uint32_t const index = NameToTimer(name_) * 4;
	uint32_t counterControl = hw_RegRead( name,TTC_COUNTER_CONTROL_1_OFFSET + index);
	counterControl = counterControl & ~bits_;
	hw_RegWrite(name,TTC_COUNTER_CONTROL_1_OFFSET + index,counterControl);
}
static void SetClockControlBits(Name const name_, uint32_t bits_) {
	uint32_t const name = NameToRegister(name_);
	uint32_t const index = NameToTimer(name_) * 4;
	uint32_t clockControl = hw_RegRead( name,TTC_CLOCK_CONTROL_1_OFFSET + index);
	clockControl = clockControl | bits_;
	hw_RegWrite(name,TTC_CLOCK_CONTROL_1_OFFSET + index,clockControl);
}
static void ClearClockControlBits(Name const name_, uint32_t bits_) {
	uint32_t const name = NameToRegister(name_);
	uint32_t const index = NameToTimer(name_) * 4;
	uint32_t clockControl = hw_RegRead( name,TTC_CLOCK_CONTROL_1_OFFSET + index);
	clockControl = clockControl & ~bits_;
	hw_RegWrite(name,TTC_CLOCK_CONTROL_1_OFFSET + index,clockControl);
}


void SetInterval(Name const name_, uint32_t value_) {
	hw_RegWrite(NameToRegister(name_),TTC_INTERVAL_COUNTER_1_OFFSET + (NameToTimer(name_) * 4),value_);
}

uint32_t GetInterval(Name const name_) {
	return hw_RegRead(NameToRegister(name_),TTC_INTERVAL_COUNTER_1_OFFSET + (NameToTimer(name_) * 4));
}

void SetCounter(Name const name_, uint32_t value_) {
	hw_RegWrite(NameToRegister(name_),TTC_COUNTER_VALUE_1_OFFSET + (NameToTimer(name_) * 4),value_);
}

uint32_t GetCounter(Name const name_) {
	return hw_RegRead(NameToRegister(name_),TTC_COUNTER_VALUE_1_OFFSET + (NameToTimer(name_) * 4));
}

void SetType(Name const name_, Type type_) {
	switch(type_) {
		case Type::Counter: ClearCounterControlBits(name_, HW_REG_FIELD_MASK(TTC, COUNTER_CONTROL_1, INT)); break;
		case Type::Interval: SetCounterControlBits(name_, HW_REG_FIELD_MASK(TTC, COUNTER_CONTROL_1, INT)); break;
	}
}

void EnableMatchInterrupts(Name const name_) {
	SetCounterControlBits(name_, HW_REG_FIELD_MASK(TTC, COUNTER_CONTROL_1, MATCH));
}

void DisableMatchInterrupts(Name const name_, uint32_t match_) {
	ClearCounterControlBits(name_, HW_REG_FIELD_MASK(TTC, COUNTER_CONTROL_1, MATCH));
}

void SetMatch(Name const name_, uint32_t index_, uint32_t match_) {
	hw_RegWrite(NameToRegister(name_),
							TTC_MATCH_1_COUNTER_1_OFFSET + (NameToTimer(name_) * 12) + index_,
							match_);
}

void StartInterrupts(Name const name_, uint32_t interrupt_) {
	uint32_t const name = NameToRegister(name_);
	uint32_t const index = NameToTimer(name_) * 4;
	uint32_t interruptEnable = hw_RegRead( name,TTC_INTERRUPT_REGISTER_1_OFFSET + index);
	interruptEnable |= (uint32_t)interrupt_;
	hw_RegWrite(name, TTC_INTERRUPT_ENABLE_1_OFFSET + index, interruptEnable);
}

void StopInterrupts(Name const name_, Interrupt interrupt_) {
	uint32_t const name = NameToRegister(name_);
	uint32_t const index = NameToTimer(name_) * 4;
	uint32_t interruptEnable = hw_RegRead( name,TTC_INTERRUPT_REGISTER_1_OFFSET + index);
	interruptEnable &= ~((uint32_t)interrupt_);
	hw_RegWrite(name, TTC_INTERRUPT_ENABLE_1_OFFSET + index, interruptEnable);
}

void AckInterrupts(Name const name_) {
	uint32_t const name = NameToRegister(name_);
	uint32_t const index = NameToTimer(name_) * 4;
	hw_RegRead( name,TTC_INTERRUPT_REGISTER_1_OFFSET + index);
}

void Start(Name const name_) {
	ClearCounterControlBits(name_, HW_REG_FIELD_MASK(TTC, COUNTER_CONTROL_1, DIS));
	SetCounterControlBits(name_, HW_REG_FIELD_MASK(TTC, COUNTER_CONTROL_1, RST));
}

void Stop(Name const name_) {
	SetCounterControlBits(name_, HW_REG_FIELD_MASK(TTC, COUNTER_CONTROL_1, DIS));
}

IntervalAndPrescaler CalcIntervalFromFreq(uint32_t const desiredFreq_)
{
	uint32_t tmp = TTC_CLK_FREQ_HZ / desiredFreq_;

	// to close to input frequency (gives us a max 25MHz interval)
	if (tmp < 4U) {
		return IntervalAndPrescaler{ .interval = 0, .prescaler = 255 };
	}

	// valid without a prescaler
	if (tmp < ~0U) {
		return IntervalAndPrescaler{ .interval = tmp, .prescaler = 0 };
	}

	// try each prescaler from low to high until we find one that works
	for (int i = 1; i <= 16; i++) {
		tmp =	TTC_CLK_FREQ_HZ / (desiredFreq_ * (1U << i));

		if (tmp < ~0U) {
			return IntervalAndPrescaler{ .interval = tmp, .prescaler = (uint8_t)i };
		}
	}

	// no valid timing
	return IntervalAndPrescaler{ .interval = tmp, .prescaler = 0 };
}

void SetPrescaler(Name const name_, uint8_t value_) {
	assert(value_ <= 16);

	if(value_ == 0) {
		ClearClockControlBits(name_, TTC_CLOCK_CONTROL_1_PS_EN);
	} else {
		uint32_t const name = NameToRegister(name_);
		uint32_t const index = NameToTimer(name_) * 4;
		uint32_t clockControl = hw_RegRead( name,TTC_CLOCK_CONTROL_1_OFFSET + index);
		uint32_t const v = HW_REG_ENCODE_FIELD(TTC, CLOCK_CONTROL_1, PS_V, (value_ - 1));
		clockControl = (clockControl & (~v)) | v;
		hw_RegWrite(name,TTC_CLOCK_CONTROL_1_OFFSET + index,clockControl);
		SetClockControlBits(name_, TTC_CLOCK_CONTROL_1_PS_EN);
	}
}

} // end namespace