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


extern uint32_t uart0ReceiveLast;
extern uint32_t uart0ReceiveHead;
extern "C" {

uintptr_t tmpBufferAddr = 0;
uint32_t tmpBufferSize = 0;
uint32_t tmpBufferIndex = 0;

static void tmpBufferRefill() {
	assert(tmpBufferSize == 0);

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
		tmpBufferIndex = 0;
	}
}

uint8_t readNextByte() {
reget:
	if (tmpBufferSize != 0) {
		auto const startOfBuffer = (uint8_t *) tmpBufferAddr;
		uint8_t b = startOfBuffer[tmpBufferIndex++];
		if (tmpBufferIndex == tmpBufferSize) {
			osHeap->tmpOsBufferAllocator.Free(tmpBufferAddr);
			tmpBufferSize = 0;
			tmpBufferAddr = 0;
		}
		return b;
	}
	tmpBufferRefill();
	goto reget;
}

void writeByte(uint8_t c) {
	IPI3_OsServer::PutByte(c);
}


}

void HostInterface::Loop() {
	OsService_InlinePrint(OSS_INLINE_TEXT("HostInterface Start\n"));

	this->currentState = State::RECEIVING_COMMAND;
	static const int CMD_BUF_SIZE = 256;
	uint8_t cmdBuffer[CMD_BUF_SIZE];
	uint32_t cmdBufferHead = 0;
	uint32_t cmdBufferHeadTmp;

	ZModem zModem;

	while (true) {
WaitForUart:
		uint8_t b = readNextByte();

		switch (this->currentState) {
			case State::RECEIVING_COMMAND: {
				switch (b) {
					case ASCII_BACKSPACE: // backspace
						cmdBufferHead--;
						debug_print("\b");
						goto WaitForUart;
					case ASCII_EOT: // Control C should send this...
						cmdBufferHead = 0;
						goto WaitForUart;

					case ASCII_LF:
						goto WaitForUart;

					case ASCII_CR:
						this->currentState = State::PROCESSING_COMMAND;
						goto ProcessCommand;

					default: // normal keys echo
						debug_printf("%c", b);
						if (cmdBufferHead >= CMD_BUF_SIZE - 1) {
							debug_print("\nCommand too long\n");
							cmdBufferHead = 0;
							goto WaitForUart;
						}
						cmdBuffer[cmdBufferHead++] = b;
						goto WaitForUart;
				}
			}

			case State::PROCESSING_COMMAND: {
				ProcessCommand:
				static const unsigned int MaxFinds = 16;
				unsigned int finds[MaxFinds];
				cmdBufferHeadTmp = cmdBufferHead;
				cmdBuffer[cmdBufferHeadTmp] = 0;
				cmdBufferHead = 0;
				finds[0] = cmdBufferHeadTmp;

				const auto findCount = Utils::StringFindMultiple(cmdBufferHeadTmp, (char *) cmdBuffer, ' ', MaxFinds, finds);
				if (findCount != Utils::StringNotFound) {
					Utils::StringScatterChar(cmdBufferHeadTmp, (char *) cmdBuffer, findCount, finds, 0);
				}

				switch (Utils::RuntimeHash(finds[0], (char *) cmdBuffer)) {
					case "echo"_hash: {
						EchoCmd(cmdBuffer, finds, findCount);
						break;
					}
					case "download"_hash: {
						DownloadCmd(cmdBuffer, cmdBufferHeadTmp, finds, findCount);
						break;
					}
					case "zmodem"_hash: { // zmodem download start
						//						this->currentState = State::RECEIVING_ZMODEM_IDLE;
						FrameType ft = zModem.tryz();
						if(ft == FrameType::ZCOMPL) {
							this->currentState = State::RECEIVING_COMMAND;
						} else {
							zModem.rzfiles();
							this->currentState = State::RECEIVING_COMMAND;
						}
						break;
					}
					default: {
						debug_printf(ANSI_MAGENTA_PEN "Unknown Command %s\n" ANSI_RESET_ATTRIBUTES, cmdBuffer);
						this->currentState = State::RECEIVING_COMMAND;
						break;
					}
				}
				goto WaitForUart;
			}

			case State::POST_RECEIVE_DATA: {
				PostReceiveData:
				PostDownload();
				this->currentState = State::RECEIVING_COMMAND;
				break;
			}
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
