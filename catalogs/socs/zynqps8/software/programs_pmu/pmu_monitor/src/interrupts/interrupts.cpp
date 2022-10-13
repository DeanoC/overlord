/******************************************************************************
* Copyright (c) 2020 - 2021 Dean Calver.  All rights reserved.
* Copyright (c) 2015 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
#include "core/core.h"
#include "core/math.h"
#include "dbg/raw_print.h"
#include "platform/reg_access.h"
#include "platform/memory_map.h"
#include "platform/microblaze/intrinsics_gcc.h"
#include "platform/registers/pmu_iomodule.h"
#include "platform/registers/pmu_local.h"
#include "platform/registers/pmu_global.h"
#include "interrupts.hpp"

EXTERN_C WEAK_LINKAGE void NullHandler(Interrupts::Name name) {
	using namespace Interrupts;
	if(name == Name::IN_IPI3) return;

	const char *in = "UNKNOWN INTERRUPT";
	switch (name) {
		case Name::IN_PIT0: in = "PIT0";
			break;
		case Name::IN_PIT1: in = "PIT1";
			break;
		case Name::IN_PIT2: in = "PIT2";
			break;
		case Name::IN_PIT3: in = "PIT3";
			break;
		case Name::IN_GPI0: in = "GPI0";
			break;
		case Name::IN_GPI1: in = "GPI1";
			break;
		case Name::IN_GPI2: in = "GPI2";
			break;
		case Name::IN_GPI3: in = "GPI3";
			break;
		case Name::IN_RTC_ALARM: in = "RTC_ALARM";
			break;
		case Name::IN_RTC_SECONDS: in = "RTC_SECONDS";
			break;
		case Name::IN_CORRECTABLE_ECC: in = "CORRECTABLE_ECC";
			break;
		case Name::IN_IPI0: in = "IPI0";
			break;
		case Name::IN_IPI1: in = "IPI1";
			break;
		case Name::IN_IPI2: in = "IPI2";
			break;
		case Name::IN_IPI3: in = "IPI3";
			break;
		case Name::IN_FW_REQ: in = "FW_REQ";
			break;
		case Name::IN_ISOLATION_REQ: in = "ISOLATION_REQ";
			break;
		case Name::IN_HW_RESET_REQ: in = "HW_RESET_REQ";
			break;
		case Name::IN_SW_RESET_REQ: in = "SW_RESET_REQ";
			break;
		case Name::IN_INVALID_ADDRESS: in = "INVALID_ADDRESS";
			break;
		default: break;
	}
	raw_debug_printf("Error: NullHandler Interrupt Triggered by %s!\n", in);
}

namespace Interrupts {
static HandlerFunction InterruptTable[32];

// IRQ_ENABLE register is write-only, So its state is stored here
static uint32_t ShadowIrqEnable;


static void PulseInterrupts() {
	const uint32_t IntMaskReg1 = ~(HW_REG_READ1(PMU_GLOBAL, ERROR_INT_MASK_1));
	const uint32_t IntMaskReg2 = ~(HW_REG_READ1(PMU_GLOBAL, ERROR_INT_MASK_2));

	// Disable PMU interrupts in PMU Global register
	HW_REG_WRITE1(PMU_GLOBAL, ERROR_INT_DIS_1, IntMaskReg1);
	HW_REG_WRITE1(PMU_GLOBAL, ERROR_INT_DIS_2, IntMaskReg2);
	// Enable PMU interrupts in PMU Global register
	HW_REG_WRITE1(PMU_GLOBAL, ERROR_INT_EN_1, IntMaskReg1);
	HW_REG_WRITE1(PMU_GLOBAL, ERROR_INT_EN_2, IntMaskReg2);
}

void Exception_Handler() {
	raw_debug_print("Exception!");
}


void Disable(Name name) {
	ShadowIrqEnable = ShadowIrqEnable & ~((uint32_t)name);
	HW_REG_WRITE1(PMU_IOMODULE, IRQ_ENABLE, ShadowIrqEnable);
}

void Enable(Name name) {
	ShadowIrqEnable = ShadowIrqEnable | (uint32_t)name;
	HW_REG_WRITE1(PMU_IOMODULE, IRQ_ENABLE, ShadowIrqEnable);
}

void SetHandler(Name name, HandlerFunction handler) {
	InterruptTable[Math_LogTwo_U32((uint32_t)name)] = handler;
}

void Init() {
	for (int i = 0; i < 32; ++i) {
		InterruptTable[i] = &NullHandler;
	}

	HW_REG_WRITE1(PMU_IOMODULE, IRQ_ENABLE, 0);
	microblaze_disable_exceptions();
	microblaze_disable_interrupts();
	HW_REG_WRITE1(PMU_IOMODULE, IRQ_ACK, 0xffffffffU);
	ShadowIrqEnable = 0;
}

void Start() {
	HW_REG_WRITE1(PMU_IOMODULE, IRQ_ENABLE, ShadowIrqEnable);
	microblaze_enable_exceptions();
	microblaze_enable_interrupts();
}
#define HW_EXCEPTION_RECEIVED  0x2U

__attribute__((noreturn)) void XPfw_Exception_Handler(void) {
	raw_debug_printf("Received exception\r\n"
									 "MSR: 0x%x, EAR: 0x%x, EDR: 0x%x, ESR: 0x%x\r\n",
									 mfmsr(), mfear(), mfedr(), mfesr());
	/* Write error occurrence to PERS register and trigger FW Error1 */
	HW_REG_RMW1(PMU_GLOBAL, PERS_GLOB_GEN_STORAGE5, HW_EXCEPTION_RECEIVED, HW_EXCEPTION_RECEIVED);
	HW_REG_RMW1(PMU_LOCAL, PMU_SERV_ERR, PMU_LOCAL_PMU_SERV_ERR_FWERR_MASK, PMU_LOCAL_PMU_SERV_ERR_FWERR_MASK);

	while (1) { ;
	}
}

} // end namespace

EXTERN_C void Interrupt_Handler() {
	using namespace Interrupts;
	// Latch the IRQ_PENDING register into a local variable
	const uint32_t irqReg = HW_REG_READ1(PMU_IOMODULE, IRQ_PENDING);
//	raw_debug_printf("pre irq 0x%lx\n", irqReg);
	//raw_debug_printf("NullHandler %p\n", &NullHandler);

	// Loop through the Handler Table and handle the trigger interrupts
	// TODO use find first bit
	for (uint32_t name = 0x80000000U; name > 0; name >>= 1) {
		if (irqReg & name) {
			const uint32_t functionIndex = Math_LogTwo_U32(name);
			HandlerFunction function = InterruptTable[functionIndex];
			function((Name) name);
		}
	}
	// ACK we have processed the interrupts
	HW_REG_WRITE1(PMU_IOMODULE, IRQ_ACK, irqReg);

	//	const uint32_t irqReg2 = HW_REG_READ1(PMU_IOMODULE, IRQ_PENDING);
	//	debug_printf("post irq2 0x%lx\n", irqReg2);

	// Disable and Enable PMU interrupts in PMU Global register.
	// This will re-generated any interrupt which is generated while
	// serving the other interrupt
	PulseInterrupts();
}
