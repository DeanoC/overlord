// SPDX-License-Identifier: MIT

#include "core/core.h"
#include "dbg/raw_print.h"
#include "dbg/print.h"
#include "dbg/ansi_escapes.h"
#include "platform/reg_access.h"
#include "platform/cache.h"
#include "platform/aarch64/intrinsics_gcc.h"
#include "platform/memory_map.h"
#include "zynqps8/mmu/mmu.hpp"

#include "platform/registers/csu.h"
#include "platform/registers/pmu_global.h"
#include "platform/registers/acpu_gicc.h"
#include "platform/registers/acpu_gicd.h"
#include "platform/registers/a53_system.h"

#include "osservices/osservices.h"
#include "utils/busy_sleep.h"
#include "core/snprintf.h"
#include "pmu.hpp"
#include "dp.hpp"

void PrintBanner(void);
void EnablePSToPL(void);
void ClearPendingInterrupts(void);

EXTERN_C {
	extern uint8_t _binary_pmu_monitor_bin_start[];
	extern uint8_t _binary_pmu_monitor_bin_end[];

	extern uint8_t _vector_table[];
	extern uint8_t _end[];

	extern void RegisterBringUp(void);


	extern uint8_t HeapBase[];
	extern uint8_t HeapLimit[];

}

void EarlySetupMmu();
Mmu::Manager * SetupMmu();

#define CPU_CORTEXA53_0_TIMESTAMP_CLK_FREQ 33333000
#define PSSYSMON_ANALOG_BUS_OFFSET		0x114U

// main should never exit
EXTERN_C NO_RETURN void main()
{
	// TODO catch any printfs before RegisterBringUp OR UART early bringup
	debug_force_raw_print(true);

	// setup the early MMU enough to boot the PMU OS
	EarlySetupMmu();

	write_counter_timer_frequency_register(CPU_CORTEXA53_0_TIMESTAMP_CLK_FREQ);
	ClearPendingInterrupts();
	HW_REG_WRITE1(CSU, AES_RESET, CSU_AES_RESET_RESET);
	HW_REG_WRITE1(CSU, SHA_RESET, CSU_SHA_RESET_RESET);

	RegisterBringUp();

	debug_printf(ANSI_BRIGHT_ON "Bootloader size %luKB\nPMU size = %luKB\n" ANSI_BRIGHT_OFF,
	             ((size_t) _end - (size_t) _vector_table) >> 10,
	             ((size_t) _binary_pmu_monitor_bin_end - (size_t) _binary_pmu_monitor_bin_start)>>10);
	debug_printf("PMU load started\n");
	PmuSleep();

	PmuSafeMemcpy((void *) PMURAM_0_BASE_ADDR,
								(void *) _binary_pmu_monitor_bin_start,
								(size_t) _binary_pmu_monitor_bin_end - (size_t) _binary_pmu_monitor_bin_start);
	PmuWakeup();

	debug_printf("Wait for PMU\n");
	// stall until pmu says it loaded and ready to go
	while (!(HW_REG_READ1(PMU_GLOBAL, GLOBAL_GEN_STORAGE0) & OS_GLOBAL0_PMU_READY)) {}
	debug_printf("PMU " ANSI_GREEN_PEN"Ready\n" ANSI_WHITE_PEN);

	auto mmu = SetupMmu();

	// PMU OS is enabled
	debug_force_raw_print(false);
	EnablePSToPL();

	// ensure other a53 cores are asleep
	OsService_SleepCpus(OSSC_A53_1 | OSSC_A53_2 | OSSC_A53_3);

	// this is a silicon bug fix
	HW_REG_WRITE1(PSSYSMON, ANALOG_BUS, 0X00003210U);

	PrintBanner();

	debug_force_raw_print(false);

	debug_printf("Video boot console init\n");
	BringUpDisplayPort();
	debug_printf("Video boot console init " ANSI_GREEN_PEN "DONE\n" ANSI_WHITE_PEN);

	Cache_DCacheCleanAndInvalidate();

	debug_printf("Hard Boot Complete VideoBlock @ %#010x\n", videoBlock);

	// tell PMU we have finished pass video block address and mmu for next program
	BootData bootData = {
		.mmu = (uintptr_all_t)mmu,
		.bootCodeStart = OCM_0_BASE_ADDR,
		.bootCodeSize = OCM_0_SIZE_IN_BYTES,
		.videoBlock = videoBlock,
	};
	OsService_BootComplete(&bootData);

	HW_REG_WRITE1(PMU_GLOBAL, GLOBAL_GEN_STORAGE0, HW_REG_READ1(PMU_GLOBAL, GLOBAL_GEN_STORAGE0) | OS_GLOBAL0_BOOT_COMPLETE);

	debug_printf(ANSI_GREEN_PEN "BOOT DONE\n" ANSI_WHITE_PEN);

	while(true) {
		Utils_BusySecondSleep(1);
	}
}

