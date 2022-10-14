#pragma once
#include "core/core.h"
#include "memory/memory.h"
#include "core/utf8.h"

enum FATFS_FileMode {
	FATFS_FM_Read = 1,
	FATFS_FM_Write = FATFS_FM_Read << 1,
	FATFS_FM_Append = FATFS_FM_Write << 1,
	FATFS_FM_ReadWrite = FATFS_FM_Read | FATFS_FM_Write,
	FATFS_FM_ReadAppend = FATFS_FM_Read | FATFS_FM_Append,
};

enum FATFS_FileSeekDir {
	FATFS_FSD_Begin = 0,
	FATFS_FSD_Current,
	FATFS_FSD_End,
};

typedef struct FATFS * FATFS_DriveHandle;
typedef struct FAT * FATFS_FileHandle;

#define FATFS_SIZEOF_FILE (48 + 40 + 512)
#define FATFS_SIZEOF_DRIVE (72 + 512)

#define FATFS_DECLARE_FILEHANDLE(name)\
uint8_t name##_MEMORY[FATFS_SIZEOF_FILE]; \
FATFS_FileHandle name = (FATFS_FileHandle) name##_MEMORY;

#define FATFS_DECLARE_DRIVE(name) \
uint8_t name##_MEMORY[FATFS_SIZEOF_DRIVE]; \
FATFS_DriveHandle name = (FATFS_DriveHandle) name##_MEMORY;

EXTERN_C void FATFS_Mount(FATFS_DriveHandle drive_, utf8_int8_t const* driveName);
EXTERN_C NON_NULL(1) void FATFS_Close(FATFS_FileHandle fh_);
EXTERN_C NON_NULL(1) void FATFS_Flush(FATFS_FileHandle fh_);
EXTERN_C NON_NULL(1,2) size_t FATFS_Read(FATFS_FileHandle fh_, void *buffer_, size_t byteCount_);
EXTERN_C NON_NULL(1,2) size_t FATFS_Write(FATFS_FileHandle fh_, void const *buffer_, size_t byteCount_);
EXTERN_C NON_NULL(1) bool FATFS_Seek(FATFS_FileHandle fh_, int64_t offset, enum FATFS_FileSeekDir origin_);
EXTERN_C NON_NULL(1) int64_t FATFS_Tell(FATFS_FileHandle fh_);
EXTERN_C NON_NULL(1) size_t FATFS_Size(FATFS_FileHandle fh_);
EXTERN_C NON_NULL(1) bool FATFS_IsEOF(FATFS_FileHandle fh_);
EXTERN_C NON_NULL(1,2) bool FATFS_Open(FATFS_FileHandle fh_, utf8_int8_t const *filename_, enum FATFS_FileMode mode_);
