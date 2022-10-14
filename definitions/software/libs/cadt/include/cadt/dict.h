#pragma once

#include "core/core.h"
#include "memory/memory.h"

#define CADT_DECLARE_DICT(postfix, keytype, valuetype) \
typedef struct CADT_Dict##postfix *CADT_Dict##postfix##Handle; \
EXTERN_C CADT_Dict##postfix##Handle CADT_Dict##postfix##Create(Memory_Allocator* allocator); \
EXTERN_C CADT_Dict##postfix##Handle CADT_Dict##postfix##CreateAndReserve(Memory_Allocator* allocator, size_t count); \
EXTERN_C void CADT_Dict##postfix##Destroy(CADT_Dict##postfix##Handle handle); \
EXTERN_C CADT_Dict##postfix##Handle CADT_Dict##postfix##Clone(CADT_Dict##postfix##Handle handle); \
EXTERN_C size_t CADT_Dict##postfix##Size(CADT_Dict##postfix##Handle handle); \
EXTERN_C valuetype CADT_Dict##postfix##GetByIndex(CADT_Dict##postfix##Handle handle, size_t index); \
EXTERN_C bool CADT_Dict##postfix##IsEmpty(CADT_Dict##postfix##Handle handle); \
EXTERN_C size_t CADT_Dict##postfix##Capacity(CADT_Dict##postfix##Handle handle); \
EXTERN_C void CADT_Dict##postfix##Reserve(CADT_Dict##postfix##Handle handle, size_t const size); \
EXTERN_C bool CADT_Dict##postfix##KeyExists(CADT_Dict##postfix##Handle handle, keytype const key); \
EXTERN_C valuetype CADT_Dict##postfix##Get(CADT_Dict##postfix##Handle handle, keytype const key); \
EXTERN_C bool CADT_Dict##postfix##Lookup(CADT_Dict##postfix##Handle handle, keytype const key, valuetype* out); \
EXTERN_C bool CADT_Dict##postfix##Add(CADT_Dict##postfix##Handle handle, keytype const key, valuetype const in); \
EXTERN_C void CADT_Dict##postfix##Remove(CADT_Dict##postfix##Handle handle, keytype const key); \
EXTERN_C void CADT_Dict##postfix##Replace(CADT_Dict##postfix##Handle handle, keytype const key, valuetype const in);

CADT_DECLARE_DICT(U32, uint32_t, uint32_t)
CADT_DECLARE_DICT(U64, uint64_t, uint64_t)

#undef CADT_DECLARE_DICT