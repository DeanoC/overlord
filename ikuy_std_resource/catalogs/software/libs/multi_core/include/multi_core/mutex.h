#pragma once

#include "multi_core/atomics.h"
#include "dbg/assert.h"
#include "platform/cpu.h"

#define CORE_MUTEX_UNLOCKED 0U
#define CORE_MUTEX_LOCKED 1U

typedef uint32_t Core_Mutex;

ALWAYS_INLINE NON_NULL(1) bool MultiCore_TryLockMutex(Core_Mutex * mutex) {
	uint32_t unlocked = CORE_MUTEX_UNLOCKED;
	bool result = Atomic_CompareExchange_U32(mutex, &unlocked, CORE_MUTEX_LOCKED);
	Atomic_Memory_Barrier();
	return result;
}

ALWAYS_INLINE NON_NULL(1) void MultiCore_LockMutex(Core_Mutex * mutex) {
	bool result;
	do {
		result = MultiCore_TryLockMutex(mutex);
	}
	while(!result);
}

ALWAYS_INLINE NON_NULL(1) void MultiCore_UnlockMutex(Core_Mutex * mutex) {
	Atomic_Memory_Barrier();
	Atomic_Store_U32(mutex, CORE_MUTEX_UNLOCKED);
}

ALWAYS_INLINE NON_NULL(1) bool MultiCore_TryLockRecursiveMutex(Core_Mutex * mutex) {
	uint16_t const cpuHart = GetCpuHartNumber()+1;
	uint32_t value = *mutex;
	if((value >> 16) == cpuHart) {
		// as this mutex is on this thread, we don't need to do any atomic ops
		*mutex = ((value & 0x0000FFFF)+1) | (value & 0xFFFF0000);
		return true;
	} else {
		// not owned by us, lets try and grab it atomically
		uint32_t unlocked = CORE_MUTEX_UNLOCKED;
		uint32_t const initial = (cpuHart << 16) | 1;
		bool result = Atomic_CompareExchange_U32(mutex, &unlocked, initial);
		Atomic_Memory_Barrier();
		return result;
	}
}

ALWAYS_INLINE NON_NULL(1) void MultiCore_LockRecursiveMutex(Core_Mutex * mutex) {
	bool result;
	do {
		result = MultiCore_TryLockRecursiveMutex(mutex);
	}
	while(!result);
}

ALWAYS_INLINE NON_NULL(1) void MultiCore_UnlockRecursiveMutex(Core_Mutex * mutex) {
	uint16_t const cpuHart = GetCpuHartNumber()+1;
	uint32_t value = *mutex;
	// we should only be unlocking mutex we own!
	assert((value >> 16) == cpuHart);
	if((value & 0x0000FFFF) == 1) {
		Atomic_Memory_Barrier();
		Atomic_Store_U32(mutex, CORE_MUTEX_UNLOCKED);
	} else {
		// as this mutex is on this thread, we don't need to do any atomic ops
		*mutex = ((value & 0x0000FFFF) - 1) | (value & 0xFFFF0000);
	}
}