// SPDX-License-Identifier: MIT

#include "core/core.h"
#include "platform/cache.h"
#include "platform/memory_map.h"
#include "platform/reg_access.h"
#include "platform/aarch64/intrinsics_gcc.h"
#include "platform/registers/a53_system.h"
#include "dbg/assert.h"

#define IRQ_FIQ_MASK (CPSR_IRQ_ENABLE | CPSR_FIQ_ENABLE)
#define L1_DATA_PREFETCH_CONTROL_MASK  0xE000
#define L1_DATA_PREFETCH_CONTROL_SHIFT  13
//void Xil_ConfigureL1Prefetch(u8 num);

#define CACHE_LINE_SIZE 64U
#define CACHE_LEVELS 2


static void DCacheMaybeCleanAndInvalidate(bool clean_, bool invalidate_) {
	assert( clean_ || invalidate_);

	uint32_t currmask = read_DAIF_register();
	write_DAIF_register(currmask | IRQ_FIQ_MASK);

	for(uint32_t cacheLevel = 0; cacheLevel < 2;++cacheLevel) {
		// Select cache level D cache in CSSELR
		write_CSSELR_EL1_register(HW_REG_ENCODE_FIELD(A53_SYSTEM, CSSELR_EL1, LEVEL, cacheLevel) );
		isb();

		uint32_t CsidReg = read_CCSIDR_EL1_register();

		// Get the cacheline size, way size, index size from csidr
		uint32_t LineSize = HW_REG_DECODE_FIELD(A53_SYSTEM, CCSIDR_EL1, LINESIZE, CsidReg) + 4;
		uint32_t NumWays = HW_REG_DECODE_FIELD(A53_SYSTEM, CCSIDR_EL1, ASSOCIATIVITY, CsidReg) + 1;
		uint32_t NumSet = HW_REG_DECODE_FIELD(A53_SYSTEM, CCSIDR_EL1, NUMSETS, CsidReg) + 1;

		// Flush all the cachelines
		for (uint32_t Way = 0U; Way < NumWays; Way++) {
			for (uint32_t Set = 0U; Set < NumSet; Set++) {
				if(invalidate_ && clean_) {
					write_data_cache_register(CISW, (Way << (32 - __builtin_ctz(NumWays))) | (Set << LineSize) | (cacheLevel << 1));
				} else if(invalidate_) {
					write_data_cache_register(ISW, (Way << (32 - __builtin_ctz(NumWays))) | (Set << LineSize) | (cacheLevel << 1));
				} else {
					write_data_cache_register(CSW, (Way << (32 - __builtin_ctz(NumWays))) | (Set << LineSize) | (cacheLevel << 1));
				}
			}
		}
		// Wait for Flush to complete
		dsb();
	}

	write_DAIF_register(currmask);
}


static void DCacheMaybeCleanAndInvalidateRange( uintptr_t const adr_, uintptr_t const len_, bool clean_, bool invalidate_){
	assert( clean_ || invalidate_);

	// not cleaning could lose data, so make sure everything is aligned
	if(!clean_) {
		assert((adr_ & (CACHE_LINE_SIZE - 1)) == 0 );
		assert((len_ & (CACHE_LINE_SIZE - 1)) == 0 );
	}
	if(len_ == 0) return;
	uint32_t currmask = read_DAIF_register();
	write_DAIF_register(currmask | IRQ_FIQ_MASK);

	uintptr_t const start = adr_ & (~0x3F);
	uintptr_t const end = adr_ + len_;
	int cacheLevel = 0;
doCache:
	write_CSSELR_EL1_register(HW_REG_ENCODE_FIELD(A53_SYSTEM, CSSELR_EL1, LEVEL, cacheLevel) );
	for(uintptr_t address = start; address < end; address += CACHE_LINE_SIZE) {
		if(clean_ && invalidate_)
			write_data_cache_register(CIVAC,address);
		else if(invalidate_)
			write_data_cache_register(IVAC,address);
		else
			write_data_cache_register(CVAC,address);
	}
	dsb();
	cacheLevel++;
	// do both l1 and l2
	if(cacheLevel < CACHE_LEVELS) goto doCache;

	write_DAIF_register(currmask);
}

