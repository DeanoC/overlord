#include "core/core.h"
#include "core/utf8.h"
#include "dbg/print.h"
#include "memory/memory.h"
#include "vfile/vfile.h"
#include "data_utils/lz4.h"
#include "resource_bundle/resource_bundle.h"

static enum ResourceBundle_ErrorCode DoFixups(ResourceBundle_Header * const actualHeader,
                     ResourceBundle_CompressionHeader const * const compressionHeader,
                     uint8_t const * const uncompressedBase,
                     uint32_t const sizeWithoutHeader
										 ) {
	uint32_t const * const fixupTable = (uint32_t const * const)(uncompressedBase + compressionHeader->fixupOffset);

	if (actualHeader->flags & RBHF_64BitFixups) {
		// 64 bit fixups
		uintptr_all_t const lowerBound = (uintptr_all_t)(uintptr_t)(uncompressedBase);
		uintptr_all_t const upperBound = (uintptr_all_t)(uintptr_t)(uncompressedBase + sizeWithoutHeader);
		for(size_t fi = 0; fi < compressionHeader->fixupCount; ++fi) {
			uintptr_t const varAddress = (uintptr_t)(uncompressedBase + fixupTable[fi]);
			// check the fixup address itself valid
			if (varAddress >= upperBound || varAddress < lowerBound) {
				debug_printf("fixup error var address %#018lx a!\n", varAddress);
				return RBEC_CorruptError;
			}
			// check its aligned correctly
			if ((varAddress & 0x7) != 0) {
				debug_printf("fixup alignment error var address %#018lx a!\n", varAddress);
				return RBEC_CorruptError;
			}

			uintptr_all_t * const varPtr = ((uintptr_all_t *)varAddress);
			uintptr_all_t offset = *varPtr;
			//debug_printf("fixup %zu, table %i varAddress %lx offset %lx\n", fi, fixupTable[fi], varAddress, offset);
			// check the place we are going to store the pointer address in is valid
			if (offset >= sizeWithoutHeader) {
				debug_printf("fixup error var address %#018lx offset %#018lx b\n", varAddress, offset);
				return RBEC_CorruptError;
			}
			*varPtr = (uintptr_all_t)(uintptr_t) (uncompressedBase + offset);
			// check the address we point to is valid;
			if (*varPtr >= upperBound || *varPtr < lowerBound) {
				debug_printf("fixup error c!\n");
				return RBEC_CorruptError;
			}
		}
	} else {
		// 32 bit fixups
		// if we are loaded above 32 bit address space these fixups won't work so test and fail
		if((uint64_t)uncompressedBase >= (1ULL << 32ULL) || ((uint64_t)uncompressedBase + (uint64_t)sizeWithoutHeader) >= (1ULL << 32ULL)){
			return RBEC_MemoryError;
		}
		uintptr_lo_t const lowerBound = (uintptr_lo_t)(uintptr_t)(uncompressedBase);
		uintptr_lo_t upperBound = (uintptr_lo_t)(uintptr_t)(uncompressedBase + sizeWithoutHeader);
		for(size_t fi = 0; fi < compressionHeader->fixupCount; ++fi) {
			uintptr_t const varAddress = (uintptr_t)(uncompressedBase + fixupTable[fi]);
			uintptr_lo_t * const varPtr = ((uintptr_lo_t *)varAddress);
			if (varAddress >= upperBound || varAddress < lowerBound) {
				return RBEC_CorruptError;
			}

			uintptr_lo_t offset = *varPtr;
			if (offset >= upperBound || varAddress < lowerBound) {
				return RBEC_CorruptError;
			}
			*varPtr = (uintptr_lo_t)(uintptr_t) (uncompressedBase + offset);
			if (*varPtr >= upperBound || *varPtr < lowerBound) {
				return RBEC_CorruptError;
			}
		}
	}

	return RBEC_Okay;
}