void PrintBanner(void )
{
	debug_printf(ANSI_RESET_ATTRIBUTES ANSI_YELLOW_PEN "IKUY Boot Loader\n" ANSI_WHITE_PEN);
	debug_printf("Silicon Version %d\n", HW_REG_GET_FIELD(HW_REG_GET_ADDRESS(CSU), CSU, VERSION, PS_VERSION)+1);
	debug_printf( "A53 L1 Cache Size %dKiB, LineSize %d, Ways %d, Sets %d\n",
										(Cache_GetDCacheLineSizeInBytes(1) * Cache_GetDCacheNumWays(1) * Cache_GetDCacheNumSets(1)) / 1024,
										Cache_GetDCacheLineSizeInBytes(1),
										Cache_GetDCacheNumWays(1),
										Cache_GetDCacheNumSets(1) );
	debug_printf( "A53 L2 Cache Size %dKiB, LineSize %d, Ways %d, Sets %d\n",
										(Cache_GetDCacheLineSizeInBytes(2) * Cache_GetDCacheNumWays(2) * Cache_GetDCacheNumSets(2)) / 1024,
										Cache_GetDCacheLineSizeInBytes(2),
										Cache_GetDCacheNumWays(2),
										Cache_GetDCacheNumSets(2) );
	debug_printf("A53 NEON = %d, FMA = %d FP = 0x%x\n", __ARM_NEON, __ARM_FEATURE_FMA, __ARM_FP);
	uint32_t mmfr0 = read_ID_AA64MMFR0_EL1_register();
	debug_printf("MMU 4K Page = %d, MMU 16K Page = %d, MMU 64K Page = %d\n",
									 (mmfr0 & (1 << 28)) == 0,
									 (mmfr0 & (1 << 20)) != 0,
									 (mmfr0 & (1 << 24)) == 0);

}

void EnablePSToPL(void)
{
	HW_REG_SET_BIT1(CSU, PCAP_PROG, PCFG_PROG_B);
	HW_REG_SET_BIT1(PMU_GLOBAL, PS_CNTRL, PROG_GATE);
}

