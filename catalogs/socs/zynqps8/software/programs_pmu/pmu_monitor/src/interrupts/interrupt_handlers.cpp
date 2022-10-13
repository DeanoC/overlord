#include "core/core.h"
#include "platform/reg_access.h"
#include "platform/memory_map.h"
#include "platform/registers/pmu_iomodule.h"
#include "platform/registers/pmu_lmb_bram.h"
#include "platform/registers/ipi.h"
#include "dbg/raw_print.h"
#include "dbg/print.h"

#include "interrupt_handlers.hpp"
#include "../rom_extensions.h"
#include "../os/ipi3_os_server.hpp"
#include "gic_proxy.hpp"

void IPI0_Handler(Interrupts::Name irq_name) {
	uint32_t isr = HW_REG_READ1(IPI, PMU_0_ISR);
	// write to clear to inform IPI PMU buffer is free to use now
	HW_REG_WRITE1(IPI, PMU_0_ISR, isr);

//	debug_printf ("IPI0_Handler 0x%lx\n", isr);
	RomServiceTable[REN_IPI0]();
}

void IPI3_Handler(Interrupts::Name irq_name) {
	uint32_t isr = HW_REG_READ1(IPI, PMU_3_ISR);
//	debug_printf ("irq_name 0x%x IPI3_Handler 0x%lx\n", irq_name, isr);

	for (uint32_t name = 0x80000000U; name != 0; name >>= 1) {
		if ((isr & name) == 0) {
			continue;
		}

		IPI3_OsServer::Handler((IPI_Channel) name);
	}

	// write to clear interrupt
	HW_REG_WRITE1(IPI, PMU_3_ISR, isr);
}

void CorrectableECCErrors_Handler(Interrupts::Name irq_name) {
	// write to clear status bit
	HW_REG_SET_BIT1(PMU_LMB_BRAM, ECC_STATUS, CE);
}

void GPI0_Handler(Interrupts::Name irq_name) {
	uint32_t gpi0 = HW_REG_READ1(PMU_IOMODULE, GPI0);
	// TODO use find first bit
	for (uint32_t name = 0x80000000U; name != 0; name >>= 1) {
		if ((gpi0 & name) == 0) continue;

		raw_debug_printf("GPI0_Handler 0x%lx\n", name);
		switch (name) {
			case PMU_IOMODULE_GPI0_RFT_ECC_FATAL_ERR: break;
			case PMU_IOMODULE_GPI0_RFT_VOTER_ERR: break;
			case PMU_IOMODULE_GPI0_RFT_COMPARE_ERR_23: break;
			case PMU_IOMODULE_GPI0_RFT_COMPARE_ERR_13: break;
			case PMU_IOMODULE_GPI0_RFT_COMPARE_ERR_12: break;
			case PMU_IOMODULE_GPI0_RFT_LS_MISMATCH_23_B: break;
			case PMU_IOMODULE_GPI0_RFT_LS_MISMATCH_13_B: break;
			case PMU_IOMODULE_GPI0_RFT_LS_MISMATCH_12_B: break;
			case PMU_IOMODULE_GPI0_RFT_MISMATCH_STATE: break;
			case PMU_IOMODULE_GPI0_RFT_MISMATCH_CPU: break;
			case PMU_IOMODULE_GPI0_RFT_SLEEP_RESET: break;
			case PMU_IOMODULE_GPI0_RFT_LS_MISMATCH_23_A: break;
			case PMU_IOMODULE_GPI0_RFT_LS_MISMATCH_13_A: break;
			case PMU_IOMODULE_GPI0_RFT_LS_MISMATCH_12_A: break;
			case PMU_IOMODULE_GPI0_NFT_ECC_FATAL_ERR: break;
			case PMU_IOMODULE_GPI0_NFT_VOTER_ERR: break;
			case PMU_IOMODULE_GPI0_NFT_COMPARE_ERR_23: break;
			case PMU_IOMODULE_GPI0_NFT_COMPARE_ERR_13: break;
			case PMU_IOMODULE_GPI0_NFT_COMPARE_ERR_12: break;
			case PMU_IOMODULE_GPI0_NFT_LS_MISMATCH_23_B: break;
			case PMU_IOMODULE_GPI0_NFT_LS_MISMATCH_13_B: break;
			case PMU_IOMODULE_GPI0_NFT_LS_MISMATCH_12_B: break;
			case PMU_IOMODULE_GPI0_NFT_MISMATCH_STATE: break;
			case PMU_IOMODULE_GPI0_NFT_MISMATCH_CPU: break;
			case PMU_IOMODULE_GPI0_NFT_SLEEP_RESET: break;
			case PMU_IOMODULE_GPI0_NFT_LS_MISMATCH_23_A: break;
			case PMU_IOMODULE_GPI0_NFT_LS_MISMATCH_13_A: break;
			case PMU_IOMODULE_GPI0_NFT_LS_MISMATCH_12_A: break;
			default: break;
		}
	}
}

