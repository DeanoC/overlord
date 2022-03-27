#pragma once

#include "core/core.h"

#define FATFS_MAX_DRIVES 4
#define FATFS_SECTOR_SIZE 512

enum FATFS_DriveStatus {
	FATFSDS_NoInit = (1 << 0),
	FATFSDS_NoMedia = (FATFSDS_NoInit << 1),
	FATFSDS_WriteProtected = (FATFSDS_NoMedia << 1)
};

enum FATFS_DriveIoctl {
	FATFSDI_SYNC = 0
};

typedef FATFS_DriveStatus (*FATFS_DriveInitFunc)();
typedef FATFS_DriveStatus (*FATFS_DriveStatusFunc)();
typedef bool (*FATFS_DriveReadFunc)(void* buffer_, uint32_t startSector_, uint32_t sectorCount_);
typedef bool (*FATFS_DriveWriteFunc)(void* buffer_, uint32_t startSector_, uint32_t sectorCount_);
typedef bool (*FATFS_DriveIoctl)(enum FATFS_DriveIoctl command_, void* buffer_);

typedef struct FATFS_DriveFunctions {
	FATFS_InitFitFunc initFunc;

} FATFS_DriveFunctions;

void FATFS_RegisterDrive(uint32_t)