#pragma once

#ifdef __cplusplus
extern "C" {
#endif


#define OSS_INLINE_TEXT(text) sizeof(text), text
void OsService_InlinePrint(uint8_t size, const char *text) NON_NULL(2);
void OsService_Print(const char * text) NON_NULL(1);
void OsService_PrintWithSize(unsigned int count, const char * text) NON_NULL(2);
void OsService_Printf(const char *format, ...) NON_NULL(1) __attribute__((format(printf, 1, 2)));

WARN_UNUSED_RESULT void* OsService_DdrLoBlockAlloc(uint16_t blocks1MB);
void OsService_DdrLoBlockFree(void* ptr) NON_NULL(1);
WARN_UNUSED_RESULT void* OsService_DdrHiBlockAlloc(uint16_t blocks1MB);
void OsService_DdrHiBlockFree(void* ptr) NON_NULL(1);

#ifdef __cplusplus
}
#endif
