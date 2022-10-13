#include "core/core.h"

#ifdef __cplusplus
EXTERN_C
{
#endif

// PMU doesn't have any caches so this is just a dummy
void Cache_DCacheEnable(void){}
void Cache_ICacheEnable(void){}
void Cache_DCacheDisable(void){}
void Cache_ICacheDisable(void){}

void Cache_DCacheClean(void){}
void Cache_DCacheCleanRange(uintptr_t adr, uintptr_t  len){}
void Cache_DCacheCleanLine(uintptr_t  adr){}

void Cache_DCacheCleanAndInvalidate(void){}
void Cache_DCacheCleanAndInvalidateRange(uintptr_t adr, uintptr_t  len){}
void Cache_DCacheCleanAndInvalidateLine(uintptr_t  adr){}

void Cache_ICacheCleanAndInvalidate(void){}
void Cache_ICacheCleanAndInvalidateRange(uintptr_t adr, uintptr_t  len){}
void Cache_ICacheCleanAndInvalidateLine(uintptr_t  adr){}

uint32_t Cache_GetDCacheLineSizeInBytes(uint32_t level){ return 0; }

uint32_t Cache_GetDCacheNumWays(uint32_t level) { return 0; }

uint32_t Cache_GetDCacheNumSets(uint32_t level) { return 0; }


#ifdef __cplusplus
}
#endif