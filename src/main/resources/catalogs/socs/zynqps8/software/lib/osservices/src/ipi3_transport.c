#include "core/core.h"
#include "hw/memory_map.h"
#include "hw/reg_access.h"
#include "osservices/ipi3_transport.h"

void IPI3_OsService_Submit(const IPI3_Msg *const msg) {
	// is the pmu ipi buffer clear? PMU shares the buffer amongst all 4 IPI channels
	// if the observation bits are clear, then the pmu buffer is ready to use
	while ((HW_REG_GET_BIT(IPI, PMU_0_OBS, CH3) ||
					HW_REG_GET_BIT(IPI, PMU_1_OBS, CH4) ||
					HW_REG_GET_BIT(IPI, PMU_2_OBS, CH5) ||
					HW_REG_GET_BIT(IPI, PMU_3_OBS, CH6))) {
		// stall till buffer is free to use
	}

	memcpy(IPI_MSG_BEGIN(IA_PMU), msg, 32);
	HW_REG_SET_BIT(IPI, PMU_3_TRIG, CH6);
}

void IPI3_OnService_FetchResponse(IPI3_Response * reply) {
	while ((HW_REG_GET_BIT(IPI, PMU_0_OBS, CH3) ||
					HW_REG_GET_BIT(IPI, PMU_1_OBS, CH4) ||
					HW_REG_GET_BIT(IPI, PMU_2_OBS, CH5) ||
					HW_REG_GET_BIT(IPI, PMU_3_OBS, CH6))) {
		// stall till buffer is free to use
	}
#if CPU_pmu == 1
	memcpy(reply, IPI_RESPONSE_BEGIN(IA_PMU), 32);
#elif CPU_a53 == 1
	memcpy(reply, IPI_RESPONSE_BEGIN(IA_APU), 32);
#elif CPU_r5f == 1
#error TODO R5F needs to check which core its on
#else
#error Unknown CPU
#endif

	*IPI_MSG_BEGIN(IA_PMU) = OSF_RESPONSE_BUFFER_FREE;
	HW_REG_SET_BIT(IPI, PMU_3_TRIG, CH6);
}

