#include "core/core.h"
#include "core/cpp/compile_time_hash.hpp"
#include "host_interface.hpp"
#include "osservices/osservices.h"
#include "os_heap.hpp"
#include "utils/string_utils.hpp"
#include "dbg/ansi_escapes.h"
#include "zynqps8/dma/lpddma.hpp"
#include "rom_extensions.h"
#include "hw/reg_access.h"
#include "hw/memory_map.h"
#include "hw_regs/apu.h"
#include "hw_regs/crl_apb.h"
#include "hw_regs/crf_apb.h"
#include "hw_regs/uart.h"
#include "ipi3_os_server.hpp"
#include "zmodem.hpp"

#define SIZED_TEXT(text) sizeof(text), (uint8_t const*)text
extern uint8_t textConsoleSkip;

extern uint32_t uart0ReceiveLast;
extern uint32_t uart0ReceiveHead;
static void HostInputCallback() {
	HostInterface* host = &osHeap->hostInterface;
	assert(host);
	host->InputCallback();
}
static void HostCommandCallback() {
	HostInterface* host = &osHeap->hostInterface;
	assert(host);
	host->CommandCallback();
}

void HostInterface::Init() {
	this->currentState = State::RECEIVING_COMMAND;
	this->cmdBuffer = (uint8_t*) osHeap->tmpOsBufferAllocator.Alloc(CMD_BUF_SIZE/64);
	this->cmdBufferHead = 0;
	osHeap->hundredHzCallbacks[(int)HundredHzTasks::HOST_INPUT] = &HostInputCallback;
	osHeap->hundredHzCallbacks[(int)HundredHzTasks::HOST_COMMANDS_PROCESSING] = &HostCommandCallback;
	zModem.Init();
}

[[maybe_unused]] void HostInterface::Fini() {
	zModem.Fini();
	osHeap->tmpOsBufferAllocator.Free((uintptr_t)this->cmdBuffer);
}

void HostInterface::TmpBufferRefill(uintptr_t& tmpBufferAddr, uint32_t& tmpBufferSize) {
	if (uart0ReceiveLast != uart0ReceiveHead) {
		uint32_t last = uart0ReceiveLast;
		uint32_t head = uart0ReceiveHead;

		if (last > head) {
			// wrapped
			auto firstSize = OsHeap::UartBufferSize - last;
			tmpBufferSize = firstSize + head;
			assert(BitOp::PowerOfTwoContaining(tmpBufferSize / 64) < 128);
			tmpBufferAddr = osHeap->tmpOsBufferAllocator.Alloc(BitOp::PowerOfTwoContaining(tmpBufferSize / 64));
			memcpy((char *) tmpBufferAddr, &osHeap->uart0ReceiveBuffer[last], firstSize);
			memcpy((char *) tmpBufferAddr + firstSize, &osHeap->uart0ReceiveBuffer[0], head);
		} else {
			tmpBufferSize = head - last;
			tmpBufferAddr = osHeap->tmpOsBufferAllocator.Alloc(BitOp::PowerOfTwoContaining(tmpBufferSize / 64));
			memcpy((char *) tmpBufferAddr, &osHeap->uart0ReceiveBuffer[last], tmpBufferSize);
		}
		uart0ReceiveLast = head;
	}
}

void HostInterface::InputCallback() {
	if(this->currentState != State::RECEIVING_COMMAND)
		return;

	uintptr_t tmpBufferAddr = 0;
	uint32_t tmpBufferSize = 0;
	uint32_t tmpBufferIndex = 0;

	TmpBufferRefill(tmpBufferAddr, tmpBufferSize);

	while(true) {
		uint8_t c;
		if (tmpBufferSize != 0) {
			auto const startOfBuffer = (uint8_t *) tmpBufferAddr;
			if (tmpBufferIndex == tmpBufferSize) {
				osHeap->tmpOsBufferAllocator.Free(tmpBufferAddr);
				tmpBufferSize = 0;
				tmpBufferAddr = 0;
				return;
			}
			c = startOfBuffer[tmpBufferIndex++];
		} else {
			return;
		}

		switch (c) {
			case ASCII_BACKSPACE: // backspace
				this->cmdBufferHead--;
				IPI3_OsServer::PutByte('\b');
				continue;
			case ASCII_EOT: // Control C should send this...
				this->cmdBufferHead = 0;
				continue;
			case ASCII_LF: continue;
			case ASCII_CR: this->currentState = State::PROCESSING_COMMAND;
				continue;
			default:
				// normal keys
				// echo
				IPI3_OsServer::PutByte(c);
				if (this->cmdBufferHead >= CMD_BUF_SIZE - 1) {
					debug_print("\nCommand too long\n");
					this->cmdBufferHead = 0;
					continue;
				}
				this->cmdBuffer[this->cmdBufferHead++] = c;
				continue;
		}
	}

}
void HostInterface::CommandCallback() {
	switch (this->currentState) {
		case State::PROCESSING_COMMAND:
			this->WhatCommand();
			return;
		case State::ZMODEM: {
			ZModem::Result result = zModem.Receive();
			switch (result) {
				case ZModem::Result::FAIL:
					osHeap->console.console.NewLine();
					osHeap->console.console.PrintLn(ANSI_RED_PEN ANSI_BRIGHT "ZModem FAIL" ANSI_RESET_ATTRIBUTES);
					this->currentState = State::RECEIVING_COMMAND;
					textConsoleSkip = 0;
					return;
				case ZModem::Result::CONTINUE:
					return;
				case ZModem::Result::SUCCESS:
					osHeap->console.console.NewLine();
					osHeap->console.console.PrintLn(ANSI_GREEN_PEN ANSI_BRIGHT "ZModem SUCCESS" ANSI_RESET_ATTRIBUTES);
					this->currentState = State::RECEIVING_COMMAND;
					textConsoleSkip = 0;
					return;
			}
		}
		default:
			return;
	}
}