ResourceBundle_LoadReturn ResourceBundle_Load(VFile_Handle fileHandle,
																					 Memory_Allocator* allocator,
																					 Memory_Allocator* tempAllocator) {
	ResourceBundle_LoadReturn ret;
	LZ4_SetAllocator( allocator );
	// read header and validate it
	ResourceBundle_Header header;
	if(VFile_Read(fileHandle, &header, sizeof(header)) != sizeof (header)) {
		ret.errorCode = RBEC_ReadError;
		return ret;
	}
	if(header.magic != ('B' << 0ul | 'U' << 8ul | 'N' << 16ul | 'D' << 24ul)) {
		char magic[5];
		memcpy(magic, &header.magic, 4);
		magic[5] = 0;
		debug_printf("Wrong magic (%s) in resource bundle\n", magic);
		ret.errorCode = RBEC_CorruptError;
		return ret;
	}
	if( header.majorVersion != ResourceBundle_MajorVersion ) {
		debug_printf("Wrong version (%i.%i) in resource bundle\n", header.majorVersion, header.minorVersion);
		ret.errorCode = RBEC_WrongVersion;
		return ret;
	}

	// allocate the entire memory for the bundle and the temporary decompression buffer
	void* decompressionBuffer = MALLOC(tempAllocator, header.decompressionBlockSize);
	void* bundleMemory = MALLOC(allocator, header.uncompressedSize);
	if(!decompressionBuffer || !bundleMemory) {
		debug_printf("Out of Memory decomp buffer %p or bundle %p\n",decompressionBuffer, bundleMemory);
		ret.errorCode = RBEC_MemoryError;
		MFREE(allocator, bundleMemory);
		MFREE(allocator, decompressionBuffer);
		return ret;
	}

	ResourceBundle_Header * const actualHeader = (ResourceBundle_Header*)bundleMemory;
	memcpy(actualHeader, &header, sizeof(header));

	uint8_t const * uncompressedBase = (uint8_t *) (actualHeader+1);
	uint32_t const sizeWithoutHeader = actualHeader->uncompressedSize - sizeof(header);

	// decompress rest of the bundle
	LZ4_FrameDecompress(fileHandle, (void*)uncompressedBase, sizeWithoutHeader, decompressionBuffer, actualHeader->decompressionBlockSize, tempAllocator);

	MFREE(tempAllocator, decompressionBuffer);

	// decode compression header, get directory, string table and fixup table
	ResourceBundle_CompressionHeader const * const compressionHeader = (ResourceBundle_CompressionHeader *)uncompressedBase;
	enum ResourceBundle_ErrorCode fixUpResult = DoFixups(actualHeader, compressionHeader, uncompressedBase, sizeWithoutHeader);
	if(fixUpResult != RBEC_Okay) {
		ret.errorCode = fixUpResult;
		MFREE(allocator, bundleMemory);
		return ret;
	} else {
		ret.resourceBundle = actualHeader;
		ret.errorCode = RBEC_Okay;
		return ret;
	}

}
uint32_t ResourceBundle_GetDirectoryCount(ResourceBundle_Header* header) {
	ResourceBundle_CompressionHeader const * const compressionHeader = (ResourceBundle_CompressionHeader *)(header+1);
	return compressionHeader->directoryCount;
}
ResourceBundle_DirectoryEntry const* ResourceBundle_GetDirectory(ResourceBundle_Header* header, uint32_t index) {
	uint8_t const * uncompressedBase = (uint8_t *) (header+1);
	ResourceBundle_CompressionHeader const * const compressionHeader = (ResourceBundle_CompressionHeader *)(uncompressedBase);
	assert(index < compressionHeader->directoryCount);
	ResourceBundle_DirectoryEntry const * directoryEntry = (ResourceBundle_DirectoryEntry const *)(uncompressedBase + compressionHeader->directoryOffset);
	return directoryEntry+index;
}
