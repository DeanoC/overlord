//
// Created by deano on 06/11/2021.
//
#include "core/core.h"
#include "timers.hpp"
#include "interrupts/interrupts.hpp"
#include "platform/reg_access.h"
#include "platform/memory_map.h"
#include "platform/registers/pmu_iomodule.h"
#include "main_loop.hpp"

#define MICROBLAZE_FREQ_KHZ 180000

// PIT1 is 30Hz
#define PIT1_TICK_MILLISECONDS	33U
#define PIT1_COUNT_PER_TICK (MICROBLAZE_FREQ_KHZ * PIT1_TICK_MILLISECONDS)
// PIT2 is 100Hz
#define PIT2_TICK_MILLISECONDS	10U
#define PIT2_COUNT_PER_TICK (MICROBLAZE_FREQ_KHZ * PIT2_TICK_MILLISECONDS)

namespace Timers {

static void PIT1_Handler(Interrupts::Name irq_name) {
	loopy.thirtyHertzTrigger = true;
}

static void PIT2_Handler(Interrupts::Name irq_name) {
	loopy.hundredHertzTrigger = true;
}

void Init() {
	// PIT1 (30Hz timer callback)
	HW_REG_WRITE1(PMU_IOMODULE, PIT1_CONTROL, 0); // disable till ready
	HW_REG_WRITE1(PMU_IOMODULE, PIT1_PRELOAD, PIT1_COUNT_PER_TICK);
	Interrupts::SetHandler(Interrupts::Name::IN_PIT1, &PIT1_Handler);
	Interrupts::Enable(Interrupts::Name::IN_PIT1);
	// PIT2 (100Hz timer callback)
	HW_REG_WRITE1(PMU_IOMODULE, PIT2_CONTROL, 0); // disable till ready
	HW_REG_WRITE1(PMU_IOMODULE, PIT2_PRELOAD, PIT2_COUNT_PER_TICK);
	Interrupts::SetHandler(Interrupts::Name::IN_PIT2, &PIT2_Handler);
	Interrupts::Enable(Interrupts::Name::IN_PIT2);

}

void Start() {
	// kick off the timers
	HW_REG_WRITE1(PMU_IOMODULE, PIT1_PRELOAD, PIT1_COUNT_PER_TICK);
	HW_REG_WRITE1(PMU_IOMODULE, PIT1_CONTROL,
						 HW_REG_FIELD(PMU_IOMODULE, PIT1_CONTROL, PRELOAD) |
						 HW_REG_FIELD(PMU_IOMODULE, PIT1_CONTROL, EN) );

	HW_REG_WRITE1(PMU_IOMODULE, PIT2_PRELOAD, PIT2_COUNT_PER_TICK);
	HW_REG_WRITE1(PMU_IOMODULE, PIT2_CONTROL,
						 HW_REG_FIELD(PMU_IOMODULE, PIT2_CONTROL, PRELOAD) |
						 HW_REG_FIELD(PMU_IOMODULE, PIT2_CONTROL, EN) );

}

}