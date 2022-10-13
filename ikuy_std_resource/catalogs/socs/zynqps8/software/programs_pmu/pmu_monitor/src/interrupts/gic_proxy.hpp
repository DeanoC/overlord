#pragma once

#include "core/math.h"
#include "platform/zynqmp/interrupts.h"

#define GICPROXY_INTERRUPT_GROUP_SIZE (LPD_SLCR_GICP1_IRQ_STATUS_OFFSET - LPD_SLCR_GICP0_IRQ_STATUS_OFFSET)

CONST_EXPR ALWAYS_INLINE Interrupt_Names GIC_ProxyToName(const uint32_t group, const uint32_t src) {
	switch(group) {
		default:
		case 0: return (Interrupt_Names)(INT_UNKNOWN_0 + 0 + (Math_LogTwo_U32(src)));
		case 1: return (Interrupt_Names)(INT_UNKNOWN_0 + 32 + Math_LogTwo_U32(src));
		case 2: return (Interrupt_Names)(INT_UNKNOWN_0 + 64 + Math_LogTwo_U32(src));
		case 3: return (Interrupt_Names)(INT_UNKNOWN_0 + 96 + Math_LogTwo_U32(src));
		case 4: return (Interrupt_Names)(INT_UNKNOWN_0 + 128 + Math_LogTwo_U32(src));
	}
}

CONST_EXPR ALWAYS_INLINE void GIC_NameToProxy(Interrupt_Names name, uint32_t* group, uint32_t* src) {
	const uint32_t group0start = INT_UNKNOWN_0;
	const uint32_t group0end = INT_UNKNOWN_0 + 32;
	const uint32_t group1end = group0end + 32;
	const uint32_t group2end = group1end + 32;
	const uint32_t group3end = group2end + 32;
	const uint32_t group4end = group3end + 32;

	if(name < group0start) {
		*group = 0xFFFFFFFF; // error
		return;
	}

	if(name < group0end) {
		*group = 0;
		*src = Math_PowTwo_U32(8 + name - group0start);
	} else if(name < group1end) {
		*group = 1;
		*src = Math_PowTwo_U32(name - group1end);
	}else if(name < group2end) {
		*group = 2;
		*src = Math_PowTwo_U32(name - group2end);
	}else if(name < group3end) {
		*group = 3;
		*src = Math_PowTwo_U32(name - group3end);
	}else if(name < group4end) {
		*group = 4;
		*src = Math_PowTwo_U32(name - group4end);
	}
}

// return true to call the ROM routine
void GIC_Proxy();

