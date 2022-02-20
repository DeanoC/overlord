#pragma once


// IKUY doesn't have threads but doesn't support multiple cores (effectively 1 thread per core)
// as such we don't have thread local storage but do support core local storage
// for host platforms this is actually just thread local

#if !defined(CPU_host)

#include "hw/cpu.h"

#define CORE_LOCAL(type, name) type name[CPU_CORE_COUNT]
#define READ_CORE_LOCAL(name) name[GetCpuHartNumber()]
#define WRITE_CORE_LOCAL(name, value) name[GetCpuHartNumber()] = (value)

#else

#define CORE_LOCAL(type, name) thread_local type name
#define READ_CORE_LOCAL(name) name
#define WRITE_CORE_LOCAL(name, value) name = value

#endif