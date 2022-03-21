#pragma once

#include "core/core.h"

#ifdef __cplusplus
EXTERN_C
{
#endif

#ifndef DO_ASSERT
# define DO_ASSERT 1
#endif

#if DO_ASSERT

#if CPU_pmu != 1
NO_RETURN void assert_printf(char const *file, int line, char const* txt);
#else
void assert_printf(char const *file, int line, char const* txt);
#endif

#define assert(test) if (!(test)) { assert_printf(__FILE__, __LINE__, #test); }

#define assert_msg(test, msg) if (!(test)) { assert_printf(__FILE__, __LINE__, msg ": " #test); }

#else

#define assert(test)
#define assert_msg(test, msg)

#endif

#ifdef __cplusplus
}
#endif