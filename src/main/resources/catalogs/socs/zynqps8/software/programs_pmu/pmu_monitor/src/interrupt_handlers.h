#pragma once

#include "interrupts.h"
#ifdef __cplusplus
extern "C"
{
#endif

	void IPI0_Handler(Interrupt_Names irq_name);
	void IPI3_Handler(Interrupt_Names irq_name);
	void CorrectableECCErrors_Handler(Interrupt_Names irq_name);
	void GPI0_Handler(Interrupt_Names irq_name);
	void GPI1_Handler(Interrupt_Names irq_name);
	void GPI2_Handler(Interrupt_Names irq_name);
	void GPI3_Handler(Interrupt_Names irq_name);
	void RTCAlarms_Handler(Interrupt_Names irq_name);
	void RTCSeconds_Handler(Interrupt_Names irq_name);

#ifdef __cplusplus
}
#endif