void GPI1_Handler(Interrupts::Name irq_name) {
	uint32_t gpi1 = HW_REG_READ1(PMU_IOMODULE, GPI1);
//	raw_debug_printf("GPI1 0x%lx\n", HW_REG_READ1(PMU_IOMODULE, GPI1));

	// TODO use find first bit
	for (uint32_t name = 0x80000000U; name != 0; name >>= 1) {
		if ((gpi1 & name) == 0) continue;

		switch (name) {
			case PMU_IOMODULE_GPI1_APB_AIB_ERROR: RomServiceTable[REN_APBAIBERR]();
				break;
			case PMU_IOMODULE_GPI1_AXI_AIB_ERROR: RomServiceTable[REN_AXIAIBERR]();
				break;
			case PMU_IOMODULE_GPI1_ERROR_2: RomServiceTable[REN_ERROR2]();
				break;
			case PMU_IOMODULE_GPI1_ERROR_1: RomServiceTable[REN_ERROR1]();
				break;
			case PMU_IOMODULE_GPI1_ACPU_3_DBG_PWRUP: RomServiceTable[REN_ACPU3DBGPWRUP]();
				break;
			case PMU_IOMODULE_GPI1_ACPU_2_DBG_PWRUP: RomServiceTable[REN_ACPU2DBGPWRUP]();
				break;
			case PMU_IOMODULE_GPI1_ACPU_1_DBG_PWRUP: RomServiceTable[REN_ACPU1DBGPWRUP]();
				break;
			case PMU_IOMODULE_GPI1_ACPU_0_DBG_PWRUP: RomServiceTable[REN_ACPU0DBGPWRUP]();
				break;
			case PMU_IOMODULE_GPI1_FPD_WAKE_GIC_PROXY:
				GIC_Proxy(); break;
			case PMU_IOMODULE_GPI1_MIO_WAKE_5: RomServiceTable[REN_MIO5WAKE]();
				break;
			case PMU_IOMODULE_GPI1_MIO_WAKE_4: RomServiceTable[REN_MIO4WAKE]();
				break;
			case PMU_IOMODULE_GPI1_MIO_WAKE_3: RomServiceTable[REN_MIO3WAKE]();
				break;
			case PMU_IOMODULE_GPI1_MIO_WAKE_2: RomServiceTable[REN_MIO2WAKE]();
				break;
			case PMU_IOMODULE_GPI1_MIO_WAKE_1: RomServiceTable[REN_MIO1WAKE]();
				break;
			case PMU_IOMODULE_GPI1_MIO_WAKE_0: RomServiceTable[REN_MIO0WAKE]();
				break;
			case PMU_IOMODULE_GPI1_DAP_RPU_WAKE: RomServiceTable[REN_DAPRPUWAKE]();
				break;
			case PMU_IOMODULE_GPI1_DAP_FPD_WAKE: RomServiceTable[REN_DAPFPDWAKE]();
				break;
			case PMU_IOMODULE_GPI1_USB_1_WAKE: RomServiceTable[REN_USB1WAKE]();
				break;
			case PMU_IOMODULE_GPI1_USB_0_WAKE: RomServiceTable[REN_USB0WAKE]();
				break;
			case PMU_IOMODULE_GPI1_R5_1_WAKE: RomServiceTable[REN_R5F1WAKE]();
				break;
			case PMU_IOMODULE_GPI1_R5_0_WAKE: RomServiceTable[REN_R5F0WAKE]();
				break;
			case PMU_IOMODULE_GPI1_ACPU_3_WAKE: RomServiceTable[REN_ACPU3WAKE]();
				break;
			case PMU_IOMODULE_GPI1_ACPU_2_WAKE: RomServiceTable[REN_ACPU2WAKE]();
				break;
			case PMU_IOMODULE_GPI1_ACPU_1_WAKE: RomServiceTable[REN_ACPU1WAKE]();
				break;
			case PMU_IOMODULE_GPI1_ACPU_0_WAKE: RomServiceTable[REN_ACPU0WAKE]();
				break;
			default: break;
		}
	}
//	raw_debug_printf("GPI1 0x%lx\n", HW_REG_READ1(PMU_IOMODULE, GPI1));

}