void HostInterface::WhatCommand() {
	static const unsigned int MaxFinds = 16;
	unsigned int finds[MaxFinds];
	uint32_t cmdBufferHeadTmp = this->cmdBufferHead;
	this->cmdBuffer[cmdBufferHeadTmp] = 0;
	this->cmdBufferHead = 0;
	finds[0] = cmdBufferHeadTmp;

	// special case Zmodem receive code first
	if(cmdBufferHeadTmp > 4 && this->cmdBuffer[4] == 0x18) {
		if(Utils::RuntimeHash(4, (char *) this->cmdBuffer) == "rz**"_hash) {
			// zmodem download start
			osHeap->console.console.PrintLn("ZModem download started");
			textConsoleSkip = 30;
			this->zModem.ReInit();
			this->currentState = State::ZMODEM;
			return;
		}
	}

	const auto findCount = Utils::StringFindMultiple(cmdBufferHeadTmp, (char *) this->cmdBuffer, ' ', MaxFinds, finds);
	if (findCount != Utils::StringNotFound) {
		Utils::StringScatterChar(cmdBufferHeadTmp, (char *) this->cmdBuffer, findCount, finds, 0);
	}
	osHeap->console.console.PrintWithSize(finds[0], (char *) this->cmdBuffer);
	osHeap->console.console.PrintLn(" command received");

	switch (Utils::RuntimeHash(finds[0], (char *) this->cmdBuffer)) {
		case "echo"_hash: {
			EchoCmd(cmdBuffer, finds, findCount);
			break;
		}
		default: {
			debug_printf(ANSI_MAGENTA_PEN "Unknown Command %s\n" ANSI_RESET_ATTRIBUTES, cmdBuffer);
			this->currentState = State::RECEIVING_COMMAND;
			break;
		}
	}
}

void HostInterface::EchoCmd(uint8_t const *cmdBuffer, unsigned int const *finds, unsigned int const findCount) {
	if (findCount != 2) {
		debug_print("\nARG ERROR: echo arg\n");
		this->currentState = State::RECEIVING_COMMAND;
		return;
	}
	assert(findCount == 2);
	debug_print("\n");
	OsService_PrintWithSize(finds[1] - finds[0] - 1, (char const *) (cmdBuffer + finds[0] + 1));
	debug_print("\n");
	this->currentState = State::RECEIVING_COMMAND;
}

#if 0

static void A53Sleep() {
	RomServiceTable[REN_ACPU0SLEEP]();
	RomServiceTable[REN_ACPU1SLEEP]();
	RomServiceTable[REN_ACPU2SLEEP]();
	RomServiceTable[REN_ACPU3SLEEP]();
}
static void R5FSleep() {
	RomServiceTable[REN_R50SLEEP]();
	RomServiceTable[REN_R51SLEEP]();
}

static void A53WakeUp() {
	RomServiceTable[REN_ACPU0WAKE]();
	RomServiceTable[REN_ACPU1WAKE]();
	RomServiceTable[REN_ACPU2WAKE]();
	RomServiceTable[REN_ACPU3WAKE]();
}

static void R5FWakeUp() {
	RomServiceTable[REN_R50WAKE]();
	RomServiceTable[REN_R51WAKE]();
}

