#pragma once

#ifdef __cplusplus
EXTERN_C {
#endif

#define OS_GLOBAL0_PMU_READY 		(1 << 0)
#define OS_GLOBAL0_BOOT_COMPLETE 	(1 << 1)

typedef struct BootData {
	uint16_t frameBufferWidth;
	uint16_t frameBufferHeight;
	uint16_t frameBufferHertz;
	uint16_t videoBlockSizeInMB;	// video block size in MB
	uint32_t bootCodeSize; 			 	// size of boot program in bytes

	uintptr_lo_t videoBlock; 			// 4K dma desc space then framebuffer
	uintptr_lo_t bootCodeStart; 	// location where the boot program begins
} BootData;
static_assert(sizeof(BootData) <= 29, "Boot Data Too big");

#define OSS_INLINE_TEXT(text) sizeof(text), text
void OsService_InlinePrint(uint8_t size, const char *text) NON_NULL(2);
void OsService_Print(const char * text) NON_NULL(1);
void OsService_PrintWithSize(unsigned int count, const char * text) NON_NULL(2);
void OsService_Printf(const char *format, ...) NON_NULL(1) __attribute__((format(printf, 1, 2)));
void OsService_EnableScreenConsole(bool enable);
void OsService_ScreenConsoleInlinePrint(uint8_t size, const char *text) NON_NULL(2);
void OsService_ScreenConsolePrintWithSize(unsigned int count, const char * text) NON_NULL(2);
void OsService_ScreenConsolePrint(const char *text) NON_NULL(1);
void OsService_ScreenConsolePrintf(const char *format, ...) NON_NULL(1) __attribute__((format(printf, 1, 2)));

WARN_UNUSED_RESULT uintptr_lo_t OsService_DdrLoBlockAlloc(uint32_t blocks128KB_);
void OsService_DdrLoBlockFree(uintptr_lo_t ptr_, uint32_t blockCount_);
WARN_UNUSED_RESULT uintptr_all_t OsService_DdrHiBlockAlloc(uint32_t blocks128KB_);
void OsService_DdrHiBlockFree(uintptr_all_t ptr_, uint32_t blockCount_);

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
uint8_t OsService_GetCoreHart();

#ifdef __cplusplus
}
#endif
