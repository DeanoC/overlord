#pragma once
#include "core/core.h"

#define LACKS_SYS_TYPES_H 1
#define LACKS_UNISTD_H 1
#define LACKS_FCNTL_H 1
#define LACKS_SYS_PARAM_H 1
#define LACKS_SYS_MMAN_H 1
#define LACKS_STRINGS_H 1
#define LACKS_STRING_H  1
#define LACKS_SYS_TYPES_H 1
#define LACKS_ERRNO_H   1
#define LACKS_STDLIB_H  1
#define LACKS_SCHED_H   1
#define LACKS_TIME_H 1
#define ABORT
#define MALLOC_FAILURE_ACTION
#define malloc_getpagesize (64*1024)
#define PROT_READ 1
#define PROT_WRITE 2
#define MAP_PRIVATE 1
#define MAP_ANONYMOUS 2
#define EINVAL 1
#define ENOMEM 2
EXTERN_C void *FAKE_mmap(void *addr, size_t length, int prot, int flags, int fd, int64_t offset);
EXTERN_C int FAKE_munmap(void *addr, size_t length);
#define mmap FAKE_mmap
#define munmap FAKE_munmap
