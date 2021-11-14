//
// Created by deano on 06/11/2021.
//
#include "core/core.h"
#include "timers.hpp"
#include "interrupts.hpp"
#include "hw/reg_access.h"
#include "hw/memory_map.h"
#include "hw_regs/pmu/pmu_iomodule.h"
#include "main_loop.hpp"

#define MICROBLAZE_FREQ_KHZ 180000

// PIT1 is 30Hz
#define PIT1_TICK_MILLISECONDS	33U
#define PIT1_COUNT_PER_TICK (MICROBLAZE_FREQ_KHZ * PIT1_TICK_MILLISECONDS)

namespace Timers {

static void PIT1_Handler(Interrupts::Name irq_name) {
	loopy.ThirtyHertzTrigger = true;
}

void Init() {
	// PIT1 (30Hz timer callback)
	HW_REG_SET(PMU_IOMODULE, PIT1_CONTROL, 0); // disable till ready
	HW_REG_SET(PMU_IOMODULE, PIT1_PRELOAD, PIT1_COUNT_PER_TICK);
	Interrupts::SetHandler(Interrupts::Name::IN_PIT1, &PIT1_Handler);
	Interrupts::Enable(Interrupts::Name::IN_PIT1);

}

void Start() {
	// kick off the timer
	HW_REG_SET(PMU_IOMODULE, PIT1_PRELOAD, PIT1_COUNT_PER_TICK);
	HW_REG_SET(PMU_IOMODULE, PIT1_CONTROL,
						 HW_REG_FIELD(PMU_IOMODULE, PIT1_CONTROL, PRELOAD) |
						 HW_REG_FIELD(PMU_IOMODULE, PIT1_CONTROL, EN) );
}

}