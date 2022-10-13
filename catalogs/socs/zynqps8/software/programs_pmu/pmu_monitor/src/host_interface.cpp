#include "core/core.h"
#include "core/compile_time_hash.hpp"
#include "host_interface.hpp"
#include "osservices/osservices.h"
#include "os_heap.hpp"
#include "utils/string_utils.hpp"
#include "dbg/ansi_escapes.h"
#include "platform/reg_access.h"
#include "platform/memory_map.h"
#include "platform/registers/apu.h"
#include "platform/registers/crf_apb.h"
#include "os/ipi3_os_server.hpp"
#include "cpuwake.hpp"
#include "zynqps8/dma/lpddma.hpp"

#define SIZED_TEXT(text) sizeof(text), (uint8_t const*)text
extern uint8_t textConsoleSkip;

extern uint32_t uartDebugReceiveLast;
extern uint32_t uartDebugReceiveHead;
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
	this->downloadAddress = 0x8'0000'0000;
	this->lastReadAddress = 0;
	this->lastCommandWasR = false;

	osHeap->hundredHzCallbacks[(int)HundredHzTasks::HOST_INPUT] = &HostInputCallback;
	osHeap->hundredHzCallbacks[(int)HundredHzTasks::HOST_COMMANDS_PROCESSING] = &HostCommandCallback;

}

[[maybe_unused]] void HostInterface::Fini() {
	osHeap->tmpOsBufferAllocator.Free((uintptr_t)this->cmdBuffer, CMD_BUF_SIZE/64 );
}

void HostInterface::TmpBufferRefill(uintptr_t& tmpBufferAddr, uint32_t& tmpBufferSize) {
	if (uartDebugReceiveLast != uartDebugReceiveHead) {
		uint32_t last = uartDebugReceiveLast;
		uint32_t head = uartDebugReceiveHead;

		if (last > head) {
			// wrapped
			auto firstSize = OsHeap::UartBufferSize - last;
			tmpBufferSize = firstSize + head;
			tmpBufferAddr = osHeap->tmpOsBufferAllocator.Alloc(BitOp::PowerOfTwoContaining(tmpBufferSize / 64));
			memcpy((char *) tmpBufferAddr, &osHeap->uartDEBUGReceiveBuffer[last], firstSize);
			memcpy((char *) tmpBufferAddr + firstSize, &osHeap->uartDEBUGReceiveBuffer[0], head);
		} else {
			tmpBufferSize = head - last;
			tmpBufferAddr = osHeap->tmpOsBufferAllocator.Alloc(BitOp::PowerOfTwoContaining(tmpBufferSize / 64));
			memcpy((char *) tmpBufferAddr, &osHeap->uartDEBUGReceiveBuffer[last], tmpBufferSize);
		}
		uartDebugReceiveLast = head;
	} else {
		tmpBufferAddr = 0;
		tmpBufferSize = 0;
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
				osHeap->tmpOsBufferAllocator.Free(tmpBufferAddr, BitOp::PowerOfTwoContaining(tmpBufferSize / 64));
				tmpBufferSize = 0;
				tmpBufferAddr = 0;
				return;
			}
			c = startOfBuffer[tmpBufferIndex++];
		} else {
			return;
		}
