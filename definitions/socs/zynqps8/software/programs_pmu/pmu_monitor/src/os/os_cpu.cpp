#include "core/core.h"
#include "osservices/ipi3_transport.h"
#include "ipi3_os_server.hpp"
#include "dbg/print.h"
#include "platform/memory_map.h"
#include "platform/reg_access.h"
#include "platform/registers/apu.h"
#include "../cpuwake.hpp"
#include "platform/registers/lpd_slcr.h"

namespace IPI3_OsServer {

void CpuWakeOrSleep(IPI3_Msg const *const msgBuffer) {
	uintptr_all_t const addr =  msgBuffer->Payload.CPUWakeOrSleep.wakeAddress;
	if(msgBuffer->Payload.CPUWakeOrSleep.sleepA53_0) {
		A53Sleep0();
	}
	if(msgBuffer->Payload.CPUWakeOrSleep.sleepA53_1) {
		A53Sleep1();
	}
	if(msgBuffer->Payload.CPUWakeOrSleep.sleepA53_2) {
		A53Sleep2();
	}
	if(msgBuffer->Payload.CPUWakeOrSleep.sleepA53_3) {
		A53Sleep3();
	}
	if(msgBuffer->Payload.CPUWakeOrSleep.sleepR5f_0) {
		R5FSleep0();
	}
	if(msgBuffer->Payload.CPUWakeOrSleep.sleepR5f_1) {
		R5FSleep1();
	}

	auto const lowAddress = (uint32_t) (addr & 0x0000'0000'FFFF'FFFFull);
	auto const hiAddress = (uint32_t) ((addr & 0xFFFF'FFFF'0000'0000ull) >> 32ull);
	if(msgBuffer->Payload.CPUWakeOrSleep.wakeA53_0) {
		HW_REG_WRITE1(APU, RVBARADDR0L, lowAddress);
		HW_REG_WRITE1(APU, RVBARADDR0H, hiAddress);
		A53WakeUp0();
	}
	if(msgBuffer->Payload.CPUWakeOrSleep.wakeA53_1) {
		HW_REG_WRITE1(APU, RVBARADDR1L, lowAddress);
		HW_REG_WRITE1(APU, RVBARADDR1H, hiAddress);
		A53WakeUp1();
	}
	if(msgBuffer->Payload.CPUWakeOrSleep.wakeA53_2) {
		HW_REG_WRITE1(APU, RVBARADDR2L, lowAddress);
		HW_REG_WRITE1(APU, RVBARADDR2H, hiAddress);
		A53WakeUp2();
	}
	if(msgBuffer->Payload.CPUWakeOrSleep.wakeA53_3) {
		HW_REG_WRITE1(APU, RVBARADDR3L, lowAddress);
		HW_REG_WRITE1(APU, RVBARADDR3H, hiAddress);
		A53WakeUp3();
	}
	if(msgBuffer->Payload.CPUWakeOrSleep.wakeR5f_0) {
		R5FWakeUp0();
	}
	if(msgBuffer->Payload.CPUWakeOrSleep.wakeR5f_1) {
		R5FWakeUp1();
	}
}

} // end namespace