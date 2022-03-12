#include "core/core.h"
#include "platform/memory_map.h"
#include "platform/reg_access.h"
#include "osservices/ipi3_transport.h"
#include "platform/cache.h"

// Manual is wrong,
// see https://forums.xilinx.com/t5/Processor-System-Design-and-AXI/Error-in-UG1085-Zynq-UltraScale-Device-TRM-IPI-Buffers/m-p/1281937
#if CPU_a53
#define IPI_OBS CH0_OBS
#define IPI_TRIG CH0_TRIG
#define IPI_BUFFER IBO_APU

#elif CPU_r5f
#warning TODO R5F need to use core hart for buffer
#define IPI_OBS CH1_OBS
#define IPI_TRIG CH1_TRIG
#define IPI_BUFFER IBO_R5F_0

#elif CPU_pmu
#define IPI_OBS PMU_3_OBS
#define IPI_TRIG PMU_3_TRIG
#define IPI_BUFFER IBO_PMU


#else
#error CPU not supported for IPI
#endif
//#define WAIT_FOR_ACK(a,b) while (HW_REG_GET(IPI, a) & (b)) { /*raw_debug_printf("%#010x %#010x",HW_REG_GET(IPI, a), (uint32_t)b); */}
#define WAIT_FOR_ACK(a,b) while (HW_REG_GET(IPI, a) & (b)) {}

void IPI3_OsService_Submit(const IPI3_Msg *const msg) {
	WAIT_FOR_ACK(IPI_OBS, IC_PMU_3)

	memcpy(IPI_MSG(IPI_BUFFER, IA_PMU), msg, 32);
	Cache_DCacheCleanAndInvalidateLine((uintptr_t)IPI_MSG(IPI_BUFFER, IA_PMU));
	HW_REG_SET(IPI, IPI_TRIG, IC_PMU_3);

}

void IPI3_OnService_SubmitAndFetchResponse(const IPI3_Msg *const msg, IPI3_Response * reply) {
	IPI3_OsService_Submit(msg);
	WAIT_FOR_ACK(IPI_OBS, IC_PMU_3)
	memcpy(reply, IPI_RESPONSE(IPI_BUFFER, IA_PMU), 32);
}

