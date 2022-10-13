#pragma once

#include "core/core.h"
#include "platform/zynqmp/interrupts.h"

namespace TripleTimerCounter {

enum class Name : uint8_t {
	TTC_1_1 = INT_TTC0_1,
	TTC_1_2 = INT_TTC0_2,
	TTC_1_3 = INT_TTC0_3,
	TTC_2_1 = INT_TTC1_1,
	TTC_2_2 = INT_TTC1_2,
	TTC_2_3 = INT_TTC1_3,
	TTC_3_1 = INT_TTC2_1,
	TTC_3_2 = INT_TTC2_2,
	TTC_3_3 = INT_TTC2_3,
	TTC_4_1 = INT_TTC3_1,
	TTC_4_2 = INT_TTC3_2,
	TTC_4_3 = INT_TTC3_3,
};

enum class Type : uint8_t {
	Counter = 0,
	Interval = 1
};

enum Interrupt {
	INT_Timer_Overflow 		= (1 << 5),
	INT_Counter_Overflow 	= (1 << 4),
	INT_Match_3 					= (1 << 3),
	INT_Match_2 					= (1 << 2),
	INT_Match_1 					= (1 << 1),
	INT_Interval 					= (1 << 0),
};

void SetInterval(Name const name_, uint32_t value_);
uint32_t GetInterval(Name const name_);

void SetCounter(Name const name_, uint32_t value_);
uint32_t GetCounter(Name const name_);

void SetType(Name const name_, Type type_);

void EnableMatchInterrupts(Name const name_);
void DisableMatchInterrupts(Name const name_);
void SetMatch(Name const name_, uint32_t index_, uint32_t match_);

void StartInterrupts(Name const name_, uint32_t interrupt_);
void StopInterrupts(Name const name_, uint32_t interrupt_);
void AckInterrupts(Name const name_);

void Start(Name const name_);
void Stop(Name const name_);

struct IntervalAndPrescaler {
	uint32_t interval;
	uint8_t prescaler; // prescaler of 255 means not possible
};

IntervalAndPrescaler CalcIntervalFromFreq(uint32_t const desiredFreq_);

// prescaler 0-16 (inclusive), value_: 0 = disable, (1 << value_), so 16 (max) == 65536
void SetPrescaler(Name const name_, uint8_t value_);

} // end namespace