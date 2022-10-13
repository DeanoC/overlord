#pragma once

#include "core/core.h"
#include "core/utf8.h"

enum VFile_SeekDir {
    VFile_SD_Begin = 0,
    VFile_SD_Current,
    VFile_SD_End,
};

typedef struct VFile_Interface_t *VFile_Handle;

EXTERN_C void VFile_Close(VFile_Handle handle);
EXTERN_C void VFile_Flush(VFile_Handle handle);
EXTERN_C size_t VFile_Read(VFile_Handle handle, void *buffer, size_t byteCount);
EXTERN_C size_t VFile_Write(VFile_Handle handle, void const *buffer, size_t byteCount);
EXTERN_C bool VFile_Seek(VFile_Handle handle, int64_t offset, enum VFile_SeekDir origin);
EXTERN_C WARN_UNUSED_RESULT int64_t VFile_Tell(VFile_Handle handle);
EXTERN_C WARN_UNUSED_RESULT size_t VFile_Size(VFile_Handle handle);
EXTERN_C WARN_UNUSED_RESULT utf8_int8_t const *VFile_GetName(VFile_Handle handle);
EXTERN_C WARN_UNUSED_RESULT bool VFile_IsEOF(VFile_Handle handle);
EXTERN_C WARN_UNUSED_RESULT uint32_t VFile_GetType(VFile_Handle handle);
EXTERN_C WARN_UNUSED_RESULT void* VFile_GetTypeSpecificData(VFile_Handle handle);

typedef void (*VFile_CloseFunc)(struct VFile_Interface_t *);
typedef void (*VFile_FlushFunc)(struct VFile_Interface_t *);
typedef size_t (*VFile_ReadFunc)(struct VFile_Interface_t *, void *buffer, size_t byteCount);
typedef size_t (*VFile_WriteFunc)(struct VFile_Interface_t *, void const *buffer, size_t byteCount);
typedef bool (*VFile_SeekFunc)(struct VFile_Interface_t *, int64_t offset, enum VFile_SeekDir origin);
typedef int64_t (*VFile_TellFunc)(struct VFile_Interface_t *);
typedef size_t (*VFile_SizeFunc)(struct VFile_Interface_t *);
typedef utf8_int8_t const *(*VFile_GetNameFunc)(struct VFile_Interface_t *);
typedef bool (*VFile_IsEOFFunc)(struct VFile_Interface_t *);

static const uint32_t InterfaceMagic = 0xDEA0DEA0;

// this is the shared header to the various vfile specific
// all vfile derived should have this as the first part of there
// own structure
typedef struct VFile_Interface_t {

    uint32_t magic;
    uint32_t type;

    VFile_CloseFunc closeFunc;
    VFile_FlushFunc flushFunc;
    VFile_ReadFunc readFunc;
    VFile_WriteFunc writeFunc;
    VFile_SeekFunc seekFunc;
    VFile_TellFunc tellFunc;
    VFile_SizeFunc sizeFunc;
    VFile_GetNameFunc nameFunc;
    VFile_IsEOFFunc isEofFunc;

} VFile_Interface_t;


