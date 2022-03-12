#pragma once

#include "core/core.h"

#ifdef __cplusplus
EXTERN_C
{
#endif

#if CPU_pmu == 1
WEAK_LINKAGE void debug_print(char const * text) NON_NULL(1);
WEAK_LINKAGE void debug_printf(const char *format, ...) NON_NULL(1) __attribute__((format(printf, 1, 2)));
WEAK_LINKAGE void debug_sized_print(uint32_t size, char const * text) NON_NULL(2);
#else
void debug_print(char const * text) NON_NULL(1);
void debug_printf(const char *format, ...) NON_NULL(1) __attribute__((format(printf, 1, 2)));
void debug_sized_print(uint32_t size, char const * text) NON_NULL(2);
#endif

void debug_force_raw_print(bool enabled);

#ifdef __cplusplus
}
#endif

#ifndef DO_TRACE
# define DO_TRACE 0
#endif

#if DO_TRACE
#define trace_msg(msg) debug_printf("TRACE @ %s(%d): - %s\r\n", __FILE__, __LINE__, msg)
#else

#define trace_msg(msg)

#endif
