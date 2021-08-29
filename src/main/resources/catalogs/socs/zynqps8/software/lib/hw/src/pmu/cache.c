#include "core/core.h"

// PMU doesn't have any caches so this is just a dummy
void DCacheEnable(void){}
void ICacheEnable(void){}
void DCacheDisable(void){}
void ICacheDisable(void){}

void DCacheCleanAndInvalidate(void){}
void DCacheCleanAndInvalidateRange(uintptr_t adr, uintptr_t  len){}
void DDCacheCleanAndInvalidateLine(uintptr_t  adr){}

void ICacheCleanAndInvalidate(void){}
void ICacheCleanAndInvalidateRange(uintptr_t adr, uintptr_t  len){}
void ICacheCleanAndInvalidateLine(uintptr_t  adr){}

uint32_t GetDCacheLineSizeInBytes(uint32_t level){ return 0; }

uint32_t GetDCacheNumWays(uint32_t level) { return 0; }

uint32_t GetDCacheNumSets(uint32_t level) { return 0; }

