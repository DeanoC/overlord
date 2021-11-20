// SPDX-License-Identifier: MIT

#include "core/core.h"
#include "hw/reg_access.h"
#include "hw/memory_map.h"
#include "hw_regs/pmu_global.h"
#include "hw_regs/pmu/pmu_lmb_bram.h"
#include "hw_regs/pmu/pmu_iomodule.h"
#include "hw_regs/pmu/pmu_local.h"

#include "hw_regs/lpd_slcr.h"
#include "hw_regs/ipi.h"
#include "hw_regs/ttc.h"
#include "hw_regs/uart.h"
#include "dbg/raw_print.h"
#include "dbg/ansi_escapes.h"
#include "dbg/print.h"

#include "interrupts.hpp"
#include "interrupt_handlers.hpp"
#include "ipi3_os_server.hpp"
#include "gic_proxy.hpp"
#include "host_interface.hpp"
#include "rom_extensions.h"
#include "timers.hpp"
#include "os_heap.hpp"
#include "main_loop.hpp"

typedef struct {
	uint32_t namesz;
	uint32_t descsz;
	uint32_t type;
} ElfNoteSection_t;

MainLoop loopy;

extern const ElfNoteSection_t g_note_build_id;
void PrintBanner(void)
{
	raw_debug_print(ANSI_YELLOW_PEN "IKUY PMU Monitor\n" ANSI_RESET_ATTRIBUTES);
	const uint8_t *build_id_data = &((uint8_t*)(&g_note_build_id)+1)[g_note_build_id.namesz];

	// print Build ID
	raw_debug_print("Build ID: ");
	for (uint32_t i = 0; i < g_note_build_id.descsz; ++i) raw_debug_printf("%02x", build_id_data[i]);
	raw_debug_print("\n");
}

void ttc0_1_setup(void) {
	uint32_t group = 0;
	uint32_t bit = 0;

	GIC_NameToProxy(GICIN_TTC0_1, &group, &bit);

	HW_REG_SET(TTC0, CLOCK_CONTROL_1, 0);
	HW_REG_SET(TTC0, INTERVAL_COUNTER_1, 0);
	HW_REG_SET(TTC0, MATCH_1_COUNTER_1, 0);
	HW_REG_SET(TTC0, MATCH_1_COUNTER_2, 0);
	HW_REG_SET(TTC0, MATCH_1_COUNTER_3, 0);
	HW_REG_SET(TTC0, INTERRUPT_ENABLE_1, 0);
	HW_REG_SET(TTC0, INTERRUPT_REGISTER_1, TTC_INTERRUPT_REGISTER_1_USERMASK);

	HW_REG_SET(TTC0, INTERVAL_COUNTER_1, 0x0F00FFFF);
	HW_REG_SET_BIT(TTC0, COUNTER_CONTROL_1, RST);
	HW_REG_SET_BIT(TTC0, COUNTER_CONTROL_1, INT);
	hw_RegWrite(LPD_SLCR_BASE_ADDR, LPD_SLCR_GICP0_IRQ_ENABLE_OFFSET + (group * GICPROXY_INTERRUPT_GROUP_SIZE), bit);
	hw_RegWrite(LPD_SLCR_BASE_ADDR, LPD_SLCR_GICP_PMU_IRQ_ENABLE_OFFSET, Math_PowTwo_U32(group));

	HW_REG_SET(TTC0, INTERRUPT_ENABLE_1, TTC_INTERRUPT_REGISTER_1_IV);
	HW_REG_CLR_BIT(TTC0, COUNTER_CONTROL_1, DIS);
}


void main() __attribute__((noreturn));

