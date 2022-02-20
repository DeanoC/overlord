#pragma once

#include "multi_core/atomics.h"

#define CORE_MUTEX_UNLOCKED 0U
#define CORE_MUTEX_LOCKED 1U

typedef uint32_t Core_Mutex;

ALWAYS_INLINE NON_NULL(1) void MultiCore_LockMutex(Core_Mutex * mutex) {
	uint32_t unlocked = CORE_MUTEX_UNLOCKED;
	uint32_t locked = CORE_MUTEX_LOCKED;
	bool result;
	do {
		result = Atomic_CompareExchange_ptr_U32(mutex, &unlocked, &locked);
	}
	while(!result);

	Atomic_Memory_Barrier();
}

ALWAYS_INLINE NON_NULL(1) void MultiCore_UnlockMutex(Core_Mutex * mutex) {
	Atomic_Memory_Barrier();
	Atomic_Store_U32(mutex, CORE_MUTEX_UNLOCKED);
}

ALWAYS_INLINE NON_NULL(1) bool MultiCore_TryLockMutex(Core_Mutex * mutex) {
	uint32_t unlocked = CORE_MUTEX_UNLOCKED;
	bool result = Atomic_CompareExchange_U32(mutex, &unlocked, CORE_MUTEX_LOCKED);
	Atomic_Memory_Barrier();
	return result;
}
