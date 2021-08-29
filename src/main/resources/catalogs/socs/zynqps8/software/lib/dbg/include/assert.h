#pragma once

#ifndef DO_ASSERT
# define DO_ASSERT 1
#endif

#if DO_ASSERT

void assert_printf(char const *file, int line, char const* txt);

#define assert(test) \
if (!(test))       \
{                  \
assert_printf(__FILE__, __LINE__, #test); \
}

#else

#define assert(test)

#endif