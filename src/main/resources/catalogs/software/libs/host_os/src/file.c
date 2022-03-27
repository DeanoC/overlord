#include "core/core.h"
#include "dbg/assert.h"
#include "host_os/file.h"
#include <stdio.h>

static void TranslateFileAccessFlags(enum Os_FileMode modeFlags, char *fileAccessString, int strLength) {
  assert(fileAccessString != nullptr && strLength >= 4);
  memset(fileAccessString, '\0', strLength);
  int index = 0;

  // Read + Write uses w+ then filemode (b or t)
  if (modeFlags & Os_FM_Read && modeFlags & Os_FM_Write) {
    fileAccessString[index++] = 'w';
    fileAccessString[index++] = '+';
  }
    // Read + Append uses a+ then filemode (b or t)
  else if (modeFlags & Os_FM_Read && modeFlags & Os_FM_Append) {
    fileAccessString[index++] = 'a';
    fileAccessString[index++] = '+';
  } else {
    if (modeFlags & Os_FM_Read) {
      fileAccessString[index++] = 'r';
    }
    if (modeFlags & Os_FM_Write) {
      fileAccessString[index++] = 'w';
    }
    if (modeFlags & Os_FM_Append) {
      fileAccessString[index++] = 'a';
    }
  }

  if (modeFlags & Os_FM_Binary) {
    fileAccessString[index++] = 'b';
  } else {
    fileAccessString[index++] = 't';
  }

  fileAccessString[index] = '\0';
}

Os_FileHandle Os_FileOpen(char const *filename, enum Os_FileMode mode) {
  char flags[4];
  TranslateFileAccessFlags(mode, flags, 4);
  FILE *fp = fopen(filename, flags);
  return (Os_FileHandle)fp;
}

bool Os_FileClose(Os_FileHandle handle) {
  return (fclose((FILE *) handle) == 0);
}

void Os_FileFlush(Os_FileHandle handle) {
  fflush((FILE *) handle);
}

size_t Os_FileRead(Os_FileHandle handle, void *buffer, size_t byteCount) {
  return fread(buffer,
               1,
               byteCount,
               (FILE *) handle);
}

bool Os_FileSeek(Os_FileHandle handle, int64_t offset, enum Os_FileSeekDir origin) {
  return fseek((FILE *) handle, (long) offset, origin) == 0;
}

int64_t Os_FileTell(Os_FileHandle handle) {
  return ftell((FILE *) handle);
}

size_t Os_FileWrite(Os_FileHandle handle, void const *buffer, size_t byteCount) {
  return fwrite(buffer,
                1,
                byteCount,
                (FILE *) handle);
}

size_t Os_FileSize(Os_FileHandle handle) {
  int64_t curPos = Os_FileTell(handle);
  Os_FileSeek(handle, 0, Os_FSD_End);
  int64_t length = Os_FileTell(handle);
  Os_FileSeek(handle, curPos, Os_FSD_Begin);
  return (size_t) length;
}

bool Os_FileIsEOF(Os_FileHandle handle) {
  return feof((FILE *) handle);
}

bool Os_FileIsOpen(Os_FileHandle handle) {
	return handle != NULL;
}