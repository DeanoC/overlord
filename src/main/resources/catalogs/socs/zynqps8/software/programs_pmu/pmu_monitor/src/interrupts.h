#pragma once
#ifdef __cplusplus
extern "C"
{
#endif

#include "hw_regs/pmu/pmu_iomodule.h"

typedef enum {
	IN_PIT0 = PMU_IOMODULE_IRQ_ENABLE_PIT0,
	IN_PIT1 = PMU_IOMODULE_IRQ_ENABLE_PIT1,
	IN_PIT2 = PMU_IOMODULE_IRQ_ENABLE_PIT2,
	IN_PIT3 = PMU_IOMODULE_IRQ_ENABLE_PIT3,
	IN_GPI0 = PMU_IOMODULE_IRQ_ENABLE_GPI0,
	IN_GPI1 = PMU_IOMODULE_IRQ_ENABLE_GPI1,
	IN_GPI2 = PMU_IOMODULE_IRQ_ENABLE_GPI2,
	IN_GPI3 = PMU_IOMODULE_IRQ_ENABLE_GPI3,
	IN_RTC_ALARM = PMU_IOMODULE_IRQ_ENABLE_RTC_ALARM,
	IN_RTC_SECONDS = PMU_IOMODULE_IRQ_ENABLE_RTC_EVERY_SECOND,
	IN_CORRECTABLE_ECC = PMU_IOMODULE_IRQ_ENABLE_CORRECTABLE_ECC,
	IN_IPI0 = PMU_IOMODULE_IRQ_ENABLE_IPI0,
	IN_IPI1 = PMU_IOMODULE_IRQ_ENABLE_IPI1,
	IN_IPI2 = PMU_IOMODULE_IRQ_ENABLE_IPI2,
	IN_IPI3 = PMU_IOMODULE_IRQ_ENABLE_IPI3,
	IN_FW_REQ = PMU_IOMODULE_IRQ_ENABLE_FW_REQ,
	IN_ISOLATION_REQ = PMU_IOMODULE_IRQ_ENABLE_ISO_REQ,
	IN_HW_RESET_REQ = PMU_IOMODULE_IRQ_ENABLE_HW_RST_REQ,
	IN_SW_RESET_REQ = PMU_IOMODULE_IRQ_ENABLE_SW_RST_REQ,
	IN_INVALID_ADDRESS = PMU_IOMODULE_IRQ_ENABLE_INV_ADDR,
} Interrupt_Names;

typedef void(*Interrupt_HandlerFunction)(Interrupt_Names name);

void Interrupts_Init(void);
void Interrupts_SetHandler(Interrupt_Names name, Interrupt_HandlerFunction handler) NON_NULL(2);
void Interrupts_Disable(Interrupt_Names name);
void Interrupts_Enable(Interrupt_Names name);
void Interrupts_Start(void);

void Interrupt_Handler(void) __attribute((interrupt_handler));

#ifdef __cplusplus
}
#endif