#define ASCII_EOT 3
#define ASCII_BACKSPACE 8
#define ASCII_LF 10
#define ASCII_CR 13
#define ASCII_CAN 24
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

	if(*this->cmdBuffer == 0) {
		if(this->lastCommandWasR == true) {
			Read16BCmd( nullptr, nullptr, Utils::StringNotFound);
		}
		this->currentState = State::RECEIVING_COMMAND;
		return;
	}

	this->lastCommandWasR = false;

	const auto findCount = Utils::StringFindMultiple(cmdBufferHeadTmp, (char *) this->cmdBuffer, ' ', MaxFinds, finds);
	if (findCount != Utils::StringNotFound) {
		Utils::StringScatterChar(cmdBufferHeadTmp, (char *) this->cmdBuffer, findCount, finds, 0);
	}

	switch (Core::RuntimeHash(finds[0], (char *) this->cmdBuffer)) {
		case "echo"_hash: {
			EchoCmd(cmdBuffer, finds, findCount);
			break;
		}
		case "read4b"_hash:
		case "read4B"_hash: {
			Read4BCmd(cmdBuffer, finds, findCount);
			break;
		}
		case "R"_hash:
		case "r"_hash:
		case "read"_hash:
		case "read16b"_hash:
		case "read16B"_hash: {
			Read16BCmd(cmdBuffer, finds, findCount);
			break;
		}
		case "download_at"_hash: {
			DownloadAt(cmdBuffer, finds, findCount);
			break;
		}
		case "sleep_cpu"_hash: {
			SleepCpu(cmdBuffer, finds, findCount);
			break;
		}
		case "wake_cpu"_hash: {
			WakeUpCpu(cmdBuffer, finds, findCount);
			break;
		}
		case "boot_cpu"_hash: {
			BootCpu(cmdBuffer, finds, findCount);
			break;
		}
		case "reset"_hash: {
			Reset(cmdBuffer, finds, findCount);
			break;
		}
		default: {
			bool hasPrintable = false;
			uint8_t const * ptr  = this->cmdBuffer;
			while(*ptr) {
				uint8_t const c = *ptr++;
				if(c > ' ' && c <= 127) {
					hasPrintable = true;
					break;
				}
			}

			if(hasPrintable) {
				debug_printf(ANSI_MAGENTA_PEN "\nUnknown Command %s\n" ANSI_RESET_ATTRIBUTES, cmdBuffer);
			}
			this->currentState = State::RECEIVING_COMMAND;
			break;
		}
	}
}

void HostInterface::EchoCmd(uint8_t const *cmdBuffer, unsigned int const *finds, unsigned int const findCount) {
	if (findCount != 2) {
		debug_print(ANSI_RED_PAPER ANSI_BRIGHT_ON "\nARG ERROR: echo arg\n" ANSI_RESET_ATTRIBUTES);
		this->currentState = State::RECEIVING_COMMAND;
		return;
	}
	assert(findCount == 2);
	debug_print("\n");
	OsService_PrintWithSize(finds[1] - finds[0] - 1, (char const *) (cmdBuffer + finds[0] + 1));
	debug_print("\n");
	this->currentState = State::RECEIVING_COMMAND;
}

bool HostInterface::DecodeAddress(uint8_t const *cmdBuffer, unsigned int const *finds, unsigned int const findCount) {
	uint32_t address = Utils::StringToU32(finds[1] - finds[0] - 1, (char const *) (cmdBuffer + finds[0] + 1)) & ~ 0x3;
	if(address == 0xDEAD0CF1U) {
		OsService_Print( ANSI_RED_PAPER ANSI_BRIGHT_ON "lo_address overflow" ANSI_RESET_ATTRIBUTES "\n" );
		return false;
	}
	else {
		this->lastReadAddress = address;
		return true;
	}
}

void HostInterface::Read4BCmd(uint8_t const *cmdBuffer, unsigned int const *finds, unsigned int const findCount) {
	if (findCount > 2 && findCount != Utils::StringNotFound) {
		debug_print(ANSI_RED_PAPER ANSI_BRIGHT_ON "\nARG ERROR: read4b [lo_address]" ANSI_RESET_ATTRIBUTES  "\n");
		this->currentState = State::RECEIVING_COMMAND;
		return;
	}
	debug_print( "\n" );
	if(findCount == 2) DecodeAddress( cmdBuffer, finds, findCount );
	else {
		OsService_Printf( ANSI_CURSOR_UP );
		this->lastReadAddress += 4;
	}

	OsService_Printf( ANSI_WHITE_PEN ANSI_BRIGHT_ON "%#010llx %#010lx" ANSI_RESET_ATTRIBUTES "\n", this->lastReadAddress, *((uintptr_lo_t *) this->lastReadAddress));

	this->currentState = State::RECEIVING_COMMAND;
}

