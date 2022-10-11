#include "core/core.h"
#include "dbg/print.h"
#include "platform/reg_access.h"
#include "platform/a53/memory_map.h"
#include "platform/registers/acpu_gic.h"
#include "platform/registers/acpu_gicc.h"
#include "platform/registers/acpu_gicd.h"
#include "zynqps8/gic_v2/gic_v2.hpp"
#include "platform/cpu.h"
#include "multi_core/mutex.h"

namespace GicV2 {

static Core_Mutex mutex;

void InitDist() {
	// disable distributor
	HW_REG_WRITE1(ACPU_GICD, CTLR, 0x0);

	// set shared interrupts to edge level active high (2 bits, 16 interrupts at time)
	for (int i = INT_UNKNOWN_0; i < Interrupt_Names::INT_SMMU; i += 16) {
		// i >> 2 = (INT / 16) * 4
		hw_RegWrite(ACPU_GICD_BASE_ADDR, ACPU_GICD_ICFGR0_OFFSET + (i >> 2), 0);
	}
	// priorities private don't have priority so index starts at 0 for this register bank
	// 1 byte per interrupt, 4 at a time
	static const uint32_t DefaultPriority = 0xa0a0a0a0U;
	for (int i = 0; i < Interrupt_Names::INT_SMMU - INT_UNKNOWN_0; i += 4) {
		hw_RegWrite(ACPU_GICD_BASE_ADDR, ACPU_GICD_ICFGR0_OFFSET + i, DefaultPriority);
	}
	// disable all shared interrupts by default (32 at time)
	for (int i = 0; i < Interrupt_Names::INT_SMMU - INT_UNKNOWN_0; i += 32) {
		// i >> 3 = (INT / 32) * 4
		hw_RegWrite(ACPU_GICD_BASE_ADDR, ACPU_GICD_ICENABLER0_OFFSET + (i >> 3), 0xffffffffU);
	}
	// enable distributor for both groups
	HW_REG_WRITE1(ACPU_GICD, CTLR, 0x3);
}

void InitCPU() {
	disable_exceptions(CPSR_IRQ_ENABLE | CPSR_FIQ_ENABLE);

	HW_REG_SET_FIELD(HW_REG_GET_ADDRESS(ACPU_GICC), ACPU_GICC, PMR, PRIORITY, 0xF0);

	// enable cpu for both groups
	HW_REG_WRITE1(ACPU_GICC, CTLR, 0x0);
	HW_REG_SET_BIT1(ACPU_GICC, CTLR, ENABLE_GROUP_0);
	HW_REG_SET_BIT1(ACPU_GICC, CTLR, ENABLE_GROUP_1);
	HW_REG_SET_BIT1(ACPU_GICC, CTLR, GROUP_0_FIQ);

	enable_exceptions(CPSR_IRQ_ENABLE | CPSR_FIQ_ENABLE);
}

void EnableInterruptForThisCore(Interrupt_Names name_) {
	MultiCore_LockMutex(&mutex);

	// update distributor
	{
		uint32_t const cpuId = GetCpuHartNumber();
		uint32_t const targetRegNum = name_ / 4;
		uint32_t const interruptIdReg = name_ & 0x3;
		uint32_t targets = hw_RegRead(ACPU_GICD_BASE_ADDR,
																	ACPU_GICD_ITARGETSR0_OFFSET + (targetRegNum * 4));
		targets |= 1 << ((interruptIdReg * 8) + cpuId);
		hw_RegWrite(ACPU_GICD_BASE_ADDR, ACPU_GICD_ITARGETSR0_OFFSET + (targetRegNum * 4), targets);
	}

	// enable
	{
		uint32_t const targetRegNum = name_ / 32;
		uint32_t const interruptIdReg = name_ & 0x1f;

		hw_RegWrite(ACPU_GICD_BASE_ADDR,
								ACPU_GICD_ISENABLER0_OFFSET + (targetRegNum * 4),
								(1 << interruptIdReg));
	}
	MultiCore_UnlockMutex(&mutex);
}

void DisableInterruptForThisCore(Interrupt_Names name_) {
	MultiCore_LockMutex(&mutex);

	// disable
	{
		uint32_t const targetRegNum = name_ / 32;
		uint32_t const interruptIdReg = name_ & 0x1f;

		hw_RegWrite(ACPU_GICD_BASE_ADDR,
								ACPU_GICD_ICENABLER0_OFFSET + (targetRegNum * 4),
								(1 << interruptIdReg));
	}

	// update distributor
	{
		uint32_t const cpuId = GetCpuHartNumber();
		uint32_t const targetRegNum = name_ / 4;
		uint32_t const interruptIdReg = name_ & 0x3;
		uint32_t targets = hw_RegRead(ACPU_GICD_BASE_ADDR,
																	ACPU_GICD_ITARGETSR0_OFFSET + (targetRegNum * 4));
		targets &= ~(cpuId << (interruptIdReg << 8));
		targets |= 1 << ((interruptIdReg * 8) + cpuId);

		hw_RegWrite(ACPU_GICD_BASE_ADDR, ACPU_GICD_ITARGETSR0_OFFSET + (targetRegNum * 4), targets);
	}


	MultiCore_UnlockMutex(&mutex);
}

} // end namespace