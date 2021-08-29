#pragma once

#if CPU_a53 == 1
# include "a53/memory_map.h"
#elif CPU_pmu == 1
# include "pmu/memory_map.h"
#elif CPU_r5f == 1
# include "r5f/memory_map.h"
#else
#error CPU not supported
#endif
