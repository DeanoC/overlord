#include "core/core.h"
#include "gic_proxy.hpp"
#include "hw/memory_map.h"
#include "hw/reg_access.h"
#include "hw_regs/lpd_slcr.h"
#include "hw_regs/uart.h"
#include "hw_regs/ttc.h"
#include "dbg/raw_print.h"
#include "os_heap.hpp"

#define IsTransmitFull() (HW_REG_GET_BIT(UART0, CHANNEL_STS, TNFUL))
#define IsReceiveEmpty() (HW_REG_GET_BIT(UART0, CHANNEL_STS, REMPTY))

uint32_t uart0TransmitLast;
uint32_t uart0TransmitHead;
uint32_t uart0ReceiveLast;
uint32_t uart0ReceiveHead;

static void UART0_Interrupt() {
	uint32_t status = HW_REG_GET(UART0, CHNL_INT_STS) & HW_REG_GET(UART0, INTRPT_MASK);
	// disabled UART interupts
	HW_REG_SET(UART0, INTRPT_DIS, status);
	HW_REG_SET(UART0, CHNL_INT_STS, status);

	// if the transmit fifo is empty, or new data has been submitted
	// push from the os heap buff until the fifo is full again
	if(status & UART_CHNL_INT_STS_TEMPTY || uart0TransmitLast != uart0TransmitHead) {
		while(!IsTransmitFull())
		{
			if(uart0TransmitLast != uart0TransmitHead) {
				HW_REG_SET(UART0, TX_RX_FIFO, osHeap->uart0TransmitBuffer[uart0TransmitLast]);
				// expand newline inline
				if(osHeap->uart0TransmitBuffer[uart0TransmitLast] == '\n') {
					osHeap->uart0TransmitBuffer[uart0TransmitLast] = '\r';
				} else {
					uart0TransmitLast = (uart0TransmitLast + 1 >= OsHeap::UartBufferSize) ? 0 : uart0TransmitLast + 1;
				}
			} else {
				status &= ~UART_INTRPT_DIS_TEMPTY;
				break;
			}
		}
	}

	if(status & UART_CHNL_INT_STS_RFUL || status & UART_CHNL_INT_STS_TIMEOUT) {
		// receive FIFO is full, copy fifo out to buffer
		while(!IsReceiveEmpty()) {
			uint32_t head = uart0ReceiveHead + 1;
			uart0ReceiveHead = (head >= OsHeap::UartBufferSize) ? 0 : head;
			osHeap->uart0ReceiveBuffer[head - 1] = HW_REG_GET(UART0, TX_RX_FIFO);
		}
	}
	if(status & UART_CHNL_INT_STS_REMPTY) {
		status |= UART_INTRPT_DIS_RTRIG;
		status &= ~(UART_INTRPT_EN_TIMEOUT | UART_CHNL_INT_STS_RFUL | UART_CHNL_INT_STS_REMPTY);
	}

	if(status & UART_CHNL_INT_STS_RTRIG) {
		status |= (UART_INTRPT_EN_TIMEOUT | UART_CHNL_INT_STS_RFUL | UART_CHNL_INT_STS_REMPTY);
		status &= ~UART_INTRPT_DIS_RTRIG;
	}

	HW_REG_SET(UART0, INTRPT_EN, status); // enable interrupts
}

static void TTC0_1_Interrupt() {
//	raw_debug_print("TTC\n");
	// gcc warns that the get isn't used BUT in this case its a clear on
	// read register, so turn the warning off.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
	auto dummy = HW_REG_GET(TTC0, INTERRUPT_REGISTER_1); // clr on *read*
#pragma GCC diagnostic pop
}

void GIC_Proxy(void) {
	const uint32_t maxRestartCount = 10;
	uint32_t restartCount = 0;

restart:;
	const uint32_t gicpIrqStatus = HW_REG_GET(LPD_SLCR, GICP_PMU_IRQ_STATUS);


	for(uint32_t group = 0; group < 5; group++) {
		const uint32_t bit = 1 << group;
		if(bit & gicpIrqStatus) {
			const volatile uint32_t gicbits =
					hw_RegRead(LPD_SLCR_BASE_ADDR,LPD_SLCR_GICP0_IRQ_STATUS_OFFSET + (group * GICPROXY_INTERRUPT_GROUP_SIZE));

//			raw_debug_printf("group 0x%lx gicbits 0x%lx\n", group, gicbits);

			const GICInterrupt_Names name = GIC_ProxyToName(group, gicbits);
			switch(name) {
				case GICIN_TTC0_1: TTC0_1_Interrupt(); break;
				case GICIN_UART0: UART0_Interrupt(); break;
				default:
					break;
			}
			hw_RegWrite(LPD_SLCR_BASE_ADDR, LPD_SLCR_GICP0_IRQ_STATUS_OFFSET + (group * GICPROXY_INTERRUPT_GROUP_SIZE), gicbits);
			HW_REG_SET(LPD_SLCR, GICP_PMU_IRQ_STATUS, bit);

			if(HW_REG_GET(LPD_SLCR, GICP_PMU_IRQ_STATUS) & bit) {
				if(restartCount < maxRestartCount) {
					restartCount++;
					goto restart;
				} else {
					raw_debug_printf("GIC %d is running too fast, can't keep up!\n", name);
				}
			}
		}
	}


}


