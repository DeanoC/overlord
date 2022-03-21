#pragma once

#include "core/core.h"

#define ATOMIC_FM_CREATE_UNSIGNED(postfix, type) \
ALWAYS_INLINE NON_NULL(1) type Atomic_Load##_##postfix(const type* ptr)  { type val; __atomic_load(ptr, &val, __ATOMIC_SEQ_CST); return val; } \
ALWAYS_INLINE NON_NULL(1) void Atomic_Store##_##postfix(type* ptr, const type val) { __atomic_store(ptr, (type*) &val, __ATOMIC_SEQ_CST); } \
ALWAYS_INLINE NON_NULL(1, 2) void Atomic_StorePtr##_##postfix(type* ptr, const type* val) { __atomic_store(ptr, (type*) val, __ATOMIC_SEQ_CST); } \
ALWAYS_INLINE NON_NULL(1, 2) bool Atomic_CompareExchange##_##postfix(type* ptr, type* compare, const type exchange) { return __atomic_compare_exchange_n(ptr, compare, exchange, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST); } \
ALWAYS_INLINE NON_NULL(1, 2) bool Atomic_CompareExchange_ptr##_##postfix(type* ptr, type* compare, const type* exchange) { return __atomic_compare_exchange(ptr, compare, (type*)exchange, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST); } \
ALWAYS_INLINE NON_NULL(1) type Atomic_Swap##_##postfix(type* ptr, const type swap) { return __atomic_exchange_n(ptr, swap, __ATOMIC_SEQ_CST); } \
ALWAYS_INLINE NON_NULL(1) type Atomic_Add##_##postfix(type* ptr, const type value) { return __atomic_add_fetch(ptr, value, __ATOMIC_SEQ_CST); } \
ALWAYS_INLINE NON_NULL(1) type Atomic_Sub##_##postfix(type* ptr, const type value) { return __atomic_sub_fetch(ptr, value, __ATOMIC_SEQ_CST); } \
ALWAYS_INLINE NON_NULL(1) type Atomic_And##_##postfix(type* ptr, const type value) { return __atomic_and_fetch(ptr, value, __ATOMIC_SEQ_CST); } \
ALWAYS_INLINE NON_NULL(1) type Atomic_Xor##_##postfix(type* ptr, const type value) { return __atomic_xor_fetch(ptr, value, __ATOMIC_SEQ_CST); } \
ALWAYS_INLINE NON_NULL(1) type Atomic_Or##_##postfix(type* ptr, const type value) { return __atomic_or_fetch(ptr, value, __ATOMIC_SEQ_CST); }  \
ALWAYS_INLINE NON_NULL(1) type Atomic_Nand##_##postfix(type* ptr, const type value) { return __atomic_nand_fetch(ptr, value, __ATOMIC_SEQ_CST); }

