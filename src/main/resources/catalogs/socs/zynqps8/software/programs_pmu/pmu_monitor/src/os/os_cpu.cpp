#include "core/core.h"
#include "osservices/ipi3_transport.h"
#include "ipi3_os_server.hpp"
#include "../cpuwake.hpp"

namespace IPI3_OsServer {

void CpuWakeOrSleep(IPI3_Msg const *const msgBuffer) {
	if(msgBuffer->Payload.CPUWakeOrSleep.sleep_a53_0) {
		A53Sleep0();
	}
	if(msgBuffer->Payload.CPUWakeOrSleep.sleep_a53_1) {
		A53Sleep1();
	}
	if(msgBuffer->Payload.CPUWakeOrSleep.sleep_a53_2) {
		A53Sleep2();
	}
	if(msgBuffer->Payload.CPUWakeOrSleep.sleep_a53_3) {
		A53Sleep3();
	}
	if(msgBuffer->Payload.CPUWakeOrSleep.sleep_r5f_0) {
		R5FSleep0();
	}
	if(msgBuffer->Payload.CPUWakeOrSleep.sleep_r5f_1) {
		R5FSleep1();
	}

	if(msgBuffer->Payload.CPUWakeOrSleep.wake_a53_0) {
		A53WakeUp0();
	}
	if(msgBuffer->Payload.CPUWakeOrSleep.wake_a53_1) {
		A53WakeUp1();
	}
	if(msgBuffer->Payload.CPUWakeOrSleep.wake_a53_2) {
		A53WakeUp2();
	}
	if(msgBuffer->Payload.CPUWakeOrSleep.wake_a53_3) {
		A53WakeUp3();
	}
	if(msgBuffer->Payload.CPUWakeOrSleep.wake_r5f_0) {
		R5FWakeUp0();
	}
	if(msgBuffer->Payload.CPUWakeOrSleep.wake_r5f_1) {
		R5FWakeUp1();
	}
}

} // end namespace