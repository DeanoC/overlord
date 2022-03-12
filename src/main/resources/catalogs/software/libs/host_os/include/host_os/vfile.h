#pragma once
#include "host_platform/host_platform.h"
#include "memory/memory.h"
#include "host_os/file.h"

typedef struct VFile_OsFile_t {
		Os_FileHandle fileHandle;
} VFile_OsFile_t;

EXTERN_C VFile_Handle VFile_FromFile(char const *filename, enum Os_FileMode mode);

#if MEMORY_TRACKING_SETUP == 1
#define VFile_FromFile(filename, mode) ((Memory_TrackerPushNextSrcLoc(__FILE__, __LINE__, __FUNCTION__)) ? VFile_FromFile(filename, mode) : NULL)
#endif
