#include "core/core.h"
#include "dbg/assert.h"
#include "stb_dict.h"
#include "cadt/dict.h"

#define CDICT_IMPL(postfix, type) \
EXTERN_C CADT_Dict##postfix##Handle CADT_Dict##postfix##Create(Memory_Allocator* allocator) { \
	return (CADT_Dict##postfix##Handle) stb_dict##postfix##_create(allocator); \
} \
EXTERN_C CADT_Dict##postfix##Handle CADT_Dict##postfix##CreateAndReserve(Memory_Allocator* allocator, size_t count) { \
	return (CADT_Dict##postfix##Handle) stb_dict##postfix##_create_with_reserve(allocator, count); \
} \
EXTERN_C void CADT_Dict##postfix##Destroy(CADT_Dict##postfix##Handle handle) { \
	assert(handle); \
	stb_dict##postfix * dict = (stb_dict##postfix *) handle; \
	stb_dict##postfix##_destroy(dict); \
} \
EXTERN_C CADT_Dict##postfix##Handle CADT_Dict##postfix##Clone(CADT_Dict##postfix##Handle handle) {\
	assert(handle); \
	stb_dict##postfix * dict = (stb_dict##postfix *) handle; \
	return (CADT_Dict##postfix##Handle)stb_dict##postfix##_copy(dict); \
} \
EXTERN_C size_t CADT_Dict##postfix##Size(CADT_Dict##postfix##Handle handle) { \
	assert(handle); \
	stb_dict##postfix const * dict = (stb_dict##postfix const *) handle; \
	return dict->count; \
} \
EXTERN_C bool CADT_Dict##postfix##IsEmpty(CADT_Dict##postfix##Handle handle) { \
	return CADT_Dict##postfix##Size(handle) == 0; \
} \
EXTERN_C size_t CADT_Dict##postfix##Capacity(CADT_Dict##postfix##Handle handle) {\
	assert(handle); \
	stb_dict##postfix const * dict = (stb_dict##postfix const *) handle; \
	return dict->limit; \
} \
EXTERN_C type CADT_Dict##postfix##GetByIndex(CADT_Dict##postfix##Handle handle, size_t index) {\
	assert(handle); \
	stb_dict##postfix const * dict = (stb_dict##postfix const *) handle; \
	size_t count = 0; \
	for (size_t i = 0; i < dict->limit; ++i) { \
		if (dict->table[i].k != ~0) { \
			if (count == index) \
				return dict->table[i].v; \
			else \
				count++; \
		}	\
	} \
	return 0; \
} \
EXTERN_C void CADT_Dict##postfix##Reserve(CADT_Dict##postfix##Handle handle, size_t const size) { \
	assert(handle); \
	stb_dict##postfix * dict = (stb_dict##postfix *) handle; \
	stb_dict##postfix##_reserve(dict, (int)size); \
} \
EXTERN_C bool CADT_Dict##postfix##KeyExists(CADT_Dict##postfix##Handle handle, type const key) { \
	assert(handle); \
	stb_dict##postfix const * dict = (stb_dict##postfix const *) handle; \
	type dummy; \
	return stb_dict##postfix##_get_flag(dict, key, &dummy); \
} \
EXTERN_C bool CADT_Dict##postfix##Lookup(CADT_Dict##postfix##Handle handle, type const key, type* out) { \
	assert(handle); \
	stb_dict##postfix const * dict = (stb_dict##postfix const *) handle; \
	return stb_dict##postfix##_get_flag(dict, key, out); \
} \
EXTERN_C type CADT_Dict##postfix##Get(CADT_Dict##postfix##Handle handle, type const key) { \
	assert(handle); \
	stb_dict##postfix const * dict = (stb_dict##postfix const *) handle; \
	type out = 0; \
	stb_dict##postfix##_get_flag(dict, key, &out); \
	return out; \
} \
EXTERN_C bool CADT_Dict##postfix##Add(CADT_Dict##postfix##Handle handle, type const key, type const in) { \
	assert(handle); \
	stb_dict##postfix * dict = (stb_dict##postfix *) handle; \
	return stb_dict##postfix##_add(dict, key, in);  \
}\
EXTERN_C void CADT_Dict##postfix##Remove(CADT_Dict##postfix##Handle handle, type const key) { \
	assert(handle); \
	stb_dict##postfix * dict = (stb_dict##postfix *) handle; \
	type dummy; \
	stb_dict##postfix##_remove(dict, key, &dummy); \
} \
EXTERN_C void CADT_Dict##postfix##Replace(CADT_Dict##postfix##Handle handle, type const key, type const in) { \
	assert(handle); \
	stb_dict##postfix * dict = (stb_dict##postfix *) handle; \
	stb_dict##postfix##_update(dict, key, in);  \
}

CDICT_IMPL(U32, uint32_t)
CDICT_IMPL(U64, uint64_t)