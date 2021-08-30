#pragma once

#include "core/core.h"

void raw_debug_print(char const * text) NON_NULL(1);
void raw_debug_printf(const char *format, ...) NON_NULL(1) __attribute__((format(printf, 1, 2)));

