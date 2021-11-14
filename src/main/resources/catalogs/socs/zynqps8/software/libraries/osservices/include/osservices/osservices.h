#pragma once

#ifdef __cplusplus
extern "C" {
#endif


#define OSS_INLINE_TEXT(text) sizeof(text), text
void OsService_InlinePrint(uint8_t size, const char *text) NON_NULL(2);
void OsService_Print(const char * text) NON_NULL(1);
void OsService_PrintWithSize(unsigned int count, const char * text) NON_NULL(2);
void OsService_Printf(const char *format, ...) NON_NULL(1) __attribute__((format(printf, 1, 2)));
void OsServer_EnableScreenConsole(uintptr_lo_t framebuffer, uint16_t fbWidth, uint16_t fbHeight);
void OsServer_DisableScreenConsole();
void OsService_ScreenConsoleInlinePrint(uint8_t size, const char *text) NON_NULL(2);
void OsService_ScreenConsolePrintWithSize(unsigned int count, const char * text) NON_NULL(2);
void OsService_ScreenConsolePrint(const char *const text) NON_NULL(1);
void OsService_ScreenConsolePrintf(const char *format, ...) NON_NULL(1) __attribute__((format(printf, 1, 2)));

WARN_UNUSED_RESULT uintptr_lo_t OsService_DdrLoBlockAlloc(uint16_t blocks1MB);
void OsService_DdrLoBlockFree(uintptr_lo_t ptr);
WARN_UNUSED_RESULT uintptr_all_t OsService_DdrHiBlockAlloc(uint16_t blocks1MB);
void OsService_DdrHiBlockFree(uintptr_all_t ptr);


#ifdef __cplusplus
}
#endif
