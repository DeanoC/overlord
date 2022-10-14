#pragma once

#include "core/core.h"
EXTERN_C bool FATFS_ZynqMPSDCardInit();
EXTERN_C bool FATFS_ZynqMPSDCardRead(uint32_t startSector_, void * buffer_, uint32_t sectors_);
EXTERN_C bool FATFS_ZynqMPSDCardWrite(uint32_t startSector_, void const * buffer_, uint32_t sectors_);
EXTERN_C void FATFS_ZynqMPSDCardIdle();
