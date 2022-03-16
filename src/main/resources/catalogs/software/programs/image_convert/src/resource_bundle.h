#pragma once

#include "core/core.h"
#include "core/utf8.h"
#include "dbg/assert.h"

#ifdef __cplusplus
/// used to identify chunks, each should be unique to a project etc. IFF like
constexpr uint32_t operator "" _bundle_id(char const* s, size_t count)
{
	assert(count == 4);
	return s[3] << 24 | s[2] << 16 | s[1] << 8 | s[0] << 0;
}
#endif
#define RESOURCE_BUNDLE_ID(a,b,c,d) (uint32_t)(((d) << 24UL) | ((c) << 16UL)| ((b) << 8UL) | (a) >> 0UL)

typedef enum ResourceBundle_HeaderFlags {
	RBHF_Is32Bit = (1 << 0),
} ResourceBundle_HeaderFlags;

#define ResourceBundle_MajorVersion 1
#define ResourceBundle_MinorVersion 0


typedef struct PACKED ResourceBundle_Header {
		uint32_t magic;

		uint16_t flags;
		uint8_t majorVersion;
		uint8_t minorVersion;

		uint32_t directoryCount;
		uint32_t stringTableSize;
		uint32_t uncompressedSize;
		uint32_t decompressionBufferSize;

		union {
				uintptr_all_t padd0;
				struct Memory_Allocator* allocator;
		};
} ResourceBundle_Header;
static_assert(sizeof(ResourceBundle_Header) == 32, "ResourceBundle_Header is not 32 bytes long");

typedef struct PACKED ResourceBundle_DiskDirEntry32
{
		uint32_t id;
		uint32_t uncompressedCrc32c;
		uint32_t flags;
		uint32_t uncompressedSize;

		union {
				struct {
						uint32_t nameOffset;
						uint32_t padd0;
				};
				utf8_int8_t * namePtr;
		};
		union {
				struct {
						uint32_t storedOffset;
						uint32_t storedSize;
				};
				void* chunkPtr;
		};
} ResourceBundle_DiskDirEntry32;
static_assert(sizeof(ResourceBundle_DiskDirEntry32) == 32, "ResourceBundle_DiskDirEntry32 is not 32 bytes long");

typedef enum ResourceBundle_ChunkFlags {
		RBCF_64BitFixups = (1 << 0),
} ResourceBundle_ChunkFlags;

typedef struct PACKED ResourceBundle_ChunkHeader32
{
		union {
				struct {
						uint32_t dataOffset;
						uint32_t dataSize;
				};
				void* dataPtr;
		};

		uint32_t fixupOffset;
		uint16_t fixupSize;
		uint8_t version;
		uint8_t flags;
} ResourceBundle_ChunkHeader32;
static_assert(sizeof(ResourceBundle_ChunkHeader32) == 16, "ResourceBundle_ChunkHeader32 is not 16 bytes long");

typedef struct ResourceBundle_LoadReturn {
		enum ResourceBundle_ErrorCode
		{
				RBEC_Okay = 0,              // no error
				RBEC_WrongVersion,          // Wrong Version
				RBEC_AddressLength,         // address length issues probably 64 bit bundle on 32 bit system
				RBEC_ReadError,             // stream issues
				RBEC_CorruptError,          // failed internal crc checks
				RBEC_CompressionError,      // decompression failed
				RBEC_MemoryError,           // error allocating memory
				RBEC_OtherError             // generic error
		} errorCode;
		ResourceBundle_Header* resourceBundle;
} ResourceBundle_LoadReturn;