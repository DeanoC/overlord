#include <stdbool.h>
#include <stdint.h>

int32_t atoi(const char * str)
{
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

	while(*str >= '0' && *str <= '9') {
		val = (10 * val) + (*str++ - '0');
	}

	return (neg ? -val : val);
}
