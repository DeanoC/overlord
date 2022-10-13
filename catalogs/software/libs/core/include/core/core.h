#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifndef __cplusplus
#define nullptr ((void*) 0)
#endif

#define BRANCH_LIKELY(x) __builtin_expect((x),1)
#define BRANCH_UNLIKELY(x) __builtin_expect((x),0)

#define RESTRICT __restrict

#define WEAK_LINKAGE __attribute__((weak))
#define WARN_UNUSED_RESULT __attribute__((warn_unused_result))
#define PACKED  __attribute__((__packed__))
#define HIDDEN __attribute__((__visibility__("hidden")))

#define ALIGN(x) __attribute__((aligned(x)))
#define KEEP __attribute__((used))
#define ALIAS(x) __attribute__((alias(#x)))

#define INLINE __attribute__((gnu_inline)) inline
#define ONLY_INLINE __attribute__((gnu_inline)) extern inline
#define ALWAYS_INLINE __attribute__((always_inline)) inline

#define NON_NULL(...) __attribute__((nonnull(__VA_ARGS__)))
#define FORMAT_PRINT(...) __attribute__((format(printf, __VA_ARGS__)))

#ifdef __cplusplus
#define CONST_EXPR constexpr
#define NO_RETURN [[noreturn]]
#define EXTERN_C extern "C"

#else
#define CONST_EXPR
#define static_assert(...) _Static_assert(__VA_ARGS__)
#define NO_RETURN _Noreturn
#define EXTERN_C extern
#endif // end __cplusplus

#define STACK_ALLOC(x) __builtin_alloca(x)

typedef float float_t;
typedef double double_t;

typedef ptrdiff_t ssize_t;

typedef uint32_t uintptr_lo_t;
typedef uint64_t uintptr_all_t;

#if CPU_host != 1
EXTERN_C WARN_UNUSED_RESULT int memcmp ( const void * a, const void * b, size_t num ) NON_NULL(1,2);

EXTERN_C void * memset ( void * RESTRICT destination, int c, size_t num ) NON_NULL(1);
EXTERN_C void * memcpy ( void * RESTRICT destination, const void * RESTRICT source, size_t bytes ) NON_NULL(1, 2);
EXTERN_C void * memmove ( void * RESTRICT destination, const void * RESTRICT source, size_t bytes )NON_NULL(1,2);

ALWAYS_INLINE WARN_UNUSED_RESULT long unsigned int strlen(char const * const RESTRICT str) {
	char const * p = str;
	while(*p){ p++; };
	return p - str;
}

WARN_UNUSED_RESULT int strncmp(char const * RESTRICT a, char const * RESTRICT b, size_t bytes);

void srand(unsigned int seed);

int rand(void);
#else
#ifndef __cplusplus
	#include <string.h>
#else
	#include <cstring>
	#include <string>
#endif // end CPU_Host

#endif

#define IKUY_DEBUG_BREAK() __builtin_trap();