void HostInterface::Read16BCmd(uint8_t const *cmdBuffer, unsigned int const *finds, unsigned int const findCount) {
	if (findCount > 2 && findCount != Utils::StringNotFound) {
		debug_print(ANSI_RED_PAPER ANSI_BRIGHT_ON "\nARG ERROR: read16b lo_address\n" ANSI_RESET_ATTRIBUTES);
		this->currentState = State::RECEIVING_COMMAND;
		return;
	}
	debug_print( "\n" );
	if(findCount == 2) DecodeAddress( cmdBuffer, finds, findCount );
	else {
		OsService_Printf( ANSI_CURSOR_UP );
		this->lastReadAddress += 16;
		this->lastCommandWasR = true;
	}

	uintptr_lo_t* address = (uintptr_lo_t*) this->lastReadAddress;
	OsService_Printf(ANSI_WHITE_PEN ANSI_BRIGHT_ON "%#010lx: ", (uintptr_lo_t) address);
	OsService_Printf("%#010lx %#010lx %#010lx %#010lx\n" ANSI_RESET_ATTRIBUTES, *address, *(address+1), *(address+2), *(address+3));

	this->currentState = State::RECEIVING_COMMAND;
}


void HostInterface::DownloadAt(uint8_t const *cmdBuffer, unsigned int const *finds, unsigned int const findCount) {
	if (findCount != 2) {
		debug_print(ANSI_RED_PAPER ANSI_BRIGHT_ON "\nARG ERROR: download_at address\n" ANSI_RESET_ATTRIBUTES);
		this->currentState = State::RECEIVING_COMMAND;
		return;
	}
	assert(findCount == 2);
	uint64_t address = Utils::StringToU64(finds[1] - finds[0] - 1, (char const *) (cmdBuffer + finds[0] + 1));
	debug_printf("\nDownload address 0x%llx\n", address);

	this->downloadAddress = address;
	this->currentState = State::RECEIVING_COMMAND;
}

void HostInterface::SleepCpu(uint8_t const *cmdBuffer, unsigned int const *finds, unsigned int const findCount) {
	if (findCount != 2) {
		debug_print(ANSI_RED_PAPER ANSI_BRIGHT_ON "\nARG ERROR: sleep_cpu [A53|R5F]\n");
		this->currentState = State::RECEIVING_COMMAND;
		return;
	}
	switch (Core::RuntimeHash(finds[1] - finds[0] - 1, (char *) cmdBuffer + finds[0] + 1)) {
		case "A53"_hash: {
			A53Sleep0();
			A53Sleep1();
			A53Sleep2();
			A53Sleep3();
			debug_print("\nA53s going to sleep\n");
			break;
		}
		case "R5F"_hash: {
			R5FSleep0();
			R5FSleep1();
			debug_print("\nR5Fs going to sleep\n");
			break;
		}
		default:
			debug_printf(ANSI_RED_PAPER ANSI_BRIGHT_ON "\nUnknown CPU target\n" ANSI_RESET_ATTRIBUTES);
			this->currentState = State::RECEIVING_COMMAND;
			return;
	}
}

void HostInterface::WakeUpCpu(uint8_t const *cmdBuffer, unsigned int const *finds, unsigned int const findCount) {
	if (findCount != 2) {
		debug_print(ANSI_RED_PAPER ANSI_BRIGHT_ON "\nARG ERROR: wake_cpu [A53|R5F]\n" ANSI_RESET_ATTRIBUTES);
		this->currentState = State::RECEIVING_COMMAND;
		return;
	}
	switch (Core::RuntimeHash(finds[1] - finds[0] - 1, (char *) cmdBuffer + finds[0] + 1)) {
		case "A53"_hash: {
			A53WakeUp0();
			A53WakeUp1();
			A53WakeUp2();
			A53WakeUp3();
			debug_print("\nA53s waking up\n");
			break;
		}
		case "R5F"_hash: {
			R5FWakeUp0();
			R5FWakeUp1();
			debug_print("\nR5Fs waking up\n");
			break;
		}
		default:
			debug_printf(ANSI_RED_PAPER ANSI_BRIGHT_ON "\nUnknown CPU target\n" ANSI_RESET_ATTRIBUTES);
			this->currentState = State::RECEIVING_COMMAND;
			return;
	}
}

