#pragma once
#include "core/core.h"
#include "core/utf8.h"
#include "vfile/vfile.h"
#include "fatfs.h"

EXTERN_C VFile_Handle FATFS_VFileFromName(utf8_int8_t const *filename, enum FATFS_FileMode mode, Memory_Allocator* allocator);
EXTERN_C VFile_Handle FATFS_VFileFromFileHandle(FATFS_FileHandle fh, utf8_int8_t const *filename, Memory_Allocator* allocator);
