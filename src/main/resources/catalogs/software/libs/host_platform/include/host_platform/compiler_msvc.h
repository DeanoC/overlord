#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <memory.h>

#undef ALWAYS_INLINE
#define ALWAYS_INLINE __forceinline

#define HOST_PLATFORM_THREAD_LOCAL __declspec(thread)

//#define AL2O3_RESTRICT __restrict
//#define AL2O3_DEBUG_BREAK() __debugbreak()