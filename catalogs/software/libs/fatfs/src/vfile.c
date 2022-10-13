#include "core/core.h"
#include "core/utf8.h"
#include "memory/memory.h"
#include "vfile/vfile.h"
#include "vfile/utils.h"
#include "fatfs/fatfs.h"
#include "ff.h"
#include "fatfs/vfile.h"

typedef struct FATFS_VFile_t {
	Memory_Allocator* allocator;
	FIL fileHandle;
} FATFS_VFile_t;

static void FATFS_VFile_Close(VFile_Interface_t *vif) {
	assert(vif != nullptr);
	FATFS_VFile_t *vof = (FATFS_VFile_t *) (vif + 1);
	f_close(&vof->fileHandle);
	MFREE(vof->allocator, vif);
}

static void FATFS_VFile_Flush(VFile_Interface_t *vif) {
	assert(vif != nullptr);
	FATFS_VFile_t *vof = (FATFS_VFile_t *) (vif + 1);
	FATFS_Flush((FATFS_FileHandle)&vof->fileHandle);
}

static size_t FATFS_VFile_Read(VFile_Interface_t *vif, void *buffer, size_t byteCount) {
	assert(vif != nullptr);
	FATFS_VFile_t *vof = (FATFS_VFile_t *) (vif + 1);
	return FATFS_Read((FATFS_FileHandle)&vof->fileHandle, buffer, byteCount);
}
static size_t FATFS_VFile_Write(VFile_Interface_t *vif, void const *buffer, size_t byteCount) {
	assert(vif != nullptr);
	FATFS_VFile_t *vof = (FATFS_VFile_t *) (vif + 1);
	return  FATFS_Write((FATFS_FileHandle)&vof->fileHandle, buffer, byteCount);
}

static bool FATFS_VFile_Seek(VFile_Interface_t *vif, int64_t offset, enum VFile_SeekDir origin) {
	assert(vif != nullptr);
	FATFS_VFile_t *vof = (FATFS_VFile_t *) (vif + 1);
	FRESULT result = FR_OK;
	switch (origin) {
		case VFile_SD_Begin:
			result = f_lseek(&vof->fileHandle, offset);
			return (result == FR_OK);
		case VFile_SD_Current:
			result = f_lseek(&vof->fileHandle, f_tell(&vof->fileHandle) + offset);
			return (result == FR_OK);
		case VFile_SD_End:
			result = f_lseek(&vof->fileHandle, f_size(&vof->fileHandle) - offset);
			return (result == FR_OK);
		default: return false;
	}
}

static int64_t FATFS_VFile_Tell(VFile_Interface_t *vif) {
	assert(vif != nullptr);
	FATFS_VFile_t *vof = (FATFS_VFile_t *) (vif + 1);
	return f_tell(&vof->fileHandle);
}

static size_t FATFS_VFile_Size(VFile_Interface_t *vif) {
	assert(vif != nullptr);
	FATFS_VFile_t *vof = (FATFS_VFile_t *) (vif + 1);
	return FATFS_Size((FATFS_FileHandle)&vof->fileHandle);
}

static char const *FATFS_VFile_GetName(VFile_Interface_t *vif) {
	assert(vif != nullptr);
	FATFS_VFile_t * vof = (FATFS_VFile_t *) (vif + 1);
	char const *name = (char const *) (vof + 1);
	return name;
}

static bool FATFS_VFile_IsEOF(VFile_Interface_t *vif) {
	assert(vif != nullptr);
	FATFS_VFile_t *vof = (FATFS_VFile_t *) (vif + 1);
	return FATFS_IsEOF((FATFS_FileHandle)&vof->fileHandle);
}

VFile_Handle FATFS_VFileFromName(utf8_int8_t const *filename, enum FATFS_FileMode mode, Memory_Allocator* allocator) {
	FIL fh;
	FRESULT result = FATFS_Open((FATFS_FileHandle)&fh, filename, mode);
	if (result != FR_OK) { return nullptr; }
	return FATFS_VFileFromFileHandle((FATFS_FileHandle)&fh, filename, allocator);
}


EXTERN_C VFile_Handle FATFS_VFileFromFileHandle(FATFS_FileHandle fh, utf8_int8_t const *filename, Memory_Allocator* allocator) {
	uint64_t const mallocSize = sizeof(VFile_Interface_t) + sizeof(FATFS_VFile_t) + utf8size(filename) + 1;
	VFile_Interface_t *vif = (VFile_Interface_t *) MALLOC(allocator, mallocSize);
	vif->magic = InterfaceMagic;
	vif->type =   VFILE_MAKE_ID('F', 'A', 'T', 'F');
	vif->closeFunc = &FATFS_VFile_Close;
	vif->flushFunc = &FATFS_VFile_Flush;
	vif->readFunc = &FATFS_VFile_Read;
	vif->writeFunc = &FATFS_VFile_Write;
	vif->seekFunc = &FATFS_VFile_Seek;
	vif->tellFunc = &FATFS_VFile_Tell;
	vif->sizeFunc = &FATFS_VFile_Size;
	vif->nameFunc = &FATFS_VFile_GetName;
	vif->isEofFunc = &FATFS_VFile_IsEOF;

	FATFS_VFile_t *vof = (FATFS_VFile_t *) (vif + 1);
	memcpy(&vof->fileHandle, fh, sizeof(FIL));

	vof->allocator = allocator;
	char *dstname = (char *) (vof + 1);
	utf8cpy(dstname, filename);

	return (VFile_Handle) vif;
}