void GPI2_Handler(Interrupts::Name irq_name) {
	uint32_t gpi2 = HW_REG_READ1(PMU_IOMODULE, GPI2);

	// TODO use find first bit
	for (uint32_t name = 0x80000000U; name != 0; name >>= 1) {
		if ((gpi2 & name) == 0) {
			continue;
		}

		raw_debug_printf("GPI2_Handler 0x%lx\n", name);
		switch (name) {
			case PMU_IOMODULE_GPI2_ACPU_0_SLEEP: RomServiceTable[REN_ACPU0SLEEP]();
				break;
			case PMU_IOMODULE_GPI2_ACPU_1_SLEEP: RomServiceTable[REN_ACPU1SLEEP]();
				break;
			case PMU_IOMODULE_GPI2_ACPU_2_SLEEP: RomServiceTable[REN_ACPU2SLEEP]();
				break;
			case PMU_IOMODULE_GPI2_ACPU_3_SLEEP: RomServiceTable[REN_ACPU3SLEEP]();
				break;
			case PMU_IOMODULE_GPI2_R5_0_SLEEP: RomServiceTable[REN_R5F0SLEEP]();
				break;
			case PMU_IOMODULE_GPI2_R5_1_SLEEP: RomServiceTable[REN_R5F1SLEEP]();
				break;
			case PMU_IOMODULE_GPI2_DBG_RCPU0_RST_REQ: RomServiceTable[REN_RCPU0_DBG_RST]();
				break;
			case PMU_IOMODULE_GPI2_DBG_RCPU1_RST_REQ: RomServiceTable[REN_RCPU1_DBG_RST]();
				break;
			case PMU_IOMODULE_GPI2_CP_ACPU0_RST_REQ: RomServiceTable[REN_ACPU0_CP_RST]();
				break;
			case PMU_IOMODULE_GPI2_CP_ACPU1_RST_REQ: RomServiceTable[REN_ACPU1_CP_RST]();
				break;
			case PMU_IOMODULE_GPI2_CP_ACPU2_RST_REQ: RomServiceTable[REN_ACPU2_CP_RST]();
				break;
			case PMU_IOMODULE_GPI2_CP_ACPU3_RST_REQ: RomServiceTable[REN_ACPU3_CP_RST]();
				break;
			case PMU_IOMODULE_GPI2_DBG_ACPU0_RST_REQ: RomServiceTable[REN_ACPU0_DBG_RST]();
				break;
			case PMU_IOMODULE_GPI2_DBG_ACPU1_RST_REQ: RomServiceTable[REN_ACPU1_DBG_RST]();
				break;
			case PMU_IOMODULE_GPI2_DBG_ACPU2_RST_REQ: RomServiceTable[REN_ACPU2_DBG_RST]();
				break;
			case PMU_IOMODULE_GPI2_DBG_ACPU3_RST_REQ: RomServiceTable[REN_ACPU3_DBG_RST]();
				break;
			case PMU_IOMODULE_GPI2_VCC_AUX_DISCONNECT: RomServiceTable[REN_VCCAUX_DISCONNECT]();
				break;
			case PMU_IOMODULE_GPI2_VCC_INT_DISCONNECT: RomServiceTable[REN_VCCINT_DISCONNECT]();
				break;
			case PMU_IOMODULE_GPI2_VCC_INT_FP_DISCONNECT: RomServiceTable[REN_VCCINTFP_DISCONNECT]();
				break;
			default: break;
		}
	}
}

void GPI3_Handler(Interrupts::Name irq_name) {
	uint32_t gpi3 = HW_REG_READ1(PMU_IOMODULE, GPI3);

	// TODO use find first bit
	for (uint32_t name = 0x80000000U; name != 0; name >>= 1) {
		if ((gpi3 & name) == 0) {
			continue;
		}
		raw_debug_printf("GPI3_Handler 0x%lx\n", name);

		switch (name) {
			default: break;
		}
	}
}

void RTCAlarms_Handler(Interrupts::Name irq_name) {
}

void RTCSeconds_Handler(Interrupts::Name irq_name) {
	static bool tick = true;
	if(tick) {
		raw_debug_print("tick\n");
		tick = false;
	} else {
		raw_debug_print("tock\n");
		tick = true;
	}

}
