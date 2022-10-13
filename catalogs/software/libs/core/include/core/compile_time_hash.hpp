#pragma once
#include "core/core.h"

namespace Core
{
	// FNV-1a 32bit hashing algorithm.
	CONST_EXPR uint32_t CompileTimeHash(size_t count, char const* s)
	{
		return ((count > 1 ? CompileTimeHash(count - 1, s) : 2166136261ul) ^ s[count-1]) * 16777619ul;
	}

	// FNV-1a 32 bit hash without a multiple and non-recursive which might be faster?
	ALWAYS_INLINE uint32_t RuntimeHash(size_t count, char const* s) {
		uint32_t hash = 2166136261ul;
		for(size_t i = 0; i < count; ++i) {
			hash ^= s[i];
			hash += (hash<<1) + (hash<<4) + (hash<<7) + (hash<<8) + (hash<<24);
		}
		return hash;
	}
}

CONST_EXPR uint32_t operator"" _hash(char const* s, size_t count)
{
	return Core::CompileTimeHash(count, s);
}