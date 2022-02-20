#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifndef __cplusplus
#define nullptr ((void*) 0)
#endif

#define BRANCH_LIKELY(x) __builtin_expect((x),1)
#define BRANCH_UNLIKELY(x) __builtin_expect((x),0)

#define WEAK_LINKAGE __attribute__((weak))
#define WARN_UNUSED_RESULT __attribute__((warn_unused_result))
#define PACKED  __attribute__((__packed__))
#define HIDDEN __attribute__((__visibility__("hidden")))

#define ALIGN(x) __attribute__((aligned(x)))
#define KEEP __attribute((used))
#define ALIAS(x) __attribute((alias(#x)))

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
#define EXTERN_C
#endif
#define ALLOCA(x) __builtin_alloca(x)

typedef float float_t;
typedef double double_t;

typedef uint32_t uintptr_lo_t;
typedef uint64_t uintptr_all_t;

EXTERN_C WARN_UNUSED_RESULT int memcmp ( const void * a, const void * b, size_t num ) NON_NULL(1,2);

EXTERN_C void * memset ( void *destination, int c, size_t num ) NON_NULL(1);
EXTERN_C void * memcpy ( void * destination, const void * source, size_t bytes ) NON_NULL(1, 2);
EXTERN_C void * memmove ( void * destination, const void * source, size_t bytes ) NON_NULL(1,2);

