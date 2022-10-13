#pragma once

#if CPU_a53 == 1
# include "a53/cpu.h"

#include "aarch64/intrinsics_gcc.h"

ALWAYS_INLINE uint8_t GetCpuHartNumber() {
	return read_MPIDR_EL1_register() & 0xFF;
}

#elif CPU_pmu == 1
# include "pmu/cpu.h"
ALWAYS_INLINE uint8_t GetCpuHartNumber() {
	return 0;
}

#elif CPU_r5f == 1
# include "r5f/cpu.h"
#elif CPU_host == 1
# include "host/cpu.h"
#else
#error CPU not supported
#endif