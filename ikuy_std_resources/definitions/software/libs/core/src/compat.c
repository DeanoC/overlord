//
// Created by deano on 8/25/22.
//

#include "core/core.h"

int strncmp(char const * RESTRICT a, char const * RESTRICT b, size_t bytes) {
	for(size_t i = 0; i < bytes; ++i) {
		if(!*a) return -1;
		if(!*b) return 1;
		int diff = (*a - *b);
		if(diff) return diff;
		a++;
		b++;
	}
	return 0;
}

static unsigned int rand_seed = 1;

void srand(unsigned int seed) {
	if(seed) rand_seed = 1;
}

int rand(void) {
	return(((rand_seed = rand_seed * 214013L + 2531011L) >> 16) & 0x7fff);
}
