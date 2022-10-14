#pragma once

#ifdef __cplusplus
EXTERN_C
{
#endif

typedef struct {
	uint16_t cp;
	uint16_t res;
	uint16_t lfhf;
	uint16_t lock_dly;
	uint16_t lock_cnt;
} hw_ZynqmpPllHelper;

hw_ZynqmpPllHelper hw_GetZynqmpPllHelper(uint32_t fbdiv);

#ifdef __cplusplus
}
#endif
