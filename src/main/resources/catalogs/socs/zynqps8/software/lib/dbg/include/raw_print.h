#pragma once

#include "core/core.h"

#ifdef __cplusplus
extern "C"
{
#endif

void raw_debug_print(char const * text) NON_NULL(1);
void raw_debug_printf(const char *format, ...) NON_NULL(1) __attribute__((format(printf, 1, 2)));

#ifdef __cplusplus
}
#endif
