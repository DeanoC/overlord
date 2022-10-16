#pragma once

namespace Atomic {
template<typename type>
ALWAYS_INLINE NON_NULL(1) type Load(const type* ptr)  { type val; __atomic_load(ptr, &val, __ATOMIC_SEQ_CST); return val; }
template<typename type>
ALWAYS_INLINE NON_NULL(1) void Store(type* ptr, const type val) { __atomic_store(ptr, (type*) &val, __ATOMIC_SEQ_CST); }
template<typename type>
ALWAYS_INLINE NON_NULL(1, 2) void StorePtr(type* ptr, const type* val) { __atomic_store(ptr, (type*) val, __ATOMIC_SEQ_CST); }
template<typename type>
ALWAYS_INLINE NON_NULL(1, 2) bool CompareExchange(type* ptr, const type* compare, const type exchange) { return __atomic_compare_exchange_n(ptr, (type*)compare, exchange, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST); }
template<typename type>
ALWAYS_INLINE NON_NULL(1, 2) bool CompareExchangePtr(type* ptr, const type* compare, const type* exchange) { return __atomic_compare_exchange(ptr, (type*)compare, (type*)exchange, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST); }
template<typename type>
ALWAYS_INLINE NON_NULL(1) type Swap(type* ptr, const type swap) { return __atomic_exchange_n(ptr, swap, __ATOMIC_SEQ_CST); }
template<typename type>
ALWAYS_INLINE NON_NULL(1, 2) type SwapPtr(type* ptr, const type* swap) { return __atomic_exchange_n(ptr, (type*)swap, __ATOMIC_SEQ_CST); }
template<typename type>
ALWAYS_INLINE NON_NULL(1) type Add(type* ptr, const type value) { return __atomic_fetch_add(ptr, value, __ATOMIC_SEQ_CST); }
template<typename type>
ALWAYS_INLINE NON_NULL(1) type Sub(type* ptr, const type value) { return __atomic_fetch_sub(ptr, value, __ATOMIC_SEQ_CST); }
template<typename type>
ALWAYS_INLINE NON_NULL(1) type And(type* ptr, const type value) { return __atomic_fetch_and(ptr, value, __ATOMIC_SEQ_CST); }
template<typename type>
ALWAYS_INLINE NON_NULL(1) type Xor(type* ptr, const type value) { return __atomic_fetch_xor(ptr, value, __ATOMIC_SEQ_CST); }
template<typename type>
ALWAYS_INLINE NON_NULL(1) type Or(type* ptr, const type value) { return __atomic_fetch_or(ptr, value, __ATOMIC_SEQ_CST); }
template<typename type>
ALWAYS_INLINE NON_NULL(1) type Nand(type* ptr, const type value) { return __atomic_fetch_nand(ptr, value, __ATOMIC_SEQ_CST); }

ALWAYS_INLINE void Memory_Barrier() { __atomic_thread_fence(__ATOMIC_SEQ_CST); }
ALWAYS_INLINE void Compiler_Barrier() { __atomic_signal_fence(__ATOMIC_SEQ_CST); }

}