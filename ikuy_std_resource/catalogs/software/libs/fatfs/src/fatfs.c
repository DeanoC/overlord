#include "core/core.h"
#include "core/utf8.h"
#include "dbg/print.h"
#include "memory/memory.h"
#include "fatfs/fatfs.h"
#include "fatfs/filesystem.h"
#include "ff.h"

static_assert(sizeof(FFOBJID) == 48);
#define SizeOfFIL (offsetof(FIL, buf) + FF_MAX_SS)
static_assert(sizeof(FIL) == SizeOfFIL);
static_assert(sizeof(FIL) == FATFS_SIZEOF_FILE);
#define SizeOfFATFS (72 + FF_MAX_SS)
static_assert(sizeof(FATFS) == SizeOfFATFS);

static BYTE TranslateFileAccessFlags(enum FATFS_FileMode modeFlags) {
	BYTE flags = FA_OPEN_EXISTING;

	if (modeFlags & FATFS_FM_Read && modeFlags & FATFS_FM_Write) {
		flags |= FA_READ | FA_WRITE | FA_OPEN_ALWAYS;
	}
	else if (modeFlags & FATFS_FM_Read && modeFlags & FATFS_FM_Append) {
		// Read + Append uses Open and append
		flags |= FA_READ | FA_OPEN_APPEND;
	} else {
		if (modeFlags & FATFS_FM_Read) {
			flags |= FA_READ;
		}
		if (modeFlags & FATFS_FM_Write) {
			flags |= FA_WRITE | FA_CREATE_ALWAYS;
		}
		if (modeFlags & FATFS_FM_Append) {
			flags |= FA_WRITE | FA_OPEN_APPEND;
		}
	}
	return flags;
}

void FATFS_Mount(FATFS_DriveHandle drive_, utf8_int8_t const* driveName) {
	FATFS* fatfs = (FATFS *) drive_;
	f_mount(fatfs, driveName, 0);
}

bool FATFS_Open(FATFS_FileHandle fh_, utf8_int8_t const *filename_, enum FATFS_FileMode mode_) {
	FIL* fh = (FIL *) fh_;
	FRESULT result = f_open(fh, (char const*)filename_, TranslateFileAccessFlags(mode_));
	return (result == FR_OK);
}

void FATFS_Close(FATFS_FileHandle fh_) {
	FIL* fh = (FIL *) fh_;
	f_close(fh);
}

void FATFS_Flush(FATFS_FileHandle fh_) {
	FIL* fh = (FIL *) fh_;
	FRESULT result = f_sync(fh);
	assert(result == FR_OK);
}

size_t FATFS_Read(FATFS_FileHandle fh_, void *buffer_, size_t byteCount_) {
	FIL* fh = (FIL *) fh_;
	UINT bytesRead = 0;
	FRESULT result = f_read(fh, buffer_, byteCount_, &bytesRead);
	assert(result == FR_OK);

	return bytesRead;
}

size_t FATFS_Write(FATFS_FileHandle fh_, void const *buffer_, size_t byteCount_) {
	FIL* fh = (FIL *) fh_;
	UINT bytesWritten = 0;
	FRESULT result = f_write(fh, buffer_, byteCount_, &bytesWritten);
	assert(result == FR_OK);
	return bytesWritten;
}

bool FATFS_Seek(FATFS_FileHandle fh_, int64_t offset_, enum FATFS_FileSeekDir origin_) {
	FIL* fh = (FIL *) fh_;
	FRESULT result = FR_OK;
	switch (origin_) {
		case FATFS_FSD_Begin:
			result = f_lseek(fh, offset_);
			return (result == FR_OK);
		case FATFS_FSD_Current:
			result = f_lseek(fh, f_tell(fh) + offset_);
			return (result == FR_OK);
		case FATFS_FSD_End:
			result = f_lseek(fh, f_size(fh) - offset_);
			return (result == FR_OK);
		default: return false;
	}
}

int64_t FATFS_Tell(FATFS_FileHandle fh_) {
	FIL* fh = (FIL *) fh_;
	return f_tell(fh);
}

size_t FATFS_Size(FATFS_FileHandle fh_) {
	FIL* fh = (FIL *) fh_;
	return f_size(fh);
}

bool FATFS_IsEOF(FATFS_FileHandle fh_) {
	FIL* fh = (FIL *) fh_;
	return f_eof(fh);
}

typedef struct FATFS_DirectoryEnumerator {
	DIR dir;
	FATFS_DirectoryEnumeratorItem lastItem;
	FILINFO fileInfo;
} FATFS_DirectoryEnumerator;

bool FATFS_DirectoryEnumeratorCreate(FATFS_DirectoryEnumeratorHandle* handle_, utf8_int8_t const * path_) {
	FATFS_DirectoryEnumerator* enumerator = (FATFS_DirectoryEnumerator*)handle_;
	if(f_opendir(&enumerator->dir, path_) == FR_OK) {
		return true;
	}

	return false;
}

void FATFS_DirectoryEnumeratorDestroy(FATFS_DirectoryEnumeratorHandle handle) {
	FATFS_DirectoryEnumerator * enumerator = (FATFS_DirectoryEnumerator *) handle;
	f_closedir(&enumerator->dir);
}

FATFS_DirectoryEnumeratorItem const* FATFS_DirectoryEnumeratorNext(FATFS_DirectoryEnumeratorHandle handle_) {
	FATFS_DirectoryEnumerator* enumerator = (FATFS_DirectoryEnumerator*)handle_;

	while(f_readdir(&enumerator->dir, &enumerator->fileInfo) == FR_OK) {
		if(	!enumerator->fileInfo.fname[0] &&
				!utf8ncmp(enumerator->fileInfo.fname, ".", 1) &&
				!utf8ncmp(enumerator->fileInfo.fname, "..", 2) ) {
			enumerator->lastItem.filename = enumerator->fileInfo.fname;
			enumerator->lastItem.directory = enumerator->fileInfo.fattrib & AM_DIR;
			return &enumerator->lastItem;
		}
	}

	return nullptr;
}

bool FATFS_MakeDirectory(utf8_int8_t const * dirpath_) {
	return(f_mkdir(dirpath_) == FR_OK);
}
bool FATFS_Delete(utf8_int8_t const * path_) {
	return(f_unlink(path_) == FR_OK);
}
bool FATFS_Rename(utf8_int8_t const * oldpath_, utf8_int8_t const * newpath_) {
	return(f_rename(oldpath_, newpath_) == FR_OK);
}
