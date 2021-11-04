/******************************************************************************
* Copyright (c) 2020 - 2021 Dean Calver.  All rights reserved.
* Copyright (c) 2015 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
#include "core/core.h"
#include "core/c/math.h"
#include "dbg/raw_print.h"
#include "hw/reg_access.h"
#include "hw/memory_map.h"
#include "hw/microblaze/intrinsics_gcc.h"
#include "hw_regs/pmu/pmu_iomodule.h"
#include "hw_regs/pmu/pmu_local.h"
#include "hw_regs/pmu_global.h"
#include "interrupts.h"

static Interrupt_HandlerFunction InterruptTable[32];

// IRQ_ENABLE register is write-only, So its state is stored here
static uint32_t ShadowIrqEnable;

WEAK_LINKAGE void Interrupt_NullHandler(Interrupt_Names name)
{
	const char* in = "UNKNOWN INTERRUPT";
	switch (name) {
		case IN_PIT0: in = "PIT0"; break;
		case IN_PIT1: in = "PIT1"; break;
		case IN_PIT2: in = "PIT2"; break;
		case IN_PIT3: in = "PIT3"; break;
		case IN_GPI0: in = "GPI0"; break;
		case IN_GPI1: in = "GPI1"; break;
		case IN_GPI2: in = "GPI2"; break;
		case IN_GPI3: in = "GPI3"; break;
		case IN_RTC_ALARM: in = "RTC_ALARM"; break;
		case IN_RTC_SECONDS: in = "RTC_SECONDS"; break;
		case IN_CORRECTABLE_ECC: in = "CORRECTABLE_ECC"; break;
		case IN_IPI0: in = "IPI0"; break;
		case IN_IPI1: in = "IPI1"; break;
		case IN_IPI2: in = "IPI2"; break;
		case IN_IPI3: in = "IPI3"; break;
		case IN_FW_REQ: in = "FW_REQ"; break;
		case IN_ISOLATION_REQ: in = "ISOLATION_REQ"; break;
		case IN_HW_RESET_REQ: in = "HW_RESET_REQ"; break;
		case IN_SW_RESET_REQ: in = "SW_RESET_REQ"; break;
		case IN_INVALID_ADDRESS: in = "INVALID_ADDRESS"; break;
		default: break;
	}

	raw_debug_printf("Error: NullHandler Interrupt Triggered by %s!\n", in);
}
static void PulseInterrupts(void)
{
	const uint32_t IntMaskReg1 = ~(HW_REG_GET(PMU_GLOBAL, ERROR_INT_MASK_1));
	const uint32_t IntMaskReg2 = ~(HW_REG_GET(PMU_GLOBAL, ERROR_INT_MASK_2));

	// Disable PMU interrupts in PMU Global register
	HW_REG_SET(PMU_GLOBAL, ERROR_INT_DIS_1, IntMaskReg1);
	HW_REG_SET(PMU_GLOBAL, ERROR_INT_DIS_2, IntMaskReg2);
	// Enable PMU interrupts in PMU Global register
	HW_REG_SET(PMU_GLOBAL, ERROR_INT_EN_1, IntMaskReg1);
	HW_REG_SET(PMU_GLOBAL, ERROR_INT_EN_2, IntMaskReg2);
}

/*
void Break_Handler(void)  __attribute__ ((break_handler));

void Break_Handler(void)
{
}
*/
void Exception_Handler(void) {
	raw_debug_print("Exception!");
}

void Interrupt_Handler(void)
{
	// Latch the IRQ_PENDING register into a local variable
	const uint32_t irqReg = HW_REG_GET(PMU_IOMODULE, IRQ_PENDING);
//	debug_printf("pre irq 0x%lx\n", irqReg);

	// Loop through the Handler Table and handle the trigger interrupts
	// TODO use find first bit
	for (uint32_t name = 0x80000000U; name > 0; name >>= 1) {
		if(irqReg & name) {
			const uint32_t functionIndex = Math_LogTwo_U32(name);
			Interrupt_HandlerFunction function = InterruptTable[functionIndex];
			function((Interrupt_Names)name);

			// ACK we have processed the interrupts
			HW_REG_SET(PMU_IOMODULE, IRQ_ACK, name);
		}
	}
//	const uint32_t irqReg2 = HW_REG_GET(PMU_IOMODULE, IRQ_PENDING);
//	debug_printf("post irq2 0x%lx\n", irqReg2);

	// Disable and Enable PMU interrupts in PMU Global register.
	// This will re-generated any interrupt which is generated while
	// serving the other interrupt
	PulseInterrupts();
}

