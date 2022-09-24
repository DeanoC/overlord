//
// Created by deano on 8/21/22.
//
#include "core/core.h"
#include "platform/reg_access.h"
#include "platform/memory_map.h"
#include "platform/registers/pmu_global.h"
#include "platform/registers/ipi.h"
#include "pmu.hpp"

void PmuSleep() {
	// Enable PMU_0 IPI
	HW_REG_SET_BIT1( IPI, PMU_0_IER, CH3 );

	// Trigger PMU0 IPI in PMU IPI TRIG Reg
	HW_REG_SET_BIT1( IPI, PMU_0_TRIG, CH3 );

	// Wait until PMU Microblaze goes to sleep state,
	// before starting firmware download to PMU RAM
	while(!HW_REG_GET_BIT1( PMU_GLOBAL, GLOBAL_CNTRL, MB_SLEEP )) {
		// stall
	}

	HW_REG_CLR_BIT1( PMU_GLOBAL, GLOBAL_CNTRL, FW_IS_PRESENT );
}

void PmuWakeup() {

	HW_REG_SET_BIT1( PMU_GLOBAL, GLOBAL_CNTRL, DONT_SLEEP );

	// pmu firmware set the FW_IS_PRESENT flag once it running
	while(HW_REG_GET_BIT1( PMU_GLOBAL, GLOBAL_CNTRL, FW_IS_PRESENT ) == 0) {
		// stall
	}
}

// pmu ram only accepts 32 bit access, this ensure thats true
void PmuSafeMemcpy( void *destination, const void *source, size_t num_in_bytes ) {
	uint32_t *dst = (uint32_t *) destination;
	const uint32_t *src = (uint32_t *) source;

	// copy in 32 bit words chunks into PMU_RAM
	for(uint32_t copy_count = 0; copy_count < (num_in_bytes + 3) / 4; copy_count++) {
		*(dst + copy_count) = *(src + copy_count);
	}
}
