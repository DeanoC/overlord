#include "core/core.h"
#include "platform/aarch64/intrinsics_gcc.h"

#define CPU_CORTEXA53_0_TIMESTAMP_CLK_FREQ 33333000
#define TIMESTAMP_COUNTS_PER_SECOND     CPU_CORTEXA53_0_TIMESTAMP_CLK_FREQ

void Utils_BusyClkSleep(uint64_t clks) {
	uint64_t tEnd, tCur;

	tCur = read_counter_timer_register();
	tEnd = tCur + clks;
	do
	{
		tCur = read_counter_timer_register();
	} while (tCur < tEnd);
}

void Utils_BusyMilliSleep(uint64_t useconds)
{
	Utils_BusyClkSleep(useconds * ((TIMESTAMP_COUNTS_PER_SECOND + 500)/1000));
}

void Utils_BusyMicroSleep(uint64_t useconds)
{
	Utils_BusyClkSleep(useconds * ((TIMESTAMP_COUNTS_PER_SECOND + 500000)/1000000));
}

void Utils_BusySecondSleep(uint64_t seconds)
{
	Utils_BusyClkSleep(seconds * TIMESTAMP_COUNTS_PER_SECOND);
}