void Interrupts_Disable(Interrupt_Names name)
{
	ShadowIrqEnable = ShadowIrqEnable & (~name);
	HW_REG_SET(PMU_IOMODULE, IRQ_ENABLE, ShadowIrqEnable);
}

void Interrupts_Enable(Interrupt_Names name)
{
	ShadowIrqEnable = ShadowIrqEnable | name;
	HW_REG_SET(PMU_IOMODULE, IRQ_ENABLE, ShadowIrqEnable);
}

void Interrupts_SetHandler(Interrupt_Names name, Interrupt_HandlerFunction handler) {
	InterruptTable[Math_LogTwo_U32(name)] = handler;
}

void Interrupts_Init(void)
{
	for (int i = 0; i < 32; ++i) {
		InterruptTable[i] = &Interrupt_NullHandler;
	}

	HW_REG_SET(PMU_IOMODULE, IRQ_ENABLE, 0);
	microblaze_disable_exceptions();
	microblaze_disable_interrupts();
	HW_REG_SET(PMU_IOMODULE, IRQ_ACK, 0xffffffffU);
	ShadowIrqEnable = 0;
}

void Interrupts_Start(void) {
	HW_REG_SET(PMU_IOMODULE, IRQ_ENABLE, ShadowIrqEnable);
	microblaze_enable_exceptions();
	microblaze_enable_interrupts();
}
#define HW_EXCEPTION_RECEIVED	0x2U

__attribute__((noreturn)) void XPfw_Exception_Handler(void)
{
	raw_debug_printf("Received exception\r\n"
									 "MSR: 0x%x, EAR: 0x%x, EDR: 0x%x, ESR: 0x%x\r\n",
									 mfmsr(), mfear(), mfedr(), mfesr());
	/* Write error occurrence to PERS register and trigger FW Error1 */
	HW_REG_MERGE(PMU_GLOBAL, PERS_GLOB_GEN_STORAGE5, HW_EXCEPTION_RECEIVED, HW_EXCEPTION_RECEIVED);
	HW_REG_MERGE(PMU_LOCAL, PMU_SERV_ERR, PMU_LOCAL_PMU_SERV_ERR_FWERR_MASK, PMU_LOCAL_PMU_SERV_ERR_FWERR_MASK);

	while(1) {
		;
	}
}

#if 0

static void PwrUpHandler(void)
{
	XStatus Status = XPfw_CoreDispatchEvent(XPFW_EV_REQ_PWRUP);

	if (XST_SUCCESS != Status) {
		XPfw_Printf(DEBUG_DETAILED,"Warning: Failed to dispatch Event ID:"
															 " %d\r\n",XPFW_EV_REQ_PWRUP);
	}
}

static void PwrDnHandler(void)
{
	XStatus Status = XPfw_CoreDispatchEvent(XPFW_EV_REQ_PWRDN);

	if (XST_SUCCESS != Status) {
		XPfw_Printf(DEBUG_DETAILED,"Warning: Failed to dispatch Event ID:"
															 " %d\r\n",XPFW_EV_REQ_PWRDN);
	}
}

static void IsolationHandler(void)
{
	XStatus Status = XPfw_CoreDispatchEvent(XPFW_EV_REQ_ISOLATION);

	if (XST_SUCCESS != Status) {
		XPfw_Printf(DEBUG_DETAILED,"Warning: Failed to dispatch Event ID:"
															 " %d\r\n",XPFW_EV_REQ_ISOLATION);
	}
}

static void Pit1Handler(void)
{
	XPfw_CoreTickHandler();
}

#endif
