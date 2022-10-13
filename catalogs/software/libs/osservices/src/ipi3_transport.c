#include "core/core.h"
#include "platform/memory_map.h"
#include "platform/reg_access.h"
#include "platform/cache.h"
#include "osservices/ipi3_transport.h"
#include "multi_core/mutex.h"

// Manual is wrong,
// see https://forums.xilinx.com/t5/Processor-System-Design-and-AXI/Error-in-UG1085-Zynq-UltraScale-Device-TRM-IPI-Buffers/m-p/1281937
#if CPU_a53
#define IPI_OBS CH0_OBS
#define IPI_TRIG CH0_TRIG
#define IPI_BUFFER IBO_APU

static Core_Mutex mutex;

#define LOCK_MUTEX() MultiCore_LockRecursiveMutex(&mutex)
#define UNLOCK_MUTEX() MultiCore_UnlockRecursiveMutex(&mutex)

#elif CPU_r5f
#warning TODO R5F need to use core hart for buffer
#define IPI_OBS CH1_OBS
#define IPI_TRIG CH1_TRIG
#define IPI_BUFFER IBO_R5F_0

#define LOCK_MUTEX()
#define UNLOCK_MUTEX()

#elif CPU_pmu
#define IPI_OBS PMU_3_OBS
#define IPI_TRIG PMU_3_TRIG
#define IPI_BUFFER IBO_PMU

#define LOCK_MUTEX()
#define UNLOCK_MUTEX()

#else
#error CPU not supported for IPI
#endif
//#define WAIT_FOR_ACK(a,b) while (HW_REG_READ1(IPI, a) & (b)) { /*raw_debug_printf("%#010x %#010x",HW_REG_READ1(IPI, a), (uint32_t)b); */}
#define WAIT_FOR_ACK(a,b) while (HW_REG_READ1(IPI, a) & (b)) {}

void IPI3_OsService_Submit(const IPI3_Msg *const msg) {
	LOCK_MUTEX();

	WAIT_FOR_ACK(IPI_OBS, IC_PMU_3)
	// TODO BUGFIX - this will fail when r5f and a53 use the os at the same time
	// we need to use the CPUs buffer not PMUs, it works now because APUs are mutexed as they only have one channel
	// but r5f each have there own buffer but this code will use the PMU one, which would have 3 user simultanously
	memcpy(IPI_MSG(IPI_BUFFER, IA_PMU), msg, 32);
	HW_REG_WRITE1(IPI, IPI_TRIG, IC_PMU_3);

	UNLOCK_MUTEX();
}

void IPI3_OnService_SubmitAndFetchResponse(const IPI3_Msg *const msg, IPI3_Response * reply) {
	LOCK_MUTEX();

	IPI3_OsService_Submit(msg);
	WAIT_FOR_ACK(IPI_OBS, IC_PMU_3)
	memcpy(reply, IPI_RESPONSE(IPI_BUFFER, IA_PMU), 32);

	UNLOCK_MUTEX();
}