void HostInterface::BootCpu(uint8_t const *cmdBuffer, unsigned int const *finds, unsigned int const findCount) {
	if (findCount != 2) {
		debug_print(ANSI_RED_PAPER ANSI_BRIGHT_ON "\nARG ERROR: boot_cpu [A53|R5F]\n" ANSI_RESET_ATTRIBUTES);
		this->currentState = State::RECEIVING_COMMAND;
		return;
	}
	switch (Core::RuntimeHash(finds[1] - finds[0] - 1, (char *) cmdBuffer + finds[0] + 1)) {
		case "A53"_hash: {
			debug_printf("\nA53s booting from %#018llx\n", downloadAddress);
			A53Sleep0();
			A53Sleep1();
			A53Sleep2();
			A53Sleep3();
			auto const lowAddress = (uint32_t) (downloadAddress & 0x0000'0000'FFFF'FFFFull);
			auto const hiAddress = (uint32_t) ((downloadAddress & 0xFFFF'FFFF'0000'0000ull) >> 32ull);
			HW_REG_WRITE1(APU, RVBARADDR0L, lowAddress);
			HW_REG_WRITE1(APU, RVBARADDR0H, hiAddress);
			HW_REG_WRITE1(APU, RVBARADDR1L, lowAddress);
			HW_REG_WRITE1(APU, RVBARADDR1H, hiAddress);
			HW_REG_WRITE1(APU, RVBARADDR2L, lowAddress);
			HW_REG_WRITE1(APU, RVBARADDR2H, hiAddress);
			HW_REG_WRITE1(APU, RVBARADDR3L, lowAddress);
			HW_REG_WRITE1(APU, RVBARADDR3H, hiAddress);
			// wakey wakey rise and shine
			A53WakeUp0();
			A53WakeUp1();
			A53WakeUp2();
			A53WakeUp3();
			break;
		}
//		case "R5F"_hash: {
//			break;
//		}
		default:
			debug_printf(ANSI_RED_PAPER ANSI_BRIGHT_ON "\nUnknown CPU target\n" ANSI_RESET_ATTRIBUTES);
			this->currentState = State::RECEIVING_COMMAND;
			return;
	}
}
void HostInterface::Reset(uint8_t const *cmdBuffer, unsigned int const *finds, unsigned int const findCount) {
	debug_print("\nSoft Reset in Progress\n");
	A53Sleep0();
	A53Sleep1();
	A53Sleep2();
	A53Sleep3();
	R5FSleep0();
	R5FSleep1();
	// restore boot program
	using namespace Dma::LpdDma;
	Stall(Channels::ChannelSevern);
	SimpleDmaCopy(Channels::ChannelSevern,
								(uintptr_all_t) osHeap->bootOCMStore,
								osHeap->bootData.bootCodeStart,
								osHeap->bootData.bootCodeSize);
	debug_printf("\nSoft reset from %#010lx\n", osHeap->bootData.bootCodeStart);

	Stall(Channels::ChannelSevern);
	auto const lowAddress = (uint32_t) (osHeap->bootData.bootCodeStart & 0x0000'0000'FFFF'FFFFull);
	auto const hiAddress = (uint32_t) ((osHeap->bootData.bootCodeStart & 0xFFFF'FFFF'0000'0000ull) >> 32ull);
	HW_REG_WRITE1(APU, RVBARADDR0L, lowAddress);
	HW_REG_WRITE1(APU, RVBARADDR0H, hiAddress);
	A53WakeUp0();

}