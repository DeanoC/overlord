/******************************************************************************
* SPDX-License-Identifier: MIT
******************************************************************************/

#pragma once

#ifdef __cplusplus
EXTERN_C
{
#endif

void Cache_DCacheEnable(void);
void Cache_ICacheEnable(void);
void Cache_DCacheDisable(void);
void Cache_ICacheDisable(void);

// ARMv8 definitions
// Cleaning writes any dirty lines out to  main memory
// Invalidating means dropping data in caches (changes will be lost and the cache space will be available)
// Cleaning and Invalidating means first writing dirty lines and then freeing up cache space
// Zeroing means ignoring any actual data (whether cache or memory), and allocating a cache line with data all set to zero

// ICache only supports invalidating

void Cache_DCacheClean(void);
void Cache_DCacheCleanRange(uintptr_t adr, uintptr_t  len);
void Cache_DCacheCleanLine(uintptr_t  adr);

void Cache_DCacheInvalidate(void);
void Cache_DCacheInvalidateRange(uintptr_t adr, uintptr_t  len);
void Cache_DCacheInvalidateLine(uintptr_t  adr);

void Cache_DCacheCleanAndInvalidate(void);
void Cache_DCacheCleanAndInvalidateRange(uintptr_t adr, uintptr_t  len);
void Cache_DCacheCleanAndInvalidateLine(uintptr_t  adr);

void Cache_DCacheZeroRange(uintptr_t adr, uintptr_t  len);
void Cache_DCacheZeroLine(uintptr_t  adr);

void Cache_ICacheInvalidate(void);
void Cache_ICacheInvalidateRange(uintptr_t adr, uintptr_t  len);
void Cache_ICacheInvalidateLine(uintptr_t  adr);

uint32_t Cache_GetDCacheLineSizeInBytes(uint32_t level);

uint32_t Cache_GetDCacheNumWays(uint32_t level);

uint32_t Cache_GetDCacheNumSets(uint32_t level);

#ifdef __cplusplus
}
#endif