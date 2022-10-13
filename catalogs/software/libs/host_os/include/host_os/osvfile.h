#pragma once
#include "platform/host/platform.h"
#include "memory/memory.h"
#include "vfile/vfile.h"
#include "host_os/file.h"

typedef struct Os_VFile_t {
		Memory_Allocator* allocator;
		Os_FileHandle fileHandle;
} Os_VFile_t;

EXTERN_C VFile_Handle Os_VFileFromFile(char const *filename, enum Os_FileMode mode, Memory_Allocator* allocator);
EXTERN_C void* Os_AllFromFile(char const *filename, bool text, size_t* outSize, Memory_Allocator* allocator);