void Cache_DCacheEnable(void)
{
	// enable caches only if they are disabled
	if(!HW_REG_DECODE_FIELD(A53_SYSTEM, SCTLR_EL3, C, read_SCTLR_EL3_register())){
		// invalidate the Data cache
		Cache_DCacheCleanAndInvalidate();
		// enable the Data cache for el3
		write_SCTLR_EL3_register(read_SCTLR_EL3_register() | HW_REG_ENCODE_FIELD(A53_SYSTEM, SCTLR_EL3, C, 0x1));
	}
}

void Cache_ICacheEnable(void)
{
	// enable caches only if they are disabled
	if(!HW_REG_DECODE_FIELD(A53_SYSTEM, SCTLR_EL3, I, read_SCTLR_EL3_register())){
		// invalidate the I cache
		Cache_ICacheInvalidate();
		// enable the I cache for el3
		write_SCTLR_EL3_register(read_SCTLR_EL3_register() | HW_REG_ENCODE_FIELD(A53_SYSTEM, SCTLR_EL3, I, 0x1));
	}
}

void Cache_DCacheDisable(void)
{
	// if dcache is enabled disable and flush
	if(!HW_REG_DECODE_FIELD(A53_SYSTEM, SCTLR_EL3, C, read_SCTLR_EL3_register())) {
		asm(
				"dsb sy\n\t"
				"mov 	x0, #0\n\t"
				"mrs	x0, SCTLR_EL3 \n\t"
				"and	w0, w0, #0xfffffffb\n\t"
				"msr	SCTLR_EL3, x0\n\t"
				"dsb sy\n\t"
				);
		Cache_DCacheCleanAndInvalidate();

		asm(
				"tlbi ALLE3\n\t"
				"dsb sy\r\n"
				"isb\n\t"
				);
	}
}
void Cache_ICacheDisable(void)
{
	// invalidate the instruction cache
	Cache_ICacheInvalidate();
	write_SCTLR_EL3_register(read_SCTLR_EL3_register() | HW_REG_ENCODE_FIELD(A53_SYSTEM, SCTLR_EL3, I, 0x0));
}

uint32_t Cache_GetDCacheLineSizeInBytes(uint32_t level) {
	write_CSSELR_EL1_register(HW_REG_ENCODE_FIELD(A53_SYSTEM, CSSELR_EL1, LEVEL, (level - 1) ) );
	uint32_t CsidReg = read_CCSIDR_EL1_register();
	uint32_t LineSize = HW_REG_DECODE_FIELD(A53_SYSTEM, CCSIDR_EL1, LINESIZE, CsidReg) + 4;
	return 1 << LineSize;
}

uint32_t Cache_GetDCacheNumWays(uint32_t level) {
	write_CSSELR_EL1_register(HW_REG_ENCODE_FIELD(A53_SYSTEM, CSSELR_EL1, LEVEL, (level - 1) ) );
	uint32_t CsidReg = read_CCSIDR_EL1_register();
	uint32_t NumWays = HW_REG_DECODE_FIELD(A53_SYSTEM, CCSIDR_EL1, ASSOCIATIVITY, CsidReg) + 1;
	return NumWays;
}

uint32_t Cache_GetDCacheNumSets(uint32_t level) {
	write_CSSELR_EL1_register(HW_REG_ENCODE_FIELD(A53_SYSTEM, CSSELR_EL1, LEVEL, (level - 1) ) );
	uint32_t CsidReg = read_CCSIDR_EL1_register();
	uint32_t NumSets = HW_REG_DECODE_FIELD(A53_SYSTEM, CCSIDR_EL1, NUMSETS, CsidReg) + 1;
	return NumSets;
}

