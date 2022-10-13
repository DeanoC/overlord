// License Summary: MIT see LICENSE file

#pragma once

#include "core/core.h"

typedef struct Os_File *Os_FileHandle;

enum Os_FileMode {
  Os_FM_Read = 1,
  Os_FM_Write = Os_FM_Read << 1,
  Os_FM_Append = Os_FM_Write << 1,
  Os_FM_Binary = Os_FM_Append << 1,
  Os_FM_ReadWrite = Os_FM_Read | Os_FM_Write,
  Os_FM_ReadAppend = Os_FM_Read | Os_FM_Append,
  Os_FM_WriteBinary = Os_FM_Write | Os_FM_Binary,
  Os_FM_ReadBinary = Os_FM_Read | Os_FM_Binary
};

enum Os_FileSeekDir {
  Os_FSD_Begin = 0,
  Os_FSD_Current,
  Os_FSD_End,
};

/// Low level file system interface providing basic file I/O operations
/// Implementations platform dependent
EXTERN_C Os_FileHandle Os_FileOpen(char const *filename, enum Os_FileMode mode);
EXTERN_C bool Os_FileClose(Os_FileHandle handle);
EXTERN_C bool Os_FileIsOpen(Os_FileHandle handle);


EXTERN_C void Os_FileFlush(Os_FileHandle handle);
EXTERN_C size_t Os_FileRead(Os_FileHandle handle, void *buffer, size_t byteCount);
EXTERN_C size_t Os_FileWrite(Os_FileHandle handle, void const *buffer, size_t byteCount);
EXTERN_C bool Os_FileSeek(Os_FileHandle handle, int64_t offset, enum Os_FileSeekDir origin);
EXTERN_C int64_t Os_FileTell(Os_FileHandle handle);
EXTERN_C size_t Os_FileSize(Os_FileHandle handle);
EXTERN_C bool Os_FileIsEOF(Os_FileHandle handle);