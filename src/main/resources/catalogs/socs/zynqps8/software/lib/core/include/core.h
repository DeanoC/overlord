#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define nullptr ((void*) 0)

#define BRANCH_LIKELY(x) __builtin_expect((x),1)
#define BRANCH_UNLIKELY(x) __builtin_expect((x),0)

#define WEAK_LINKAGE __attribute__((weak))
#define WARN_UNUSED_RESULT __attribute__((warn_unused_result))
#define PACKED  __attribute__((__packed__))
#define ALIGN(x) __attribute__((align(x))
#define KEEP __attribute((used))

#define INLINE __attribute__((gnu_inline)) inline
#define ONLY_INLINE __attribute__((gnu_inline)) extern inline
#define ALWAYS_INLINE __attribute__((always_inline)) inline

#define NON_NULL(...) __attribute__((nonnull(__VA_ARGS__)))
#define FORMAT_PRINT(...) __attribute__((format(printf, __VA_ARGS__)))

typedef uint32_t uintptr_lo_t;
typedef uint64_t uintptr_all_t;

WARN_UNUSED_RESULT int memcmp ( const void * a, const void * b, size_t num ) NON_NULL(1,2);
WARN_UNUSED_RESULT int32_t atoi( const char* str) NON_NULL(1);

void * memset ( void *destination, int c, size_t num ) NON_NULL(1);
void * memcpy ( void * destination, const void * source, size_t num ) NON_NULL(1, 2);
void * memmove ( void * destination, const void * source, size_t num ) NON_NULL(1,2);