void ClearPendingInterrupts(void)
{
	// Clear pending peripheral interrupts
	HW_REG_WRITE1(ACPU_GICD, ICENABLER0, 0xFFFFFFFFU);
	HW_REG_WRITE1(ACPU_GICD, ICPENDR0, 0xFFFFFFFFU);
	HW_REG_WRITE1(ACPU_GICD, ICACTIVER0, 0xFFFFFFFFU);

	HW_REG_WRITE1(ACPU_GICD, ICENABLER1, 0xFFFFFFFFU);
	HW_REG_WRITE1(ACPU_GICD, ICPENDR1, 0xFFFFFFFFU);
	HW_REG_WRITE1(ACPU_GICD, ICACTIVER1, 0xFFFFFFFFU);

	HW_REG_WRITE1(ACPU_GICD, ICENABLER2, 0xFFFFFFFFU);
	HW_REG_WRITE1(ACPU_GICD, ICPENDR2, 0xFFFFFFFFU);
	HW_REG_WRITE1(ACPU_GICD, ICACTIVER2, 0xFFFFFFFFU);

	HW_REG_WRITE1(ACPU_GICD, ICENABLER3, 0xFFFFFFFFU);
	HW_REG_WRITE1(ACPU_GICD, ICPENDR3, 0xFFFFFFFFU);
	HW_REG_WRITE1(ACPU_GICD, ICACTIVER3, 0xFFFFFFFFU);

	HW_REG_WRITE1(ACPU_GICD, ICENABLER4, 0xFFFFFFFFU);
	HW_REG_WRITE1(ACPU_GICD, ICPENDR4, 0xFFFFFFFFU);
	HW_REG_WRITE1(ACPU_GICD, ICACTIVER4, 0xFFFFFFFFU);

	HW_REG_WRITE1(ACPU_GICD, ICENABLER5, 0xFFFFFFFFU);
	HW_REG_WRITE1(ACPU_GICD, ICPENDR5, 0xFFFFFFFFU);
	HW_REG_WRITE1(ACPU_GICD, ICACTIVER5, 0xFFFFFFFFU);

	// Clear active software generated interrupts, if any
	HW_REG_WRITE1(ACPU_GICC, EOIR, HW_REG_READ1(ACPU_GICC, IAR));

	// Clear pending software generated interrupts
	HW_REG_WRITE1(ACPU_GICD, CPENDSGIR0, 0xFFFFFFFFU);
	HW_REG_WRITE1(ACPU_GICD, CPENDSGIR1, 0xFFFFFFFFU);
	HW_REG_WRITE1(ACPU_GICD, CPENDSGIR2, 0xFFFFFFFFU);
	HW_REG_WRITE1(ACPU_GICD, CPENDSGIR3, 0xFFFFFFFFU);

}



