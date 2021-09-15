#pragma once

#if CPU_a53 == 1
# include "a53/cpu.h"
#elif CPU_pmu == 1
# include "pmu/cpu.h"
#elif CPU_r5f == 1
# include "r5f/cpu.h"
#else
#error CPU not supported
#endif
