// SPDX-License-Identifier: MIT

#include "core/core.h"
#include "platform/reg_access.h"
#include "platform/memory_map.h"
#include "platform/registers/pmu_global.h"
#include "platform/registers/pmu_local.h"

#include "platform/registers/lpd_slcr.h"
#include "platform/registers/ipi.h"
#include "platform/registers/uart.h"
#include "dbg/raw_print.h"
#include "dbg/ansi_escapes.h"
#include "dbg/print.h"

#include "interrupts/interrupts.hpp"
#include "interrupts/interrupt_handlers.hpp"
#include "os/ipi3_os_server.hpp"
#include "rom_extensions.h"
#include "timers.hpp"
#include "os_heap.hpp"
#include "main_loop.hpp"
#include "osservices/osservices.h"
#include "cpuwake.hpp"

#define UART_DEBUG_BASE_ADDR UART1_BASE_ADDR
#define UART_DEBUG_REGISTER(reg) UART_##reg##_OFFSET
#define UART_DEBUG_FIELD(reg, field) UART_##reg##_##field
#define UART_DEBUG_FIELD_MASK(reg, field) UART_##reg##_##field##_MASK
#define UART_DEBUG_FIELD_LSHIFT(reg, field) UART_##reg##_##field##_LSHIFT
#define UART_DEBUG_FIELD_ENUM(reg, field, enm) UART_##reg##_##field##_##enm

typedef struct {
	uint32_t namesz;
	uint32_t descsz;
	uint32_t type;
} ElfNoteSection_t;

MainLoop loopy;

static void MainCallsCallback() {
	for(uint32_t i = 0; i < osHeap->mainCallCallbacksIndex;++i) {
		osHeap->mainCallCallbacks[i]();
	}
	osHeap->mainCallCallbacksIndex = 0;
}

extern const ElfNoteSection_t g_note_build_id;
void PrintBanner(void)
{
	raw_debug_print(ANSI_YELLOW_PEN ANSI_BRIGHT_ON "IKUY PMU Monitor\n" ANSI_WHITE_PEN ANSI_BRIGHT_OFF);
	const uint8_t *build_id_data = &((uint8_t*)(&g_note_build_id)+1)[g_note_build_id.namesz];

	// print Build ID
	raw_debug_print("Build ID: ");
	for (uint32_t i = 0; i < g_note_build_id.descsz; ++i) raw_debug_printf("%02x", build_id_data[i]);
	raw_debug_print("\n");
}

