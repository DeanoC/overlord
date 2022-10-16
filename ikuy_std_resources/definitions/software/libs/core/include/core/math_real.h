#pragma once

#include "core/core.h"
#if CPU_pmu == 1
#error PMU does not have a FPU
#endif

#include "internal_math.h"

/// Current math_real.h supports function split into 1 groups
/// * Real
/// | Type 		| Postfix | Group |
/// | float 	| F 			| Real |
/// | double 	| D 			| Real |
MATH_FM_CREATE_REAL(F, float)
MATH_FM_CREATE_REAL(D, double)

#undef MATH_FM_CREATE_SIGNED_INTEGER
#undef MATH_FM_CREATE_UNSIGNED_INTEGER
#undef MATH_FM_CREATE_UNSIGNED
#undef MATH_FM_CREATE_SIGNED
#undef MATH_FM_CREATE_REAL
#undef MATH_FM_CREATE_UNSIGNED_INTEGER_MAYBE_BUILTIN