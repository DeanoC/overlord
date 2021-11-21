#include "core/core.h"
#include "utils/string_utils.h"
//#include "dbg/assert.h"

int32_t Utils_DecimalStringToI32(unsigned int length, const char * str)
{
	const char* start = str;
	bool neg = false;
	int32_t val = 0;

	switch(*str) {
		case '-':
			neg = true;
			str++;
			break;
		case '+':
			str++;
	}

	while((str-start) < length && *str >= '0' && *str <= '9') {
		val = (10 * val) + (*str++ - '0');
	}

	return (neg ? -val : val);
}

NON_NULL(2) uint32_t Utils_BinaryStringToU32(unsigned int length, const char * str)
{
	const char* start = str;
	uint64_t val = 0;

	while((str-start) < length) {
		uint8_t c = *str++;
		if(c == '_' || c =='\'') continue;

		uint8_t n = c - '0';
		if(n > 1) return val;
		val = (2 * val) + n;
	}

	return val;
}

NON_NULL(2) uint32_t Utils_OctalStringToU32(unsigned int length, const char * str)
{
	const char* start = str;
	uint32_t val = 0;

	while((str-start) < length) {
		uint8_t c = *str++;
		if(c == '_' || c =='\'') continue;

		uint8_t n = c - '0';
		if(n > 7) return val;
		val = (8 * val) + n;
	}

	return val;
}


NON_NULL(2) uint32_t Utils_DecimalStringToU32(unsigned int length, const char * str)
{
	const char* start = str;
	uint32_t val = 0;

	while((str-start) < length) {
		uint8_t c = *str++;
		if(c == '_' || c =='\'') continue;

		uint8_t n = c - '0';
		if(n > 9) return val;
		val = (10 * val) + n;
	}

	return val;
}

NON_NULL(2) uint32_t Utils_HexStringToU32(unsigned int length, const char * str)
{
	const char* start = str;
	uint32_t val = 0;

	while((str-start) < length) {
		uint8_t c = *str++;
		if(c == '_' || c =='\'') continue;

		uint8_t n = c - '0';
		if (n > 9) {
			n -= ('a' - ':');
		}
		if(n > 15) return val;
		val = (16 * val) + n;
	}

	return val;
}

NON_NULL(2) uint32_t Utils_StringToU32(unsigned int length, const char * str)
{
	const char* start = str;
	if(*str++ == '0') {
		// octal, hex or binary
		switch(*str) {
			case 'x':
				return Utils_HexStringToU32(length-2, str+1);
			case 'b':
				return Utils_BinaryStringToU32(length-2, str+1);
			default:
				return Utils_OctalStringToU32(length-1, str);
		}
	} else {
		//decimal
		return Utils_DecimalStringToU32(length, start);
	}
}


int64_t Utils_DecimalStringToI64(unsigned int length, const char * str)
{
	const char* start = str;
	bool neg = false;
	int64_t val = 0;

	switch(*str) {
		case '-':
			neg = true;
			str++;
			break;
		case '+':
			str++;
	}

	while((str-start) < length && *str >= '0' && *str <= '9') {
		val = (10 * val) + (*str++ - '0');
	}

	return (neg ? -val : val);
}

NON_NULL(2) uint64_t Utils_BinaryStringToU64(unsigned int length, const char * str)
{
	const char* start = str;
	uint64_t val = 0;

	while((str-start) < length) {
		uint8_t c = *str++;
		if(c == '_' || c =='\'') continue;

		uint8_t n = c - '0';
		if(n > 1) return val;
		val = (2 * val) + n;
	}

	return val;
}

NON_NULL(2) uint64_t Utils_OctalStringToU64(unsigned int length, const char * str)
{
	const char* start = str;
	uint64_t val = 0;

	while((str-start) < length) {
		uint8_t c = *str++;
		if(c == '_' || c =='\'') continue;

		uint8_t n = c - '0';
		if(n > 7) return val;
		val = (8 * val) + n;
	}

	return val;
}


NON_NULL(2) uint64_t Utils_DecimalStringToU64(unsigned int length, const char * str)
{
	const char* start = str;
	uint64_t val = 0;

	while((str-start) < length) {
		uint8_t c = *str++;
		if(c == '_' || c =='\'') continue;

		uint8_t n = c - '0';
		if(n > 9) return val;
		val = (10 * val) + n;
	}

	return val;
}

NON_NULL(2) uint64_t Utils_HexStringToU64(unsigned int length, const char * str)
{
	const char* start = str;
	uint64_t val = 0;

	while((str-start) < length) {
		uint8_t c = *str++;
		if(c == '_' || c =='\'') continue;

		uint8_t n = c - '0';
		if (n > 9) {
			n -= ('a' - ':');
		}
		if(n > 15) return val;
		val = (16 * val) + n;
	}

	return val;
}

NON_NULL(2) uint64_t Utils_StringToU64(unsigned int length, const char * str)
{
	const char* start = str;
	if(*str++ == '0') {
		// octal, hex or binary
		switch(*str) {
			case 'x':
				return Utils_HexStringToU64(length-2, str+1);
			case 'b':
				return Utils_BinaryStringToU64(length-2, str+1);
			default:
				return Utils_OctalStringToU64(length-1, str);
		}
	} else {
		//decimal
		return Utils_DecimalStringToU64(length, start);
	}
}


NON_NULL(2) unsigned int Utils_StringFindNext(unsigned int strLen, const char* string, char c) {
	const char* ptr = string;
	while((ptr - string) < (int)strLen) {
		if(*ptr == c) return ptr - string;
		ptr++;
	}
	return Utils_StringNotFound;
}

NON_NULL(2, 5) unsigned int Utils_StringFindMultiple(unsigned int strLen, const char* string, char c, unsigned int maxFinds, unsigned int* outFinds) {
	const char* ptr = string;
	unsigned int findCount = 0;
	while((ptr - string) < (int)strLen) {
		if(*ptr == c) {
			outFinds[findCount++] = ptr - string;
			if(findCount >= maxFinds) return findCount;
		}
		ptr++;
	}
	if(findCount == 0) return Utils_StringNotFound;
	else {
		outFinds[findCount++] = strLen;
		return findCount;
	}
}

NON_NULL(2, 4) void Utils_StringScatterChar(unsigned int strLen, char* string, unsigned int indexCount, const unsigned int* indices, const char c) {
	for (int i = 0; i < indexCount; ++i) {
	//	assert(indices[i] < strLen);
		string[indices[i]] = c;
	}
}