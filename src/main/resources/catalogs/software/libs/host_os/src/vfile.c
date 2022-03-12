
#include "core/core.h"
#include "core/utf8.h"
#include "memory/memory.h"
#include "host_os/file.h"
#include "vfile/vfile.h"
#include "host_os/vfile.h"
#include <string.h>

static void VFile_OsFile_Close(VFile_Interface_t *vif) {
	VFile_OsFile_t *vof = (VFile_OsFile_t *) (vif + 1);
	Os_FileClose(vof->fileHandle);
}

static void VFile_OsFile_Flush(VFile_Interface_t *vif) {
	VFile_OsFile_t *vof = (VFile_OsFile_t *) (vif + 1);
	Os_FileFlush(vof->fileHandle);
}

static size_t VFile_OsFile_Read(VFile_Interface_t *vif, void *buffer, size_t byteCount) {
	VFile_OsFile_t *vof = (VFile_OsFile_t *) (vif + 1);
	return Os_FileRead(vof->fileHandle, buffer, byteCount);
}
static size_t VFile_OsFile_Write(VFile_Interface_t *vif, void const *buffer, size_t byteCount) {
	VFile_OsFile_t *vof = (VFile_OsFile_t *) (vif + 1);
	return Os_FileWrite(vof->fileHandle, buffer, byteCount);
}

static bool VFile_OsFile_Seek(VFile_Interface_t *vif, int64_t offset, enum VFile_SeekDir origin) {
	VFile_OsFile_t *vof = (VFile_OsFile_t *) (vif + 1);
	return Os_FileSeek(vof->fileHandle, offset, (enum Os_FileSeekDir) (origin));
}

static int64_t VFile_OsFile_Tell(VFile_Interface_t *vif) {
	VFile_OsFile_t *vof = (VFile_OsFile_t *) (vif + 1);
	return Os_FileTell(vof->fileHandle);
}

static size_t VFile_OsFile_Size(VFile_Interface_t *vif) {
	VFile_OsFile_t *vof = (VFile_OsFile_t *) (vif + 1);
	return Os_FileSize(vof->fileHandle);
}

static char const *VFile_OsFile_GetName(VFile_Interface_t *vif) {
	VFile_OsFile_t *vof = (VFile_OsFile_t *) (vif + 1);
	char const *name = (char const *) (vof + 1);
	return name;
}

static bool VFile_OsFile_IsEOF(VFile_Interface_t *vif) {
	VFile_OsFile_t *vof = (VFile_OsFile_t *) (vif + 1);
	return Os_FileIsEOF(vof->fileHandle);
}

#if MEMORY_TRACKING_SETUP == 1
#undef VFile_FromFile
#endif

EXTERN_C VFile_Handle VFile_FromFile(char const *filename, enum Os_FileMode mode) {
	Os_FileHandle handle = Os_FileOpen(filename, mode);
	if (handle == NULL) { return NULL; }

	const uint64_t mallocSize =
			sizeof(VFile_Interface_t) +
			sizeof(VFile_OsFile_t) +
			utf8size(filename) + 1;
#if MEMORY_TRACKING_SETUP == 1
	// call the allocator direct, so that the line and file comes free the caller
	VFile_Interface_t *vif = (VFile_Interface_t *) Memory_GlobalAllocator.malloc(mallocSize);
#else
	VFile_Interface_t *vif = (VFile_Interface_t *) MEMORY_MALLOC(mallocSize);
#endif
	vif->magic = InterfaceMagic;
	vif->type = VFile_Type_OsFile;
	vif->closeFunc = &VFile_OsFile_Close;
	vif->flushFunc = &VFile_OsFile_Flush;
	vif->readFunc = &VFile_OsFile_Read;
	vif->writeFunc = &VFile_OsFile_Write;
	vif->seekFunc = &VFile_OsFile_Seek;
	vif->tellFunc = &VFile_OsFile_Tell;
	vif->sizeFunc = &VFile_OsFile_Size;
	vif->nameFunc = &VFile_OsFile_GetName;
	vif->isEofFunc = &VFile_OsFile_IsEOF;

	VFile_OsFile_t *vof = (VFile_OsFile_t *) (vif + 1);
	vof->fileHandle = handle;
	char *dstname = (char *) (vof + 1);
	strcpy(dstname, filename);

	return (VFile_Handle) vif;
}