#pragma once

#include "core/core.h"
#include "memory/memory.h"
#include "image/image.h"
#include "vfile/vfile.h"

EXTERN_C Image_ImageHeader * Image_LoadPVR(VFile_Handle handle, Memory_Allocator* allocator);
EXTERN_C Image_ImageHeader * Image_LoadLDR(VFile_Handle handle, Memory_Allocator* allocator);
EXTERN_C Image_ImageHeader * Image_LoadEXR(VFile_Handle handle, Memory_Allocator* allocator);
EXTERN_C Image_ImageHeader * Image_LoadKTX(VFile_Handle handle, Memory_Allocator* allocator);
EXTERN_C Image_ImageHeader * Image_LoadDDS(VFile_Handle handle, Memory_Allocator* allocator);

EXTERN_C Image_ImageHeader * Image_Load(VFile_Handle handle, Memory_Allocator* allocator);
