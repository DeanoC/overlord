#pragma once

#include "stdint.h"
#include <arm_acle.h>
#include <arm_neon.h>

#define read_CCSIDR_EL1_register() __extension__ ({ uint64_t rval = 0U; asm volatile("mrs %0,  CCSIDR_EL1" : "=r" (rval)); rval; })
#define write_CCSIDR_EL1_register(v) asm volatile("msr CCSIDR_EL1, %0" : : "r" (v))

#define read_CSSELR_EL1_register() __extension__ ({ uint64_t rval = 0U; asm volatile("mrs %0,  CSSELR_EL1" : "=r" (rval)); rval; })
#define write_CSSELR_EL1_register(v) asm volatile("msr CSSELR_EL1, %0" : : "r" (v))

#define read_TCR_EL3_register() __extension__ ({ uint64_t rval = 0U; asm volatile("mrs %0,  TCR_EL3" : "=r" (rval)); rval; })
#define write_TCR_EL3_register(v) asm volatile("msr TCR_EL3, %0" : : "r" (v))

#define read_SCTLR_EL3_register() __extension__ ({ uint64_t rval = 0U; asm volatile("mrs %0,  SCTLR_EL3" : "=r" (rval)); rval; })
#define write_SCTLR_EL3_register(v) asm volatile("msr SCTLR_EL3, %0" : : "r" (v))

#define read_TTBR0_EL3_register() __extension__ ({ uint64_t rval = 0U; asm volatile("mrs %0,  TTBR0_EL3" : "=r" (rval)); rval; })
#define write_TTBR0_EL3_register(v) asm volatile("msr TTBR0_EL3, %0" : : "r" (v))

#define read_MAIR_EL3_register() __extension__ ({ uint64_t rval = 0U; asm volatile("mrs %0,  MAIR_EL3" : "=r" (rval)); rval; })
#define write_MAIR_EL3_register(v) asm volatile("msr MAIR_EL3, %0" : : "r" (v))

#define read_DAIF_register() __extension__ ({ uint64_t rval = 0U; asm volatile("mrs %0,  DAIF" : "=r" (rval)); rval; })
#define write_DAIF_register(v) asm volatile("msr DAIF, %0" : : "r" (v))

#define read_SP_register() __extension__ ({ uint64_t rval = 0U; asm volatile("mov %0,  SP" : "=r" (rval)); rval; })
#define write_SP_register(v) asm volatile("mov SP, %0" : : "r" (v))

#define read_ESR_EL3_register() __extension__ ({ uint64_t rval = 0U; asm volatile("mrs %0, ESR_EL3" : "=r" (rval)); rval; })
#define read_ELR_EL3_register() __extension__ ({ uint64_t rval = 0U; asm volatile("mrs %0, ELR_EL3" : "=r" (rval)); rval; })
#define read_FAR_EL3_register() __extension__ ({ uint64_t rval = 0U; asm volatile("mrs %0, FAR_EL3" : "=r" (rval)); rval; })

#define write_counter_timer_frequency_register(freq) asm volatile("msr	CNTFRQ_EL0, %0" : : "r" (freq))
#define read_counter_timer_register() __extension__ ({uint64_t rval = 0U; asm volatile("mrs %0, CNTPCT_EL0" : "=r" (rval)); rval; })

#define read_ID_AA64MMFR0_EL1_register() __extension__ ({ uint64_t rval = 0U; asm volatile("mrs %0,  ID_AA64MMFR0_EL1" : "=r" (rval)); rval; })
#define read_MPIDR_EL1_register() __extension__ ({ uint64_t rval = 0U; asm volatile("mrs %0,  MPIDR_EL1" : "=r" (rval)); rval; })

#define isb() asm volatile("isb sy")
#define dsb() asm volatile("dsb sy")
#define dmb() asm volatile("dmb sy")

#define write_data_cache_register(reg,val) asm volatile("dc " #reg ",%0"  : : "r" (val))
#define write_instuction_cache_register(reg,val) asm volatile("ic " #reg ",%0"  : : "r" (val))
#define write_tlb_register(reg,val) asm volatile("tlbi " #reg ",%0"  : : "r" (val))

#define invalid_all_ICache()	asm volatile("ic IALLU")
#define invalid_all_TLB()	asm volatile("tlbi ALLE3")

#define read_current_EL_level() __extension__ ({uint32_t rval = 0U; asm volatile("mrs %0, CurrentEL" : "=r" (rval)); rval; })

#define CPSR_IRQ_ENABLE		0x80U
#define CPSR_FIQ_ENABLE		0x40U
#define enable_exceptions(v)  write_DAIF_register(read_DAIF_register() & ~(v))
#define disable_exceptions(v)  write_DAIF_register(read_DAIF_register() | (v))