void main()
{
	HW_REG_CLR_BIT(PMU_GLOBAL, SAFETY_GATE, SCAN_ENABLE);

	// init disables interrupts and exceptions, start will enable them
	Interrupts::Init();

	HW_REG_CLR_BIT(PMU_GLOBAL, GLOBAL_CNTRL, DONT_SLEEP);

	PrintBanner();

	// sleep all other hard core processors in SoC
	// xsct fails with power down error if ACPU0 is asleep
//	RomServiceTable[REN_ACPU0SLEEP]();
	RomServiceTable[REN_ACPU1SLEEP]();
	RomServiceTable[REN_ACPU2SLEEP]();
	RomServiceTable[REN_ACPU3SLEEP]();
	RomServiceTable[REN_R50SLEEP]();
	RomServiceTable[REN_R51SLEEP]();

	OsHeap::Init();
	loopy.Init();

	IPI3_OsServer::Init();

	// install IPI0 as PMU sleep handlers (TODO seems a waste of an entire IPI...)
	HW_REG_SET(IPI, PMU_0_ISR, IPI_PMU_0_ISR_USERMASK);
	HW_REG_SET(IPI, PMU_0_IER, IPI_PMU_0_IER_USERMASK);
	Interrupts::SetHandler(Interrupts::Name::IN_IPI0, &IPI0_Handler);
	Interrupts::Enable(Interrupts::Name::IN_IPI0);

	// IPI3 OS service handler
	HW_REG_SET(IPI, PMU_3_ISR, IPI_PMU_3_ISR_USERMASK);
	HW_REG_SET(IPI, PMU_3_IER, IPI_PMU_3_IER_USERMASK);
	Interrupts::SetHandler(Interrupts::Name::IN_IPI3, &IPI3_Handler);
	Interrupts::Enable(Interrupts::Name::IN_IPI3);

	// ECC correctable interrupt
	HW_REG_SET_BIT(PMU_LMB_BRAM, ECC_STATUS, CE);
	HW_REG_SET_BIT(PMU_LMB_BRAM, ECC_IRQ_EN, CE);
	Interrupts::SetHandler(Interrupts::Name::IN_CORRECTABLE_ECC, &CorrectableECCErrors_Handler);
	Interrupts::Enable(Interrupts::Name::IN_CORRECTABLE_ECC);

	// GPI0 (Fault tolerance events) (always enabled)
	Interrupts::SetHandler(Interrupts::Name::IN_GPI0, &GPI0_Handler);
	Interrupts::Enable(Interrupts::Name::IN_GPI0);

	// GPI1 (Wakeup events)
	HW_REG_SET(PMU_LOCAL, GPI1_ENABLE, PMU_LOCAL_GPI1_ENABLE_USERMASK);
	Interrupts::SetHandler(Interrupts::Name::IN_GPI1, &GPI1_Handler);
	Interrupts::Enable(Interrupts::Name::IN_GPI1);

	// GPI1 is also used for GIC_PROXY
	HW_REG_SET(LPD_SLCR, GICP0_IRQ_ENABLE, LPD_SLCR_GICP0_IRQ_ENABLE_SRC21);
	HW_REG_SET(LPD_SLCR, GICP_PMU_IRQ_ENABLE, LPD_SLCR_GICP_PMU_IRQ_ENABLE_SRC0);
	HW_REG_SET(UART0, INTRPT_DIS, 	~UART_CHNL_INT_STS_RTRIG);
	HW_REG_SET(UART0, INTRPT_EN, 	UART_CHNL_INT_STS_RTRIG);
	HW_REG_SET(UART0, RCVR_TIMEOUT, 32);
	HW_REG_SET(UART0, RCVR_FIFO_TRIGGER_LEVEL, 1);
	ttc0_1_setup();

	// GPI2 (Reset and sleep events)
	HW_REG_SET(PMU_LOCAL, GPI2_ENABLE, PMU_LOCAL_GPI2_ENABLE_USERMASK);
	Interrupts::SetHandler(Interrupts::Name::IN_GPI2, &GPI2_Handler);
	Interrupts::Enable(Interrupts::Name::IN_GPI2);

	// GPI3 (PL to PMU events)
	HW_REG_SET(PMU_LOCAL, GPI3_ENABLE, PMU_LOCAL_GPI3_ENABLE_USERMASK);
	Interrupts::SetHandler(Interrupts::Name::IN_GPI3, &GPI3_Handler);
	Interrupts::Enable(Interrupts::Name::IN_GPI3);

	Timers::Init();

	Interrupts::Start();

	HW_REG_SET(PMU_GLOBAL, GLOBAL_GEN_STORAGE0, 1);

	debug_print("IKUY PMU Firmware setup complete\n");
	Timers::Start();

	HW_REG_SET_BIT(PMU_GLOBAL, GLOBAL_CNTRL, FW_IS_PRESENT);

	loopy.Loop();

	while(true) {
	}

}