void setupInterruptHandlers() {
	raw_debug_print("Set Up Interrupt Handlers\n");

	// install IPI0 as PMU sleep handlers (TODO seems a waste of an entire IPI...)
	raw_debug_print("  PMU sleep handler\n");
	HW_REG_WRITE1(IPI, PMU_0_ISR, IPI_PMU_0_ISR_USERMASK);
	HW_REG_WRITE1(IPI, PMU_0_IER, IPI_PMU_0_IER_USERMASK);
	Interrupts::SetHandler(Interrupts::Name::IN_IPI0, &IPI0_Handler);
	Interrupts::Enable(Interrupts::Name::IN_IPI0);

	// IPI3 OS service handler
	raw_debug_print("  PMU OS handler\n");
	HW_REG_WRITE1(IPI, PMU_3_ISR, IPI_PMU_3_ISR_USERMASK);
	HW_REG_WRITE1(IPI, PMU_3_IER, IPI_PMU_3_IER_USERMASK);
	Interrupts::SetHandler(Interrupts::Name::IN_IPI3, &IPI3_Handler);
	Interrupts::Enable(Interrupts::Name::IN_IPI3);

	// GPI0 (Fault tolerance events) (always enabled)
	raw_debug_print("  Fault Tolerance handler\n");
	Interrupts::SetHandler(Interrupts::Name::IN_GPI0, &GPI0_Handler);
	Interrupts::Enable(Interrupts::Name::IN_GPI0);

	// GPI1 (Wakeup events)
	raw_debug_print("  Wakeup handler\n");
	HW_REG_WRITE1(PMU_LOCAL, GPI1_ENABLE, PMU_LOCAL_GPI1_ENABLE_USERMASK);
	Interrupts::SetHandler(Interrupts::Name::IN_GPI1, &GPI1_Handler);
	Interrupts::Enable(Interrupts::Name::IN_GPI1);

	raw_debug_print("  GIC Proxy handler\n");
	// GPI1 is also used for GIC_PROXY
	// what UART are we using? set the interrupt based on that
#if UART_DEBUG_BASE_ADDR == 0xff010000
	HW_REG_WRITE1(LPD_SLCR, GICP0_IRQ_ENABLE, LPD_SLCR_GICP0_IRQ_ENABLE_SRC22);
#else
	HW_REG_WRITE1(LPD_SLCR, GICP0_IRQ_ENABLE, LPD_SLCR_GICP0_IRQ_ENABLE_SRC21);
#endif

	HW_REG_WRITE1(LPD_SLCR, GICP_PMU_IRQ_ENABLE, LPD_SLCR_GICP_PMU_IRQ_ENABLE_SRC0);
	HW_REG_WRITE(HW_REG_GET_ADDRESS(UART_DEBUG), UART, INTRPT_DIS, 	~UART_CHNL_INT_STS_RTRIG);
	HW_REG_WRITE(HW_REG_GET_ADDRESS(UART_DEBUG), UART, INTRPT_EN, 	UART_CHNL_INT_STS_RTRIG);
	HW_REG_WRITE(HW_REG_GET_ADDRESS(UART_DEBUG), UART, RCVR_TIMEOUT, 32);
	HW_REG_WRITE(HW_REG_GET_ADDRESS(UART_DEBUG), UART, RCVR_FIFO_TRIGGER_LEVEL, 1);

	// GPI2 (Reset and sleep events)
	raw_debug_print("  Reset and Sleep handler\n");
	HW_REG_WRITE1(PMU_LOCAL, GPI2_ENABLE, PMU_LOCAL_GPI2_ENABLE_USERMASK);
	Interrupts::SetHandler(Interrupts::Name::IN_GPI2, &GPI2_Handler);
	Interrupts::Enable(Interrupts::Name::IN_GPI2);

	// GPI3 (PL to PMU events)
	raw_debug_print("  PL to PMU handler\n");
	HW_REG_WRITE1(PMU_LOCAL, GPI3_ENABLE, PMU_LOCAL_GPI3_ENABLE_USERMASK);
	Interrupts::SetHandler(Interrupts::Name::IN_GPI3, &GPI3_Handler);
	Interrupts::Enable(Interrupts::Name::IN_GPI3);

	raw_debug_print("Set Up Interrupt Handlers Finished\n");
}



void main() __attribute__((noreturn));

void main()
{
	HW_REG_CLR_BIT1(PMU_GLOBAL, SAFETY_GATE, SCAN_ENABLE);

	// init disables interrupts and exceptions, start will enable them
	Interrupts::Init();

	HW_REG_CLR_BIT1(PMU_GLOBAL, GLOBAL_CNTRL, DONT_SLEEP);

	PrintBanner();

	// sleep all but A53 core 0 hard core processors in SoC
	A53Sleep1();
	A53Sleep2();
	A53Sleep3();
	R5FSleep();

	// use this chance to set the cache coherency broadcast (should be done during reset)
	HW_REG_SET_BIT( HW_REG_GET_ADDRESS(LPD_SLCR), LPD_SLCR, LPD_APU, BRDC_INNER);
	HW_REG_SET_BIT( HW_REG_GET_ADDRESS(LPD_SLCR), LPD_SLCR, LPD_APU, BRDC_OUTER);

	OsHeap::Init();
	loopy.Init();

	raw_debug_print("OS Server Init\n");
	IPI3_OsServer::Init();

	setupInterruptHandlers();

	raw_debug_print("Timer::Init\n");
	Timers::Init();

	raw_debug_print("Interrupt Start\n");
	Interrupts::Start();

	HW_REG_WRITE1(PMU_GLOBAL, GLOBAL_GEN_STORAGE0,
						 HW_REG_READ1(PMU_GLOBAL, GLOBAL_GEN_STORAGE0) | OS_GLOBAL0_PMU_READY);

	debug_print("IKUY PMU Firmware setup complete\n");
	Timers::Start();

	HW_REG_SET_BIT1(PMU_GLOBAL, GLOBAL_CNTRL, FW_IS_PRESENT);

	osHeap->hundredHzCallbacks[(int)HundredHzTasks::HOST_MAIN_CALLS] = &MainCallsCallback;

	loopy.Loop();

	while(true) {
	}

}



