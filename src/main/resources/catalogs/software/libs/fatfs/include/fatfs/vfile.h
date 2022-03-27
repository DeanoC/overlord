#pragma once
#include "core/core.h"
#include "vfile/vfile.h"
#include "fatfs.h"

EXTERN_C VFile_Handle FATFS_VFileFromFile(char const *filename, enum FATFS_FileMode mode, Memory_Allocator* allocator);
