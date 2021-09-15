#pragma once

#include "internal_atomics.h"

ATOMIC_FM_CREATE_UNSIGNED(U32, uint32_t)
ATOMIC_FM_CREATE_UNSIGNED(U64, uint64_t)

#undef ATOMIC_FM_CREATE_UNSIGNED
