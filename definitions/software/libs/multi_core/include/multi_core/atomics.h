#pragma once

#include "internal_atomics.h"

ATOMIC_FM_CREATE_UNSIGNED(U32, uint32_t)
ATOMIC_FM_CREATE_UNSIGNED(U64, uint64_t)
ATOMIC_FM_CREATE_UNSIGNED(Ptr, void*)

ALWAYS_INLINE void Atomic_Memory_Barrier() { __atomic_thread_fence(__ATOMIC_SEQ_CST); }
ALWAYS_INLINE void Atomic_Compiler_Barrier() { __atomic_signal_fence(__ATOMIC_SEQ_CST); }

#undef ATOMIC_FM_CREATE_UNSIGNED