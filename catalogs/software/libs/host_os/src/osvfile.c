
#include "core/core.h"
#include "core/utf8.h"
#include "dbg/print.h"
#include "memory/memory.h"
#include "host_os/file.h"
#include "vfile/vfile.h"
#include "vfile/utils.h"
#include "host_os/osvfile.h"
#include <string.h>


static void VFile_OsFile_Close(VFile_Interface_t *vif) {
	Os_VFile_t *vof = (Os_VFile_t *) (vif + 1);
	Os_FileClose(vof->fileHandle);
	MFREE(vof->allocator, vif);
}

static void VFile_OsFile_Flush(VFile_Interface_t *vif) {
	Os_VFile_t *vof = (Os_VFile_t *) (vif + 1);
	Os_FileFlush(vof->fileHandle);
}

static size_t VFile_OsFile_Read(VFile_Interface_t *vif, void *buffer, size_t byteCount) {
	Os_VFile_t *vof = (Os_VFile_t *) (vif + 1);
	return Os_FileRead(vof->fileHandle, buffer, byteCount);
}
static size_t VFile_OsFile_Write(VFile_Interface_t *vif, void const *buffer, size_t byteCount) {
	Os_VFile_t *vof = (Os_VFile_t *) (vif + 1);
	return Os_FileWrite(vof->fileHandle, buffer, byteCount);
}

static bool VFile_OsFile_Seek(VFile_Interface_t *vif, int64_t offset, enum VFile_SeekDir origin) {
	Os_VFile_t *vof = (Os_VFile_t *) (vif + 1);
	return Os_FileSeek(vof->fileHandle, offset, (enum Os_FileSeekDir) (origin));
}

static int64_t VFile_OsFile_Tell(VFile_Interface_t *vif) {
	Os_VFile_t *vof = (Os_VFile_t *) (vif + 1);
	return Os_FileTell(vof->fileHandle);
}

static size_t VFile_OsFile_Size(VFile_Interface_t *vif) {
	Os_VFile_t *vof = (Os_VFile_t *) (vif + 1);
	return Os_FileSize(vof->fileHandle);
}

static char const *VFile_OsFile_GetName(VFile_Interface_t *vif) {
	Os_VFile_t *vof = (Os_VFile_t *) (vif + 1);
	char const *name = (char const *) (vof + 1);
	return name;
}

static bool VFile_OsFile_IsEOF(VFile_Interface_t *vif) {
	Os_VFile_t *vof = (Os_VFile_t *) (vif + 1);
	return Os_FileIsEOF(vof->fileHandle);
}


void* Os_AllFromFile(char const *filename, bool text, size_t* outSize, Memory_Allocator* allocator) {
	VFile_Handle fh = Os_VFileFromFile(filename, text ? Os_FM_Read : Os_FM_ReadBinary, allocator);
	if(!fh) {
		debug_printf("ERROR: File not found %s\n", filename);
		if(outSize != nullptr) *outSize = 0;
		return nullptr;
	}

	size_t const size = VFile_Size(fh);
	void *ret = MALLOC(allocator, size);
	if(ret != nullptr) {
		VFile_Read(fh, ret, size);
		if(outSize != nullptr) *outSize = size;
	} else {
		debug_printf("ERROR: Malloc failed %s\n", filename);
		if(outSize != nullptr) *outSize = 0;
	}
	VFile_Close(fh);
	return ret;
}

VFile_Handle Os_VFileFromFile(char const *filename, enum Os_FileMode mode, Memory_Allocator* allocator) {
	Os_FileHandle handle = Os_FileOpen(filename, mode);
	if (handle == nullptr) { return nullptr; }

	const uint64_t mallocSize =
			sizeof(VFile_Interface_t) +
			sizeof(Os_VFile_t) +
			utf8size(filename) + 1;
	VFile_Interface_t *vif = (VFile_Interface_t *) MALLOC(allocator, mallocSize);
	vif->magic = InterfaceMagic;
	vif->type =   VFILE_MAKE_ID('O', 'S', 'F', 'L');
	vif->closeFunc = &VFile_OsFile_Close;
	vif->flushFunc = &VFile_OsFile_Flush;
	vif->readFunc = &VFile_OsFile_Read;
	vif->writeFunc = &VFile_OsFile_Write;
	vif->seekFunc = &VFile_OsFile_Seek;
	vif->tellFunc = &VFile_OsFile_Tell;
	vif->sizeFunc = &VFile_OsFile_Size;
	vif->nameFunc = &VFile_OsFile_GetName;
	vif->isEofFunc = &VFile_OsFile_IsEOF;

	Os_VFile_t *vof = (Os_VFile_t *) (vif + 1);
	vof->fileHandle = handle;
	vof->allocator = allocator;
	char *dstname = (char *) (vof + 1);
	strcpy(dstname, filename);

	return (VFile_Handle) vif;
}