EXTERN_C void SynchronousInterrupt(void) {
	uint64_t const esr = read_ESR_EL3_register();
	uint64_t const elr = read_ELR_EL3_register();
	uint32_t const ec = HW_REG_DECODE_FIELD(A53_SYSTEM, ESR_EL3, EC, esr);
	uint64_t const far = read_FAR_EL3_register();
	switch(ec) {
		case A53_SYSTEM_ESR_EL3_EC_UNKNOWN:
		case A53_SYSTEM_ESR_EL3_EC_WFX:
		case A53_SYSTEM_ESR_EL3_EC_COP15_AARCH32:
		case A53_SYSTEM_ESR_EL3_EC_TRAPPED_MCRR_MRRC_AARCH32:
		case A53_SYSTEM_ESR_EL3_EC_COP14_AARCH32:
		case A53_SYSTEM_ESR_EL3_EC_LDC_SRC_AARCH32:
		case A53_SYSTEM_ESR_EL3_EC_SVE_SIMD_FP:
		case A53_SYSTEM_ESR_EL3_EC_PAUTH:
		case A53_SYSTEM_ESR_EL3_EC_MRRC_COP14_AARCH32:
		case A53_SYSTEM_ESR_EL3_EC_BTI:
		case A53_SYSTEM_ESR_EL3_EC_ILLEGAL_EXECUTION:
		case A53_SYSTEM_ESR_EL3_EC_SVC:
		case A53_SYSTEM_ESR_EL3_EC_HVC:
		case A53_SYSTEM_ESR_EL3_EC_SMC:
		case A53_SYSTEM_ESR_EL3_EC_MSR_MRS:
		case A53_SYSTEM_ESR_EL3_EC_SVE:
		case A53_SYSTEM_ESR_EL3_EC_FPAC:
		case A53_SYSTEM_ESR_EL3_EC_IMPLEMENTATION_DEFINED:
		case A53_SYSTEM_ESR_EL3_EC_PC_ALIGNMENT_FAULT:
		case A53_SYSTEM_ESR_EL3_EC_SP_ALIGNMENT_FAULT:
		case A53_SYSTEM_ESR_EL3_EC_FLOATING_POINT:
		case A53_SYSTEM_ESR_EL3_EC_SERROR:
		case A53_SYSTEM_ESR_EL3_EC_BRK:
			raw_debug_printf(ANSI_RED_PEN "SynchronousInterrupt %#010lx ec %d %#010lx\n" ANSI_RESET_ATTRIBUTES,esr, ec, far);
			break;
		case A53_SYSTEM_ESR_EL3_EC_INSTRUCTION_ABORT_FROM_LOWER_EL:
		case A53_SYSTEM_ESR_EL3_EC_INSTRUCTION_ABORT:
			raw_debug_printf(ANSI_RED_PEN "Instruction Abort %#010lx ec %d %#010lx\n" ANSI_RESET_ATTRIBUTES,esr, ec, far);
			break;
		case A53_SYSTEM_ESR_EL3_EC_DATA_ABORT_FROM_LOWER_EL:
		case A53_SYSTEM_ESR_EL3_EC_DATA_ABORT: {
			uint32_t iss = HW_REG_DECODE_FIELD(A53_SYSTEM, ESR_EL3, ISS, esr);
			uint32_t dfsc = HW_REG_DECODE_FIELD(A53_SYSTEM, ISS_DATA_ABORT, DFSC, iss);
			raw_debug_printf(ANSI_RED_PEN "Data Abort @ %#018lx from instruction @ %#018lx: ", far, elr);
			int level = 0;
			switch(dfsc) {
				case A53_SYSTEM_ISS_DATA_ABORT_DFSC_ADDRESS_SIZE_FAULT_L3: level++;
					[[fallthrough]];
				case A53_SYSTEM_ISS_DATA_ABORT_DFSC_ADDRESS_SIZE_FAULT_L2: level++;
					[[fallthrough]];
				case A53_SYSTEM_ISS_DATA_ABORT_DFSC_ADDRESS_SIZE_FAULT_L1: level++;
					[[fallthrough]];
				case A53_SYSTEM_ISS_DATA_ABORT_DFSC_ADDRESS_SIZE_FAULT_L0:
					raw_debug_printf(ANSI_RED_PEN "Address Size fault Level %d\n" ANSI_RESET_ATTRIBUTES, level);
					break;
				case A53_SYSTEM_ISS_DATA_ABORT_DFSC_TRANSLATION_FAULT_L3: level++;
					[[fallthrough]];
				case A53_SYSTEM_ISS_DATA_ABORT_DFSC_TRANSLATION_FAULT_L2: level++;
					[[fallthrough]];
				case A53_SYSTEM_ISS_DATA_ABORT_DFSC_TRANSLATION_FAULT_L1: level++;
					[[fallthrough]];
				case A53_SYSTEM_ISS_DATA_ABORT_DFSC_TRANSLATION_FAULT_L0:
					raw_debug_printf(ANSI_RED_PEN "Translation fault Level %d\n" ANSI_RESET_ATTRIBUTES, level);
					break;
				case A53_SYSTEM_ISS_DATA_ABORT_DFSC_ACCESS_FLAG_FAULT_L3: level++;
					[[fallthrough]];
				case A53_SYSTEM_ISS_DATA_ABORT_DFSC_ACCESS_FLAG_FAULT_L2: level++;
					[[fallthrough]];
				case A53_SYSTEM_ISS_DATA_ABORT_DFSC_ACCESS_FLAG_FAULT_L1:
					raw_debug_printf(ANSI_RED_PEN "Access Flag fault Level %d\n" ANSI_RESET_ATTRIBUTES, level);
					break;
				case A53_SYSTEM_ISS_DATA_ABORT_DFSC_PERMISSION_FLAG_FAULT_L3: level++;
					[[fallthrough]];
				case A53_SYSTEM_ISS_DATA_ABORT_DFSC_PERMISSION_FLAG_FAULT_L2: level++;
					[[fallthrough]];
				case A53_SYSTEM_ISS_DATA_ABORT_DFSC_PERMISSION_FLAG_FAULT_L1:
					raw_debug_printf(ANSI_RED_PEN "Permission Flag fault Level %d\n" ANSI_RESET_ATTRIBUTES, level);
					break;
				case A53_SYSTEM_ISS_DATA_ABORT_DFSC_SYNCHRONOUS_EXTERNAL_ABORT_NOT_TTW:
					raw_debug_print(ANSI_RED_PEN "Synchronous External Abort Not TTW\n" ANSI_RESET_ATTRIBUTES);
					break;
				case A53_SYSTEM_ISS_DATA_ABORT_DFSC_SYNCHRONOUS_EXTERNAL_ABORT_L3: level++;
					[[fallthrough]];
				case A53_SYSTEM_ISS_DATA_ABORT_DFSC_SYNCHRONOUS_EXTERNAL_ABORT_L2: level++;
					[[fallthrough]];
				case A53_SYSTEM_ISS_DATA_ABORT_DFSC_SYNCHRONOUS_EXTERNAL_ABORT_L1: level++;
					[[fallthrough]];
				case A53_SYSTEM_ISS_DATA_ABORT_DFSC_SYNCHRONOUS_EXTERNAL_ABORT_L0:
					raw_debug_printf(ANSI_RED_PEN "Synchronous External Abort Level %d\n" ANSI_RESET_ATTRIBUTES, level);
					break;
				case A53_SYSTEM_ISS_DATA_ABORT_DFSC_ECC_ERROR_ABORT_NOT_TTW:
					raw_debug_print(ANSI_RED_PEN "ECC Error Abort Not TTW\n" ANSI_RESET_ATTRIBUTES);
					break;
				case A53_SYSTEM_ISS_DATA_ABORT_DFSC_ECC_ERROR_ABORT_L3: level++;
					[[fallthrough]];
				case A53_SYSTEM_ISS_DATA_ABORT_DFSC_ECC_ERROR_ABORT_L2: level++;
					[[fallthrough]];
				case A53_SYSTEM_ISS_DATA_ABORT_DFSC_ECC_ERROR_ABORT_L1: level++;
					[[fallthrough]];
				case A53_SYSTEM_ISS_DATA_ABORT_DFSC_ECC_ERROR_ABORT_L0:
					raw_debug_printf(ANSI_RED_PEN "ECC Error Abort Level %d\n" ANSI_RESET_ATTRIBUTES, level);
					break;
				case A53_SYSTEM_ISS_DATA_ABORT_DFSC_ALIGNMENT_FAULT:
					raw_debug_print(ANSI_RED_PEN "Alignment Fault\n" ANSI_RESET_ATTRIBUTES);
					break;
				case A53_SYSTEM_ISS_DATA_ABORT_DFSC_TLB_CONFLICT:
					raw_debug_print(ANSI_RED_PEN "TLB Conflict\n" ANSI_RESET_ATTRIBUTES);
					break;
				case A53_SYSTEM_ISS_DATA_ABORT_DFSC_UNSUPPORTED_ATOMIC_HARDWARE_FAULT:
					raw_debug_print(ANSI_RED_PEN "Unsupported Atomic Hardware Fault\n" ANSI_RESET_ATTRIBUTES);
					break;
				case A53_SYSTEM_ISS_DATA_ABORT_DFSC_LOCKDOWN:
					raw_debug_print(ANSI_RED_PEN "Lockdown Fault\n" ANSI_RESET_ATTRIBUTES);
					break;
				case A53_SYSTEM_ISS_DATA_ABORT_DFSC_UNSUPPORTED_EXCLUSIVE_OR_ATOMIC:
					raw_debug_print(ANSI_RED_PEN "Unsupported Exclusive or Atomic Fault\n" ANSI_RESET_ATTRIBUTES);
					break;
			}
			break;
		}
	}
	while(true) {
	}
}

EXTERN_C void IRQInterrupt(void) {
	uint32_t InterruptID = HW_REG_GET_FIELD( HW_REG_GET_ADDRESS(ACPU_GICC), ACPU_GICC, IAR, INTERRUPT_ID);
	raw_debug_printf("IRQInterrupt shouldn't FIRE!!! %x\n", InterruptID);
	HW_REG_SET_FIELD(HW_REG_GET_ADDRESS(ACPU_GICC), ACPU_GICC, EOIR, INTERRUPT_ID, InterruptID);
}

EXTERN_C void FIQInterrupt(void) {
	uint32_t InterruptID = HW_REG_GET_FIELD(HW_REG_GET_ADDRESS(ACPU_GICC), ACPU_GICC, IAR, INTERRUPT_ID);
	raw_debug_printf("Not handled FIQInterrupt %x\n", InterruptID);
	HW_REG_SET_FIELD(HW_REG_GET_ADDRESS(ACPU_GICC), ACPU_GICC, EOIR, INTERRUPT_ID, InterruptID);
}

EXTERN_C void SErrorInterrupt(void) {
	raw_debug_printf("SErrorInterruptHandler\n");
}