void Cache_DCacheClean(void)
{
	DCacheMaybeCleanAndInvalidate(true, false);
}

void Cache_DCacheInvalidate(void)
{
	DCacheMaybeCleanAndInvalidate(false, true);
}

void Cache_DCacheCleanAndInvalidate(void)
{
	DCacheMaybeCleanAndInvalidate(true, true);
}

void Cache_DCacheCleanLine(uintptr_t adr)
{
	uint32_t currmask = read_DAIF_register();
	write_DAIF_register(currmask | IRQ_FIQ_MASK);

	write_CSSELR_EL1_register(HW_REG_ENCODE_FIELD(A53_SYSTEM, CSSELR_EL1, LEVEL, 0) );
	write_data_cache_register(CVAC,(adr & (~0x3F)));
	dsb();

	write_CSSELR_EL1_register(HW_REG_ENCODE_FIELD(A53_SYSTEM, CSSELR_EL1, LEVEL, 1) );
	write_data_cache_register(CVAC,(adr & (~0x3F)));
	dsb();

	write_DAIF_register(currmask);
}

void Cache_DCacheInvalidateLine(uintptr_t adr_) {
	assert((adr_ & (CACHE_LINE_SIZE-1)) == 0);
	uint32_t currmask = read_DAIF_register();
	write_DAIF_register(currmask | IRQ_FIQ_MASK);

	write_CSSELR_EL1_register(HW_REG_ENCODE_FIELD(A53_SYSTEM, CSSELR_EL1, LEVEL, 0) );
	write_data_cache_register(IVAC,(adr_ & (~0x3F)));
	dsb();

	write_CSSELR_EL1_register(HW_REG_ENCODE_FIELD(A53_SYSTEM, CSSELR_EL1, LEVEL, 1) );
	write_data_cache_register(IVAC,(adr_ & (~0x3F)));
	dsb();

	write_DAIF_register(currmask);
}

void Cache_DCacheCleanAndInvalidateLine(uintptr_t adr_) {
	uint32_t currmask = read_DAIF_register();
	write_DAIF_register(currmask | IRQ_FIQ_MASK);

	write_CSSELR_EL1_register(HW_REG_ENCODE_FIELD(A53_SYSTEM, CSSELR_EL1, LEVEL, 0) );
	write_data_cache_register(CIVAC,(adr_ & (~0x3F)));
	dsb();

	write_CSSELR_EL1_register(HW_REG_ENCODE_FIELD(A53_SYSTEM, CSSELR_EL1, LEVEL, 1) );
	write_data_cache_register(CIVAC,(adr_ & (~0x3F)));
	dsb();

	write_DAIF_register(currmask);

}
void Cache_DCacheZeroLine(uintptr_t adr_) {
	// not cleaning could lose data, so make sure everything is aligned
	assert((adr_ & (CACHE_LINE_SIZE-1)) == 0);
	uint32_t currmask = read_DAIF_register();
	write_DAIF_register(currmask | IRQ_FIQ_MASK);

	write_CSSELR_EL1_register(HW_REG_ENCODE_FIELD(A53_SYSTEM, CSSELR_EL1, LEVEL, 0) );
	write_data_cache_register(ZVA, adr_);
	dsb();

	write_CSSELR_EL1_register(HW_REG_ENCODE_FIELD(A53_SYSTEM, CSSELR_EL1, LEVEL, 1) );
	write_data_cache_register(ZVA, adr_);
	dsb();

	write_DAIF_register(currmask);
}

void Cache_DCacheCleanAndInvalidateRange(uintptr_t  adr, uintptr_t len) {
	DCacheMaybeCleanAndInvalidateRange( adr, len, true, true);
}

void Cache_DCacheInvalidateRange(uintptr_t  adr, uintptr_t len) {
	DCacheMaybeCleanAndInvalidateRange( adr, len, false, true);
}

