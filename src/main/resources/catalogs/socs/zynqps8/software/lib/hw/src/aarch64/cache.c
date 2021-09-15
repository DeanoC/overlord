// SPDX-License-Identifier: MIT

#include "core/core.h"
#include "hw/cache.h"
#include "hw/memory_map.h"
#include "hw/reg_access.h"
#include "hw/aarch64/intrinsics_gcc.h"
#include "hw_regs/a53/a53_system.h"

#define IRQ_FIQ_MASK 0xC0U	/* Mask IRQ and FIQ interrupts in cpsr */
#define L1_DATA_PREFETCH_CONTROL_MASK  0xE000
#define L1_DATA_PREFETCH_CONTROL_SHIFT  13
//void Xil_ConfigureL1Prefetch(u8 num);

static void DCacheCleanAndMaybeInvalidate(bool invalidate)
{
	uint32_t currmask = read_DAIF_register();
	write_DAIF_register(currmask | IRQ_FIQ_MASK);

	for(uint32_t cacheLevel = 0; cacheLevel < 2;++cacheLevel) {
		// Select cache level D cache in CSSELR
		write_CSSELR_EL1_register(HW_REG_ENCODE_FIELD(A53_SYS, CSSELR_EL1, LEVEL, cacheLevel) );
		isb();

		uint32_t CsidReg = read_CCSIDR_EL1_register();

		// Get the cacheline size, way size, index size from csidr
		uint32_t LineSize = HW_REG_DECODE_FIELD(A53_SYS, CCSIDR_EL1, LINESIZE, CsidReg) + 4;
		uint32_t NumWays = HW_REG_DECODE_FIELD(A53_SYS, CCSIDR_EL1, ASSOCIATIVITY, CsidReg) + 1;
		uint32_t NumSet = HW_REG_DECODE_FIELD(A53_SYS, CCSIDR_EL1, NUMSETS, CsidReg) + 1;

		// Flush all the cachelines
		for (uint32_t Way = 0U; Way < NumWays; Way++) {
			for (uint32_t Set = 0U; Set < NumSet; Set++) {
				if(invalidate) {
					write_data_cache_register(CISW, (Way << (32 - __builtin_ctz(NumWays))) | (Set << LineSize) | (cacheLevel << 1));
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

static void DCacheCleanAndMaybeInvalidateRange(uintptr_t  adr, uintptr_t len, bool invalidate)
{
	uint32_t currmask = read_DAIF_register();
	write_DAIF_register(currmask | IRQ_FIQ_MASK);

	const uintptr_t cacheline = 64U;
	uintptr_t end = adr + len;
	adr = adr & (~0x3F);

	write_CSSELR_EL1_register(HW_REG_ENCODE_FIELD(A53_SYS, CSSELR_EL1, LEVEL, 0) );
	if (len != 0U) {
		while (adr < end) {
			if(invalidate)
				write_data_cache_register(CIVAC,adr);
			else
				write_data_cache_register(CVAC,adr);
			adr += cacheline;
		}
	}
	dsb();

	write_CSSELR_EL1_register(HW_REG_ENCODE_FIELD(A53_SYS, CSSELR_EL1, LEVEL, 1) );
	if (len != 0U) {
		while (adr < end) {
			if(invalidate)
				write_data_cache_register(CIVAC,adr);
			else
				write_data_cache_register(CVAC,adr);
			adr += cacheline;
		}
	}
	dsb();

	write_DAIF_register(currmask);
}


void Cache_DCacheEnable(void)
{
	// enable caches only if they are disabled
	if(!HW_REG_DECODE_FIELD(A53_SYS, SCTLR_EL3, C, read_SCTLR_EL3_register())){
		// invalidate the Data cache
		Cache_DCacheCleanAndInvalidate();
		// enable the Data cache for el3
		write_SCTLR_EL3_register(read_SCTLR_EL3_register() | HW_REG_ENCODE_FIELD(A53_SYS, SCTLR_EL3, C, 0x1));
	}
}

void Cache_ICacheEnable(void)
{
	// enable caches only if they are disabled
	if(!HW_REG_DECODE_FIELD(A53_SYS, SCTLR_EL3, I, read_SCTLR_EL3_register())){
		// invalidate the I cache
		Cache_ICacheCleanAndInvalidate();
		// enable the I cache for el3
		write_SCTLR_EL3_register(read_SCTLR_EL3_register() | HW_REG_ENCODE_FIELD(A53_SYS, SCTLR_EL3, I, 0x1));
	}
}

void Cache_DCacheDisable(void)
{
	// if dcache is enabled disable and flush
	if(!HW_REG_DECODE_FIELD(A53_SYS, SCTLR_EL3, C, read_SCTLR_EL3_register())) {
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
	Cache_ICacheCleanAndInvalidate();
	write_SCTLR_EL3_register(read_SCTLR_EL3_register() | HW_REG_ENCODE_FIELD(A53_SYS, SCTLR_EL3, I, 0x0));
}

uint32_t Cache_GetDCacheLineSizeInBytes(uint32_t level) {
	write_CSSELR_EL1_register(HW_REG_ENCODE_FIELD(A53_SYS, CSSELR_EL1, LEVEL, (level - 1) ) );
	uint32_t CsidReg = read_CCSIDR_EL1_register();
	uint32_t LineSize = HW_REG_DECODE_FIELD(A53_SYS, CCSIDR_EL1, LINESIZE, CsidReg) + 4;
	return 1 << LineSize;
}

uint32_t Cache_GetDCacheNumWays(uint32_t level) {
	write_CSSELR_EL1_register(HW_REG_ENCODE_FIELD(A53_SYS, CSSELR_EL1, LEVEL, (level - 1) ) );
	uint32_t CsidReg = read_CCSIDR_EL1_register();
	uint32_t NumWays = HW_REG_DECODE_FIELD(A53_SYS, CCSIDR_EL1, ASSOCIATIVITY, CsidReg) + 1;
	return NumWays;
}

uint32_t Cache_GetDCacheNumSets(uint32_t level) {
	write_CSSELR_EL1_register(HW_REG_ENCODE_FIELD(A53_SYS, CSSELR_EL1, LEVEL, (level - 1) ) );
	uint32_t CsidReg = read_CCSIDR_EL1_register();
	uint32_t NumSets = HW_REG_DECODE_FIELD(A53_SYS, CCSIDR_EL1, NUMSETS, CsidReg) + 1;
	return NumSets;
}

void Cache_DCacheClean(void)
{
	DCacheCleanAndMaybeInvalidate(false);
}

void Cache_DCacheCleanAndInvalidate(void)
{
	DCacheCleanAndMaybeInvalidate(true);
}

void Cache_DCacheCleanLine(uintptr_t adr)
{
	uint32_t currmask = read_DAIF_register();
	write_DAIF_register(currmask | IRQ_FIQ_MASK);

	write_CSSELR_EL1_register(HW_REG_ENCODE_FIELD(A53_SYS, CSSELR_EL1, LEVEL, 0) );
	write_data_cache_register(CIVAC,(adr & (~0x3F)));
	dsb();

	write_CSSELR_EL1_register(HW_REG_ENCODE_FIELD(A53_SYS, CSSELR_EL1, LEVEL, 1) );
	write_data_cache_register(IVAC,(adr & (~0x3F)));
	dsb();

	write_DAIF_register(currmask);
}

void Cache_DCacheCleanInvalidateLine(uintptr_t adr)
{
	uint32_t currmask = read_DAIF_register();
	write_DAIF_register(currmask | IRQ_FIQ_MASK);

	write_CSSELR_EL1_register(HW_REG_ENCODE_FIELD(A53_SYS, CSSELR_EL1, LEVEL, 0) );
	write_data_cache_register(CIVAC,(adr & (~0x3F)));
	dsb();

	write_CSSELR_EL1_register(HW_REG_ENCODE_FIELD(A53_SYS, CSSELR_EL1, LEVEL, 1) );
	write_data_cache_register(IVAC,(adr & (~0x3F)));
	dsb();

	write_DAIF_register(currmask);

}

void Cache_DCacheCleanInvalidateRange(uintptr_t  adr, uintptr_t len)
{
	DCacheCleanAndMaybeInvalidateRange(adr, len, true);
}

void Cache_DCacheCleanRange(uintptr_t  adr, uintptr_t len)
{
	DCacheCleanAndMaybeInvalidateRange(adr, len, false);
}

void Cache_ICacheCleanAndInvalidate(void)
{
	uint32_t currmask = read_DAIF_register();
	write_DAIF_register(currmask | IRQ_FIQ_MASK);

	write_CSSELR_EL1_register(	HW_REG_ENCODE_FIELD(A53_SYS, CSSELR_EL1, LEVEL, 0) |
																HW_REG_ENCODE_FIELD(A53_SYS, CSSELR_EL1, INOTD, 1));

	dsb();
	invalid_all_ICache();
	dsb();

	write_DAIF_register(currmask);
}

void Cache_ICacheCleanAndInvalidateLine(uintptr_t  adr)
{
	uint32_t currmask = read_DAIF_register();
	write_DAIF_register(currmask | IRQ_FIQ_MASK);

	write_instuction_cache_register(IVAU,adr & (~0x3F));
	dsb();
	write_DAIF_register(currmask);
}

void Cache_ICacheCleanAndInvalidateRange(uintptr_t  adr, uintptr_t len)
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

		write_CSSELR_EL1_register(	HW_REG_ENCODE_FIELD(A53_SYS, CSSELR_EL1, LEVEL, 0) |
																	HW_REG_ENCODE_FIELD(A53_SYS, CSSELR_EL1, INOTD, 1));
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