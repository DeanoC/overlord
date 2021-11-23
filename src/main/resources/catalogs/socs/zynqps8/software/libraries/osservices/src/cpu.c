#include "core/core.h"
#include "osservices/ipi3_transport.h"

void OsService_SleepCpus(uint8_t cpus) {
	IPI3_Msg msg = {
			.function = OSF_CPU_WAKE_OR_SLEEP,
			.Payload.CPUWakeOrSleep.wake_a53_0 = 0,
			.Payload.CPUWakeOrSleep.wake_a53_1 = 0,
			.Payload.CPUWakeOrSleep.wake_a53_2 = 0,
			.Payload.CPUWakeOrSleep.wake_a53_3 = 0,
			.Payload.CPUWakeOrSleep.wake_r5f_0 = 0,
			.Payload.CPUWakeOrSleep.wake_r5f_1 = 0,
			.Payload.CPUWakeOrSleep.sleep_a53_0 = cpus & OSSC_A53_0,
			.Payload.CPUWakeOrSleep.sleep_a53_1 = cpus & OSSC_A53_1,
			.Payload.CPUWakeOrSleep.sleep_a53_2 = cpus & OSSC_A53_2,
			.Payload.CPUWakeOrSleep.sleep_a53_3 = cpus & OSSC_A53_3,
			.Payload.CPUWakeOrSleep.sleep_r5f_0 = cpus & OSSC_R5F_0,
			.Payload.CPUWakeOrSleep.sleep_r5f_1 = cpus & OSSC_R5F_1,
	};
	IPI3_OsService_Submit(&msg);
}

void OsService_WakeCpu(uint8_t cpus) {
	IPI3_Msg msg = {
			.function = OSF_CPU_WAKE_OR_SLEEP,
			.Payload.CPUWakeOrSleep.wake_a53_0 = cpus & OSSC_A53_0,
			.Payload.CPUWakeOrSleep.wake_a53_1 = cpus & OSSC_A53_1,
			.Payload.CPUWakeOrSleep.wake_a53_2 = cpus & OSSC_A53_2,
			.Payload.CPUWakeOrSleep.wake_a53_3 = cpus & OSSC_A53_3,
			.Payload.CPUWakeOrSleep.wake_r5f_0 = cpus & OSSC_R5F_0,
			.Payload.CPUWakeOrSleep.wake_r5f_1 = cpus & OSSC_R5F_1,
			.Payload.CPUWakeOrSleep.sleep_a53_0 = 0,
			.Payload.CPUWakeOrSleep.sleep_a53_1 = 0,
			.Payload.CPUWakeOrSleep.sleep_a53_2 = 0,
			.Payload.CPUWakeOrSleep.sleep_a53_3 = 0,
			.Payload.CPUWakeOrSleep.sleep_r5f_0 = 0,
			.Payload.CPUWakeOrSleep.sleep_r5f_1 = 0,
	};
	IPI3_OsService_Submit(&msg);
}
