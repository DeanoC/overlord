#include "core/core.h"
#include "gic_proxy.hpp"
#include "platform/memory_map.h"
#include "platform/reg_access.h"
#include "platform/registers/lpd_slcr.h"
#include "platform/registers/uart.h"
#include "platform/registers/ttc.h"
#include "dbg/raw_print.h"
#include "../os_heap.hpp"


uint32_t uartDebugTransmitLast;
uint32_t uartDebugTransmitHead;
uint32_t uartDebugReceiveLast;
uint32_t uartDebugReceiveHead;

#define UART_DEBUG_BASE_ADDR UART1_BASE_ADDR

#define IsTransmitFull() (HW_REG_GET_BIT(HW_REG_GET_ADDRESS(UART_DEBUG), UART, CHANNEL_STS, TNFUL))
#define IsReceiveEmpty() (HW_REG_GET_BIT(HW_REG_GET_ADDRESS(UART_DEBUG), UART, CHANNEL_STS, REMPTY))

static void UART_DEBUG_Interrupt() {
	uint32_t status = HW_REG_READ(HW_REG_GET_ADDRESS(UART_DEBUG), UART, CHNL_INT_STS) &
		HW_REG_READ(HW_REG_GET_ADDRESS(UART_DEBUG), UART, INTRPT_MASK);
	// disabled UART interrupts
	HW_REG_WRITE(HW_REG_GET_ADDRESS(UART_DEBUG), UART, INTRPT_DIS, status);
	HW_REG_WRITE(HW_REG_GET_ADDRESS(UART_DEBUG), UART, CHNL_INT_STS, status);

	// if the transmit fifo is empty, or new data has been submitted
	// push from the os heap buff until the fifo is full again
	if(status & UART_CHNL_INT_STS_TEMPTY || uartDebugTransmitLast != uartDebugTransmitHead) {
		while(!IsTransmitFull())
		{
			if(uartDebugTransmitLast != uartDebugTransmitHead) {
				HW_REG_WRITE(HW_REG_GET_ADDRESS(UART_DEBUG), UART, TX_RX_FIFO, osHeap->uartDEBUGTransmitBuffer[uartDebugTransmitLast]);
				// expand newline inline
				if(osHeap->uartDEBUGTransmitBuffer[uartDebugTransmitLast] == '\n') {
					osHeap->uartDEBUGTransmitBuffer[uartDebugTransmitLast] = '\r';
				} else {
					uartDebugTransmitLast = (uartDebugTransmitLast + 1 >= OsHeap::UartBufferSize) ? 0 : uartDebugTransmitLast + 1;
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
			uint32_t head = uartDebugReceiveHead + 1;
			uartDebugReceiveHead = (head >= OsHeap::UartBufferSize) ? 0 : head;
			osHeap->uartDEBUGReceiveBuffer[head - 1] = HW_REG_READ(HW_REG_GET_ADDRESS(UART_DEBUG), UART, TX_RX_FIFO);
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

	HW_REG_WRITE(HW_REG_GET_ADDRESS(UART_DEBUG), UART, INTRPT_EN, status); // enable interrupts
}

void GIC_Proxy(void) {
	const uint32_t maxRestartCount = 10;
	uint32_t restartCount = 0;

restart:;
	const uint32_t gicpIrqStatus = HW_REG_READ1(LPD_SLCR, GICP_PMU_IRQ_STATUS);

	for(uint32_t group = 0; group < 5; group++) {
		const uint32_t bit = 1 << group;
		if(bit & gicpIrqStatus) {
			const volatile uint32_t gicbits =
					hw_RegRead(LPD_SLCR_BASE_ADDR,LPD_SLCR_GICP0_IRQ_STATUS_OFFSET + (group * GICPROXY_INTERRUPT_GROUP_SIZE));

//			raw_debug_printf("group 0x%lx gicbits 0x%lx\n", group, gicbits);

			const Interrupt_Names name = GIC_ProxyToName(group, gicbits);
			switch(name) {
#if UART_DEBUG_BASE_ADDR == 0xff010000
				case INT_UART1: UART_DEBUG_Interrupt(); break;
#else
				case INT_UART0: UART_DEBUG_Interrupt(); break;
#endif
				default:
					break;
			}
			hw_RegWrite(LPD_SLCR_BASE_ADDR, LPD_SLCR_GICP0_IRQ_STATUS_OFFSET + (group * GICPROXY_INTERRUPT_GROUP_SIZE), gicbits);
			HW_REG_WRITE1(LPD_SLCR, GICP_PMU_IRQ_STATUS, bit);

			if(HW_REG_READ1(LPD_SLCR, GICP_PMU_IRQ_STATUS) & bit) {
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
