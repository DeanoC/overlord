#pragma once

#ifdef __cplusplus
extern "C"
{
#endif
#include "core/core.h"
#include "internal_bitops.h"

BITOP_FM_CREATE_UNSIGNED(U8, uint8_t)
BITOP_FM_CREATE_UNSIGNED(U16, uint16_t)
BITOP_FM_CREATE_UNSIGNED(U32, uint32_t)
BITOP_FM_CREATE_UNSIGNED(U64, uint64_t)

#define BITOP_POSTFIXISE(macro, x) macro##_##U##x
#define APPEND_BITWIDTH_POSTFIX(macro, width) BITOP_POSTFIXISE(macro, width)


#ifdef __cplusplus
}
#endif

#undef BITOP_FM_CREATE_UNSIGNED