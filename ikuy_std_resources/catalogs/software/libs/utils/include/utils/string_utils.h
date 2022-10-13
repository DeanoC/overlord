#pragma once
#include "core/core.h"
#include "core/snprintf.h"

#ifdef __cplusplus
EXTERN_C {
#endif

WARN_UNUSED_RESULT int32_t Utils_DecimalStringToI32(unsigned int length, const char* str) NON_NULL(2);
WARN_UNUSED_RESULT int64_t Utils_DecimalStringToI64(unsigned int length, const char* str) NON_NULL(2);

// 0x (Hex), 0b (biinary), 0 (octal) or decimal prefixed string to uintXX_t
// _ or ' can be used as spacers
WARN_UNUSED_RESULT uint32_t Utils_BinaryStringToU32(unsigned int length, const char * str) NON_NULL(2);
WARN_UNUSED_RESULT uint32_t Utils_OctalStringToU32(unsigned int length, const char * str) NON_NULL(2);
WARN_UNUSED_RESULT uint32_t Utils_HexStringToU32(unsigned int length, const char * str) NON_NULL(2);
WARN_UNUSED_RESULT uint32_t Utils_DecimalStringToU32(unsigned int length, const char* str) NON_NULL(2);
WARN_UNUSED_RESULT uint32_t Utils_StringToU32(unsigned int length, const char* str) NON_NULL(2);
WARN_UNUSED_RESULT uint64_t Utils_BinaryStringToU64(unsigned int length, const char * str) NON_NULL(2);
WARN_UNUSED_RESULT uint64_t Utils_OctalStringToU64(unsigned int length, const char * str) NON_NULL(2);
WARN_UNUSED_RESULT uint64_t Utils_HexStringToU64(unsigned int length, const char * str) NON_NULL(2);
WARN_UNUSED_RESULT uint64_t Utils_DecimalStringToU64(unsigned int length, const char* str) NON_NULL(2);
WARN_UNUSED_RESULT uint64_t Utils_StringToU64(unsigned int length, const char* str) NON_NULL(2);

WARN_UNUSED_RESULT CONST_EXPR ALWAYS_INLINE NON_NULL(1) unsigned int Utils_StringLength(const char* string) {
	const char* ptr = string;
	while(*ptr) ptr++;
	return ptr - string;
}

CONST_EXPR ALWAYS_INLINE NON_NULL(1,3) unsigned int Utils_StringCopy(const char* src, unsigned int maxLength, char* dst) {
	unsigned int count = 0;
	while(*src != 0 && count < maxLength-1) {
		*dst = *src;
		src++; dst++; count++;
	}
	*dst = 0;
	return count + 1;
}

WARN_UNUSED_RESULT CONST_EXPR ALWAYS_INLINE NON_NULL(1) char const * Utils_StringChar(char const * string, char c) {
	char const * ptr = string;
	while(*ptr) {
		if(*ptr == c) { return ptr; }
		ptr++;
	}
	return nullptr;
}

static const unsigned int Utils_StringNotFound = ~0;

WARN_UNUSED_RESULT NON_NULL(2) unsigned int Utils_StringFindNext(unsigned int strLen, const char* string, char c);

WARN_UNUSED_RESULT NON_NULL(2, 5) unsigned int Utils_StringFindMultiple(unsigned int strLen, const char* string, char c, unsigned int maxFinds, unsigned int* outFinds);

NON_NULL(2, 4) void Utils_StringScatterChar(unsigned int strLen, char* string, unsigned int indexCount, const unsigned int* indices, char c);

#ifdef __cplusplus
}
#endif
