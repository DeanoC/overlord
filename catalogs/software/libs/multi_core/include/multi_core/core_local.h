#pragma once

// IKUY doesn't have threads but doesn't support multiple cores (effectively 1 thread per core)
// as such we don't have thread local storage but do support core local storage
// for host platforms this is actually just thread local

#include "platform/cpu.h"

#if !CPU_host

#define CORE_LOCAL(type, name) type name[CPU_CORE_COUNT]
#define READ_CORE_LOCAL(name) name[GetCpuHartNumber()]
#define WRITE_CORE_LOCAL(name, value) name[GetCpuHartNumber()] = (value)

#else
#include "platform/host/platform.h"

#define CORE_LOCAL(type, name) HOST_PLATFORM_THREAD_LOCAL type name
#define READ_CORE_LOCAL(name) name
#define WRITE_CORE_LOCAL(name, value) name = value

#endif