void HostInterface::DownloadCmd(uint8_t *cmdBuffer,
																uint32_t cmdBufferHead,
																unsigned int const *finds,
																unsigned int const findCount) {
	if (findCount != 3) {
		debug_print("ARG ERROR: download [A53|R5F|DATA] address\n");
		this->currentState = State::RECEIVING_COMMAND;
		return;
	}
	uint64_t address = Utils::DecimalStringToU64(finds[2] - finds[1] - 1, (char const *) (cmdBuffer + finds[1] + 1));
	debug_printf("Download address 0x%llx\n", address);

	switch (Utils::RuntimeHash(finds[1] - finds[0] - 1, (char *) cmdBuffer + finds[0] + 1)) {
		case "A53"_hash: {
			currentDownloadTarget = DownloadTarget::A53;
			A53Sleep();
			debug_print("DownloadTarget::A53\n");
			break;
		}
		case "R5F"_hash: {
			currentDownloadTarget = DownloadTarget::R5F;
			R5FSleep();
			debug_print("DownloadTarget::R5F\n");
			break;
		}
		case "DATA"_hash: {
			currentDownloadTarget = DownloadTarget::DATA;
			debug_print("DownloadTarget::DATA\n");
			break;
		}
		default: cmdBuffer[cmdBufferHead] = 0;
			debug_printf(ANSI_MAGENTA_PEN "Unknown download target %s\n" ANSI_RESET_ATTRIBUTES, cmdBuffer);
			this->currentState = State::RECEIVING_COMMAND;
			return;
	}

	this->downloadAddress = this->currentDownloadAddress = address;
	this->currentState = State::RECEIVING_COMMAND;
}

void HostInterface::PostDownload() {
	debug_print("PostDownload\n");

	switch (currentDownloadTarget) {
		case DownloadTarget::A53: {
			auto const lowAddress = (uint32_t) (downloadAddress & 0x0000'0000'FFFF'FFFFull);
			auto const hiAddress = (uint32_t) ((downloadAddress & 0xFFFF'FFFF'0000'0000ull) >> 32ull);
			HW_REG_MERGE_FIELD(APU, RVBARADDR0L, ADDR, lowAddress);
			HW_REG_MERGE_FIELD(APU, RVBARADDR0H, ADDR, hiAddress);
			/*		HW_REG_MERGE_FIELD(APU, RVBARADDR1L, ADDR, lowAddress);
					HW_REG_MERGE_FIELD(APU, RVBARADDR1H, ADDR, hiAddress);
					HW_REG_MERGE_FIELD(APU, RVBARADDR2L, ADDR, lowAddress);
					HW_REG_MERGE_FIELD(APU, RVBARADDR2H, ADDR, hiAddress);
					HW_REG_MERGE_FIELD(APU, RVBARADDR3L, ADDR, lowAddress);
					HW_REG_MERGE_FIELD(APU, RVBARADDR3H, ADDR, hiAddress);
		*/
			// put apu in reset
			HW_REG_SET(CRF_APB, RST_FPD_APU,
								 HW_REG_ENCODE_FIELD(CRF_APB, RST_FPD_APU, APU_L2_RESET, 1) |
										 HW_REG_ENCODE_FIELD(CRF_APB, RST_FPD_APU, ACPU0_RESET, 1) |
										 HW_REG_ENCODE_FIELD(CRF_APB, RST_FPD_APU, ACPU1_RESET, 1) |
										 HW_REG_ENCODE_FIELD(CRF_APB, RST_FPD_APU, ACPU2_RESET, 1) |
										 HW_REG_ENCODE_FIELD(CRF_APB, RST_FPD_APU, ACPU3_RESET, 1));

			// wakey wakey rise and shine
			A53WakeUp();

			// take them out
			HW_REG_SET(CRF_APB, RST_FPD_APU,
								 HW_REG_ENCODE_FIELD(CRF_APB, RST_FPD_APU, APU_L2_RESET, 0) |
										 HW_REG_ENCODE_FIELD(CRF_APB, RST_FPD_APU, ACPU0_RESET, 0) |
										 HW_REG_ENCODE_FIELD(CRF_APB, RST_FPD_APU, ACPU1_RESET, 0) |
										 HW_REG_ENCODE_FIELD(CRF_APB, RST_FPD_APU, ACPU2_RESET, 0) |
										 HW_REG_ENCODE_FIELD(CRF_APB, RST_FPD_APU, ACPU3_RESET, 0));

		}
			break;
		case DownloadTarget::R5F:
			// wakey wakey rise and shine
			R5FWakeUp();
			break;
		case DownloadTarget::DATA: break;
	}
}
#endif