#include "core/core.h"
#include "core/utf8.h"
#include "memory/memory.h"
#include "vfile/vfile.h"
#include "resource_bundle.h"

static const uint8_t ResourceBundle_MajorVersion = 1;
static const uint8_t ResourceBundle_MinorVersion = 0;

ResourceBundle_LoadReturn ResourceBundle_Load(VFile_Handle fileHandle,
																					 Memory_Allocator* allocator,
																					 Memory_Allocator* tempAllocator) {
	ResourceBundle_LoadReturn ret;

	// read header and validate it
	ResourceBundle_Header header;
	if(VFile_Read(fileHandle, &header, sizeof(header)) != sizeof (header)) {
		ret.errorCode = RBEC_ReadError;
		return ret;
	}
	if(header.magic != ('B' << 24ul | 'U' << 16ul | 'N' << 8ul | 'D' << 0ul)) {
		ret.errorCode = RBEC_CorruptError;
		return ret;
	}
	if( header.majorVersion != ResourceBundle_MajorVersion ||
			header.minorVersion > ResourceBundle_MinorVersion) {
		ret.errorCode = RBEC_WrongVersion;
		return ret;
	}

	// allocate the entire memory for the bundle and the temporary decompression buffer
	void* decompressionBuffer = MALLOC(tempAllocator, header.decompressionBufferSize);
	if(!decompressionBuffer) {
		ret.errorCode = RBEC_MemoryError;
		goto errorExit;
	}
	void* bundleMemory = MALLOC(allocator, header.uncompressedSize);
	if(!bundleMemory) {
		ret.errorCode = RBEC_MemoryError;
		goto errorExit;
	}
	ResourceBundle_Header * const actualHeader = (ResourceBundle_Header*)bundleMemory;
	memcpy(bundleMemory, &header, sizeof(header));
	actualHeader->allocator = allocator;

	// read the directory
	ResourceBundle_DiskDirEntry32 * const directoryHead = (ResourceBundle_DiskDirEntry32*)(actualHeader+1);
	uint32_t const directorySizeInBytes = actualHeader->directoryCount * sizeof(ResourceBundle_DiskDirEntry32);
	if(VFile_Read(fileHandle, directoryHead, directorySizeInBytes) != directorySizeInBytes) {
		ret.errorCode = RBEC_ReadError;
		goto errorExit;
	}
	// read the string table
	utf8_int8_t * const stringTableHead = ((uint8_t*)directoryHead) + actualHeader->stringTableSize;
	if(VFile_Read(fileHandle, stringTableHead, actualHeader->stringTableSize) != actualHeader->stringTableSize) {
		ret.errorCode = RBEC_ReadError;
		goto errorExit;
	}

	// now parse the directory
	for(uint32_t i = 0; i < actualHeader->directoryCount; ++i) {
		ResourceBundle_DiskDirEntry32 * const dir = (directoryHead + i);
		dir->namePtr = stringTableHead + dir->nameOffset;
		// seek to the start of the chunk (a sorted directory could make this a no-op)
		VFile_Seek(fileHandle, VFile_SD_Begin, dir->storedOffset);

		// read the chunk into bufferMemory
		void * const chunkDestAddress = ((uint8_t *)bundleMemory) + dir->storedOffset;
		if(VFile_Read(fileHandle, chunkDestAddress, dir->storedSize) != dir->storedSize) {
			ret.errorCode = RBEC_ReadError;
			goto errorExit;
		}

		if(dir->uncompressedSize != dir->storedSize) {
			// chunk is compressed
			// so we have to decompress to the temp buffer then copy that back over the top
			int okay = LZ4_decompress_safe((char*) chunkDestAddress,
			                               (char*) decompressionBuffer,
			                               (int) dir->storedSize,
			                               (int) dir->uncompressedSize);

			if(okay < 0 || okay != dir->uncompressedSize) {
				ret.errorCode = RBEC_CompressionError;
				goto errorExit;
			}
			memcpy(chunkDestAddress, decompressionBuffer, dir->uncompressedSize);
		}

		// do a crc check in case of corruption from somewhere
		uint32_t ucrc32c = crc32c_append(0, chunkDestAddress, dir->uncompressedSize);
		if(ucrc32c != dir->uncompressedCrc32c) return {
			ret.errorCode = RBEC_CorruptError;
			goto errorExit;
		};

		ResourceBundle_ChunkHeader32 * const chunk = (ResourceBundle_ChunkHeader32 *)chunkDestAddress;
		uint8_t * const chunkBase = (uint8_t*)chunkDestAddress;

		if (chunk->chunkHas64BitFixups) {
			// 64 bit fixups
			uintptr_all_t * fixupTable = (uintptr_all_t *) (chunkBase + chunk->fixupOffset);
			uintptr_all_t upperBound = (uintptr_all_t)(uintptr_t)(chunkBase + chunk->dataSize);
			for(size_t fi = 0; fi < chunk->fixupCount; ++fi) {
				uintptr_t const varAddress = (uintptr_t)(chunkBase + fixupTable[fi]);
				uintptr_all_t * const varPtr = ((uintptr_all_t *)varAddress);
				if (varAddress >= upperBound || varAddress == 0) {
					ret.errorCode = RBEC_CorruptError;
					goto errorExit;
				}

				uintptr_all_t offset = *varPtr;
				if (offset >= upperBound) {
					ret.errorCode = RBEC_CorruptError;
					goto errorExit;
				}
				*varPtr = (uintptr_all_t)(uintptr_t) (chunkBase + offset);
				if (*varPtr >= upperBound) {
					ret.errorCode = RBEC_CorruptError;
					goto errorExit;
				}
			}
		} else {
			// 32 bit fixups
			uintptr_lo_t * fixupTable = (uintptr_lo_t *) (chunkBase + chunk->fixupOffset);
			uintptr_lo_t upperBound = (uintptr_lo_t)(uintptr_t)(chunkBase + chunk->dataSize);
			for(size_t fi = 0; fi < chunk->fixupCount; ++fi) {
				uintptr_t const varAddress = (uintptr_t)(chunkBase + fixupTable[fi]);
				uintptr_lo_t * const varPtr = ((uintptr_lo_t *)varAddress);
				if (varAddress >= upperBound || varAddress == 0) {
					ret.errorCode = RBEC_CorruptError;
					goto errorExit;
				}

				uintptr_lo_t offset = *varPtr;
				if (offset >= upperBound) {
					ret.errorCode = RBEC_CorruptError;
					goto errorExit;
				}
				*varPtr = (uintptr_lo_t)(uintptr_t) (chunkBase + offset);
				if (*varPtr >= upperBound) {
					ret.errorCode = RBEC_CorruptError;
					goto errorExit;
				}
			}
		}
	}

	MFREE(tempAllocator, decompressionBuffer);
	ret.resourceBundle = actualHeader;
	ret.errorCode = RBEC_Okay;
	return ret;

errorExit:
	MFREE(tempAllocator, decompressionBuffer);
	MFREE(allocator, bundleMemory);
	return ret;
}