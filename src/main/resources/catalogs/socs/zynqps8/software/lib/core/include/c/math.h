#pragma once
#include "internal_math.h"

/// Current math.h supports function split into 4 groups
/// * Unsigned
/// * Signed
/// * Unsigned integers
/// * Signed integers
/// | Type | Postfix | Group |
/// | int8_t | I8 | Signed |
/// | uint8_t | U8 | Unsigned |
/// | int16_t | I16 | Signed |
/// | uint16_t | UI16 | Unsigned |
/// | int32_t | I32 | Signed |
/// | uint32_t | U32 | Unsigned |
/// | int64_t | I64 | Signed |
/// | uint64_t | U64 | Unsigned |

#define MATH_FM_USE_BUILTIN 1

MATH_FM_CREATE_UNSIGNED_INTEGER(U8, uint8_t)
MATH_FM_CREATE_UNSIGNED_INTEGER(U16, uint16_t)
MATH_FM_CREATE_UNSIGNED_INTEGER(U32, uint32_t)
MATH_FM_CREATE_UNSIGNED_INTEGER(U64, uint64_t)

MATH_FM_CREATE_SIGNED_INTEGER(I8, int8_t)
MATH_FM_CREATE_SIGNED_INTEGER(I16, int16_t)
MATH_FM_CREATE_SIGNED_INTEGER(I32, int32_t)
MATH_FM_CREATE_SIGNED_INTEGER(I64, int64_t)

#undef MATH_FM_USE_BUILTIN
#undef MATH_FM_CREATE_SIGNED_INTEGER
#undef MATH_FM_CREATE_UNSIGNED_INTEGER
#undef MATH_FM_CREATE_UNSIGNED
#undef MATH_FM_CREATE_SIGNED
#undef MATH_FM_CREATE_REAL
#undef MATH_FM_CREATE_UNSIGNED_INTEGER_MAYBE_BUILTIN