void Cache_DCacheCleanRange(uintptr_t  adr, uintptr_t len) {
	DCacheMaybeCleanAndInvalidateRange( adr, len, true, false);
}

void Cache_DCacheZeroRange(uintptr_t adr_, uintptr_t len_)
{
	assert((adr_ & (CACHE_LINE_SIZE-1)) == 0);
	assert((len_ & (CACHE_LINE_SIZE-1)) == 0);

	if(len_ == 0) return;

	uint32_t currmask = read_DAIF_register();
	write_DAIF_register(currmask | IRQ_FIQ_MASK);

	uintptr_t const start = adr_ & (~0x3F);
	uintptr_t const end = adr_ + len_;
	int cacheLevel = 0;
	doCache:
	write_CSSELR_EL1_register(HW_REG_ENCODE_FIELD(A53_SYSTEM, CSSELR_EL1, LEVEL, cacheLevel) );
	for(uintptr_t address = start; address < end; address += CACHE_LINE_SIZE) {
		write_data_cache_register(ZVA,address);
	}
	dsb();
	cacheLevel++;
	// do both l1 and l2
	if(cacheLevel < CACHE_LEVELS) goto doCache;

	write_DAIF_register(currmask);
}
void Cache_ICacheInvalidate(void)
{
	uint32_t currmask = read_DAIF_register();
	write_DAIF_register(currmask | IRQ_FIQ_MASK);

	write_CSSELR_EL1_register(	HW_REG_ENCODE_FIELD(A53_SYSTEM, CSSELR_EL1, LEVEL, 0) |
																HW_REG_ENCODE_FIELD(A53_SYSTEM, CSSELR_EL1, INOTD, 1));

	dsb();
	invalid_all_ICache();
	dsb();

	write_DAIF_register(currmask);
}

void Cache_ICacheInvalidateLine(uintptr_t  adr)
{
	uint32_t currmask = read_DAIF_register();
	write_DAIF_register(currmask | IRQ_FIQ_MASK);

	write_instuction_cache_register(IVAU,adr & (~0x3F));
	dsb();
	write_DAIF_register(currmask);
}

void Cache_ICacheInvalidateRange(uintptr_t  adr, uintptr_t len)
{
	uint32_t currmask = read_DAIF_register();
	write_DAIF_register(currmask | IRQ_FIQ_MASK);

	const uintptr_t cacheline = 64U;
	uintptr_t end;
	uintptr_t tempadr = adr;
	uintptr_t tempend;

	if (len != 0x00000000U) {
		end = tempadr + len;
		tempend = end;
		tempadr &= ~(cacheline - 0x00000001U);

		write_CSSELR_EL1_register(	HW_REG_ENCODE_FIELD(A53_SYSTEM, CSSELR_EL1, LEVEL, 0) |
																	HW_REG_ENCODE_FIELD(A53_SYSTEM, CSSELR_EL1, INOTD, 1));
		while (tempadr < tempend) {
			write_instuction_cache_register(IVAU,adr & (~0x3F));
			tempadr += cacheline;
		}
	}
	dsb();

	write_DAIF_register(currmask);
}

#if 0
/****************************************************************************/
/**
* @brief	Configure the maximum number of outstanding data prefetches
*               allowed in L1 cache.
*
* @param	num: maximum number of outstanding data prefetches allowed,
*                    valid values are 0-7.
*
* @return	None.
*
* @note		This function is implemented only for EL3 privilege level.
*
*****************************************************************************/
void Xil_ConfigureL1Prefetch (uint8_t num) {
	uint64_t val=0;

	val= mfcp(S3_1_C15_C2_0 );
	val &= ~(L1_DATA_PREFETCH_CONTROL_MASK);
	val |=  (num << L1_DATA_PREFETCH_CONTROL_SHIFT);
	mtcp(S3_1_C15_C2_0,val);
}

#endif