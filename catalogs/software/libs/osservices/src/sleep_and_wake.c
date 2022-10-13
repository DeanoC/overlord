#include "core/core.h"
#include "osservices/ipi3_transport.h"

#if CPU_a53
#include "platform/aarch64/intrinsics_gcc.h"
#include "dbg/print.h"
#endif

void OsService_SleepCpus(uint8_t cpus) {
	IPI3_Msg msg = {
			.function = OSF_CPU_WAKE_OR_SLEEP,
			.Payload.CPUWakeOrSleep.wakeA53_0 = 0,
			.Payload.CPUWakeOrSleep.wakeA53_1 = 0,
			.Payload.CPUWakeOrSleep.wakeA53_2 = 0,
			.Payload.CPUWakeOrSleep.wakeA53_3 = 0,
			.Payload.CPUWakeOrSleep.wakeR5f_0 = 0,
			.Payload.CPUWakeOrSleep.wakeR5f_1 = 0,
			.Payload.CPUWakeOrSleep.sleepA53_0 = (cpus & OSSC_A53_0) != 0,
			.Payload.CPUWakeOrSleep.sleepA53_1 = (cpus & OSSC_A53_1) != 0,
			.Payload.CPUWakeOrSleep.sleepA53_2 = (cpus & OSSC_A53_2) != 0,
			.Payload.CPUWakeOrSleep.sleepA53_3 = (cpus & OSSC_A53_3) != 0,
			.Payload.CPUWakeOrSleep.sleepR5f_0 = (cpus & OSSC_R5F_0) != 0,
			.Payload.CPUWakeOrSleep.sleepR5f_1 = (cpus & OSSC_R5F_1) != 0,
	};
	IPI3_OsService_Submit(&msg);
}

void OsService_WakeCpu(uint8_t cpus, uintptr_all_t wakeAddress) {
	IPI3_Msg msg = {
			.function = OSF_CPU_WAKE_OR_SLEEP,
			.Payload.CPUWakeOrSleep.wakeA53_0 = (cpus & OSSC_A53_0) != 0,
			.Payload.CPUWakeOrSleep.wakeA53_1 = (cpus & OSSC_A53_1) != 0,
			.Payload.CPUWakeOrSleep.wakeA53_2 = (cpus & OSSC_A53_2) != 0,
			.Payload.CPUWakeOrSleep.wakeA53_3 = (cpus & OSSC_A53_3) != 0,
			.Payload.CPUWakeOrSleep.wakeR5f_0 = (cpus & OSSC_R5F_0) != 0,
			.Payload.CPUWakeOrSleep.wakeR5f_1 = (cpus & OSSC_R5F_1) != 0,
			.Payload.CPUWakeOrSleep.sleepA53_0 = 0,
			.Payload.CPUWakeOrSleep.sleepA53_1 = 0,
			.Payload.CPUWakeOrSleep.sleepA53_2 = 0,
			.Payload.CPUWakeOrSleep.sleepA53_3 = 0,
			.Payload.CPUWakeOrSleep.sleepR5f_0 = 0,
			.Payload.CPUWakeOrSleep.sleepR5f_1 = 0,
			.Payload.CPUWakeOrSleep.wakeAddress = wakeAddress,
	};
	IPI3_OsService_Submit(&msg);
}
void OsService_SleepFPGA() {
	IPI3_Msg msg = {
			.function = OSF_DEVICE_WAKE_OR_SLEEP,
			.Payload.DeviceWakeOrSleep.sleepFPGA = 1,
		 	.Payload.DeviceWakeOrSleep.wakeFPGA = 0,
	};
	IPI3_OsService_Submit(&msg);
}

void OsService_WakeFPGA() {
	IPI3_Msg msg = {
			.function = OSF_DEVICE_WAKE_OR_SLEEP,
			.Payload.DeviceWakeOrSleep.sleepFPGA = 0,
			.Payload.DeviceWakeOrSleep.wakeFPGA = 1,
	};
	IPI3_OsService_Submit(&msg);
}

uint8_t OsService_GetCoreHart() {
	// no need to send to PMU
#if CPU_a53
	return read_MPIDR_EL1_register() & 0xFF;
#elif CPU_pmu
	return 0;
#else
#error CPU not supported for GetCoreId
#endif

}
