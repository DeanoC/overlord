#pragma once

#include "core/core.h"
#include "core/utf8.h"
#include "dbg/assert.h"
#include "memory/memory.h"
#include "vfile/vfile.h"

#ifdef __cplusplus
/// used to identify chunks, each should be unique to a project etc. IFF like
constexpr uint32_t operator "" _bundle_id(char const* s, size_t count)
{
	assert(count == 4);
	return s[3] << 24 | s[2] << 16 | s[1] << 8 | s[0] << 0;
}
#endif
#define RESOURCE_BUNDLE_ID(a,b,c,d) (uint32_t)(((d) << 24UL) | ((c) << 16UL)| ((b) << 8UL) | (a) >> 0UL)
#define RESOURCE_BUNDLE_ID_TO_STRING(s, id) s[0] = (id >> 0) & 0xFF; s[1] = (id >> 8) & 0xFF; s[2] = (id >> 16) & 0xFF; s[3] = (id >> 24) & 0xFF; s[4] = 0;


typedef enum ResourceBundle_HeaderFlags {
	RBHF_Is32Bit = (1 << 0),
	RBHF_64BitFixups = (1 << 1),
} ResourceBundle_HeaderFlags;

#define ResourceBundle_MajorVersion 0
#define ResourceBundle_MinorVersion 0

typedef struct PACKED ResourceBundle_Header {
		uint32_t magic;

		uint16_t flags;
		uint8_t majorVersion;
		uint8_t minorVersion;

		uint32_t uncompressedSize;
		uint32_t decompressionBlockSize;
} ResourceBundle_Header;
static_assert(sizeof(ResourceBundle_Header) == 16, "ResourceBundle_Header is not 16 bytes long");

typedef struct PACKED ResourceBundle_CompressionHeader {
		uint32_t directoryCount;
		uint32_t fixupCount;
		uint32_t directoryOffset;
		uint32_t fixupOffset;
} ResourceBundle_CompressionHeader;

static_assert(sizeof(ResourceBundle_CompressionHeader) == 16, "ResourceBundle_CompressionHeader is not 16 bytes long");

typedef struct PACKED ResourceBundle_DirectoryEntry
{
		uint32_t id;
		uint8_t version;
		uint8_t padd1;
		uint8_t padd2;
		uint8_t padd3;

		union {
				uintptr_all_t nameOffset;
				utf8_int8_t * namePtr;
		};
		union {
				uintptr_all_t storedOffset;
				void* chunkPtr;
		};
		uint64_t padd4;
} ResourceBundle_DirectoryEntry;
static_assert(sizeof(ResourceBundle_DirectoryEntry) == 32, "ResourceBundle_DirectoryEntry is not 32 bytes long");

typedef struct ResourceBundle_LoadReturn {
		enum ResourceBundle_ErrorCode
		{
				RBEC_Okay = 0,              // no error
				RBEC_WrongVersion = 1,      // Wrong Version
				RBEC_AddressLength = 2,     // address length issues probably 64 bit bundle on 32 bit system
				RBEC_ReadError = 3,         // stream issues
				RBEC_CorruptError = 4,      // failed internal crc checks
				RBEC_CompressionError = 5,  // decompression failed
				RBEC_MemoryError = 6,       // error allocating memory
				RBEC_OtherError = 7         // generic error
		} errorCode;
		ResourceBundle_Header* resourceBundle;
} ResourceBundle_LoadReturn;

EXTERN_C ResourceBundle_LoadReturn ResourceBundle_Load(VFile_Handle fileHandle, Memory_Allocator* allocator, Memory_Allocator* tempAllocator);
EXTERN_C uint32_t ResourceBundle_GetDirectoryCount(ResourceBundle_Header* header);
EXTERN_C ResourceBundle_DirectoryEntry const* ResourceBundle_GetDirectory(ResourceBundle_Header* header, uint32_t index);
