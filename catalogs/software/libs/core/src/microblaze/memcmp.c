#include "core/core.h"

NON_NULL(1,2) int memcmp(const void * const a, const void * const b, size_t count)
{
	const uint8_t *currentA = (const uint8_t*) a;
	const uint8_t *currentB = (const uint8_t*) b;
	int res = 0;

	while(0 < count) {
		res = *currentA - *currentB;
		if(BRANCH_LIKELY(res != 0)) break;

		currentA++;
		currentB++;
		count--;
	}

	return res;
}