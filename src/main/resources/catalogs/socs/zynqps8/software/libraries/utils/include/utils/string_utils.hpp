#pragma once
#include "core/core.h"
#include "utils/string_utils.h"

namespace Utils {

WARN_UNUSED_RESULT CONST_EXPR ALWAYS_INLINE NON_NULL(1) unsigned int StringLength(const char* string) {
	return Utils_StringLength(string);
}

CONST_EXPR ALWAYS_INLINE NON_NULL(1,3) unsigned int StringCopy(const char* src, unsigned int maxLength, char* dst) {
	return Utils_StringCopy(src, maxLength, dst);
}

static const unsigned int StringNotFound = Utils_StringNotFound;

WARN_UNUSED_RESULT ALWAYS_INLINE NON_NULL(2) unsigned int StringFindNext(unsigned int strLen, const char* string, char c) {
	return Utils_StringFindNext(strLen, string, c);
}

WARN_UNUSED_RESULT ALWAYS_INLINE NON_NULL(2, 5) unsigned int StringFindMultiple(unsigned int strLen, const char* string, char c, unsigned int maxFinds, unsigned int* outFinds) {
	return Utils_StringFindMultiple(strLen, string, c, maxFinds, outFinds);
}
WARN_UNUSED_RESULT ALWAYS_INLINE NON_NULL(2) int32_t DecimalStringToI32(unsigned int length, const char* str) {
	return Utils_DecimalStringToI32(length,str);
}
WARN_UNUSED_RESULT ALWAYS_INLINE NON_NULL(2) int64_t DecimalStringToI64(unsigned int length, const char* str) {
	return Utils_DecimalStringToI64(length,str);
}

WARN_UNUSED_RESULT ALWAYS_INLINE NON_NULL(2) uint32_t BinaryStringToU32(unsigned int length, const char* str) {
	return Utils_BinaryStringToU32(length,str);
}
WARN_UNUSED_RESULT ALWAYS_INLINE NON_NULL(2) uint32_t OctalStringToU32(unsigned int length, const char* str) {
	return Utils_OctalStringToU32(length,str);
}
WARN_UNUSED_RESULT ALWAYS_INLINE NON_NULL(2) uint32_t DecimalStringToU32(unsigned int length, const char* str) {
	return Utils_DecimalStringToU32(length,str);
}
WARN_UNUSED_RESULT ALWAYS_INLINE NON_NULL(2) uint32_t HexStringToU32(unsigned int length, const char* str) {
	return Utils_HexStringToU32(length,str);
}
WARN_UNUSED_RESULT ALWAYS_INLINE NON_NULL(2) uint32_t StringToU32(unsigned int length, const char* str) {
	return Utils_StringToU32(length,str);
}

WARN_UNUSED_RESULT ALWAYS_INLINE NON_NULL(2) uint64_t BinaryStringToU64(unsigned int length, const char* str) {
	return Utils_BinaryStringToU64(length,str);
}
WARN_UNUSED_RESULT ALWAYS_INLINE NON_NULL(2) uint64_t OctalStringToU64(unsigned int length, const char* str) {
	return Utils_OctalStringToU64(length,str);
}
WARN_UNUSED_RESULT ALWAYS_INLINE NON_NULL(2) uint64_t DecimalStringToU64(unsigned int length, const char* str) {
	return Utils_DecimalStringToU64(length,str);
}
WARN_UNUSED_RESULT ALWAYS_INLINE NON_NULL(2) uint64_t HexStringToU64(unsigned int length, const char* str) {
	return Utils_HexStringToU64(length,str);
}
WARN_UNUSED_RESULT ALWAYS_INLINE NON_NULL(2) uint64_t StringToU64(unsigned int length, const char* str) {
	return Utils_StringToU64(length,str);
}

ALWAYS_INLINE NON_NULL(2, 4) void StringScatterChar(unsigned int strLen, char* string, unsigned int indexCount, const unsigned int* indices, char c) {
	Utils_StringScatterChar(strLen, string,indexCount, indices, c);
}


} // end namespace