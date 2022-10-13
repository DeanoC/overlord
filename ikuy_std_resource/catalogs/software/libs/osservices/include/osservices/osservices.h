#pragma once

#ifdef __cplusplus
EXTERN_C {
#endif

#define OS_GLOBAL0_PMU_READY 		(1 << 0)
#define OS_GLOBAL0_BOOT_COMPLETE 	(1 << 1)

// video block is either null (no video output has been setup)
// or
// 640KB block setup as
// 0B: 64B dma descriptor for front buffer
// 64B: 4032B unused
// 4KB: 600KB - 640 * 480 front buffer of RGB565 pixels
// 604KB: 36KB unused
typedef struct BootData {
	uintptr_all_t mmu;            // Mmu manager
	uintptr_lo_t bootCodeStart; 	// location where the boot program begins
	uint32_t bootCodeSize; 			 	// size of boot program in bytes
	uintptr_lo_t videoBlock;      // 1MB 4096B for Descriptor + 640 x 480 * 2 front buffer
	uint32_t padd0;
} BootData;

static_assert(sizeof(BootData) <= 29, "Boot Data Too big");

#define OSS_INLINE_TEXT(text) sizeof(text), text
void OsService_InlinePrint(uint8_t size, const char *text) NON_NULL(2);
void OsService_Print(const char * text) NON_NULL(1);
void OsService_PrintWithSize(unsigned int count, const char * text) NON_NULL(2);
void OsService_Printf(const char *format, ...) NON_NULL(1) __attribute__((format(printf, 1, 2)));

WARN_UNUSED_RESULT uintptr_lo_t OsService_DdrLoBlockAlloc(uint32_t blocks64KB_, uint32_t tag_);
void OsService_DdrLoBlockFree(uintptr_lo_t ptr_, uint32_t blockCount_, uint32_t tag_);
WARN_UNUSED_RESULT uintptr_all_t OsService_DdrHiBlockAlloc(uint32_t blocks64KB_, uint32_t tag_);
void OsService_DdrHiBlockFree(uintptr_all_t ptr_, uint32_t blockCount_, uint32_t tag_);

void OsService_BootComplete(BootData const* bootData);
void OsService_FetchBootData(BootData* bootData);

typedef enum OSS_CPU {
	OSSC_A53_0 = (1 << 0),
	OSSC_A53_1 = (1 << 1),
	OSSC_A53_2 = (1 << 2),
	OSSC_A53_3 = (1 << 3),
	OSSC_R5F_0 = (1 << 4),
	OSSC_R5F_1 = (1 << 5),
} OSS_CPU;

void OsService_SleepCpus(uint8_t cpus);
void OsService_WakeCpu(uint8_t cpus, uintptr_all_t wakeAddress);
void OsService_SleepFPGA();
void OsService_WakeFPGA();

uint8_t OsService_GetCoreHart();

#define OS_SERVICE_TAG(a,b,c,d) (((uint32_t)(a) << 24) | ((uint32_t)(b) << 16) | ((uint32_t)(c) << 8) | ((uint32_t)(d) << 0))

#ifdef __cplusplus
}
#endif
