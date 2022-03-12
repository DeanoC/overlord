#include "data_utils/lz4.h"
#include "lz4.h"
#include "lz4hc.h"
#include "xxhash.h"
#include "miniz.h"

size_t LZ4_Compress(uint8_t const* src, size_t srcSize, uint8_t* dst, size_t dstCapacity) {
	return LZ4_compress_default((char const*)src, (char*)dst, (int)srcSize, (int)dstCapacity);
}
size_t LZ4_CompressHigh (uint8_t const* src, size_t srcSize, uint8_t * dst, size_t dstCapacity, int compressionLevel) {
	return LZ4_compress_HC((char const*)src, (char*)dst, (int)srcSize, (int)dstCapacity, compressionLevel );
}
size_t LZ4_Decompress(uint8_t const* src, size_t compressedSize, uint8_t* dst, size_t dstCapacity) {
	return LZ4_decompress_safe((char const*)src, (char*)dst, (int)compressedSize, (int)dstCapacity);
}
size_t LZ4_CompressionBoundFromInputSize(size_t inputSize) {
	return LZ4_compressBound((int)inputSize);
}

uint64_t XXHash_Compute(uint64_t seed, void const* src, size_t length ) {
	return XXH64(src, length, seed);
}

int Miniz_Compress(uint8_t const* src, size_t srcSize, uint8_t* dst, size_t dstCapacity) {
	mz_ulong dstSize = (mz_ulong)dstCapacity;

	if( mz_compress(dst, &dstSize, src, (mz_ulong)srcSize) != MZ_OK) {
		return 0;
	} else {
		return dstSize;
	}
}

int Miniz_Decompress(uint8_t const* src, size_t compressedSize, uint8_t* dst, size_t dstCapacity) {
	mz_ulong dstSize = (mz_ulong)dstCapacity;
	if( mz_uncompress(dst, &dstSize, src, (mz_ulong)compressedSize) != MZ_OK) {
		return 0;
	} else {
		return dstSize;
	}
}

size_t Miniz_CompressionBoundFromInputSize(size_t inputSize) {
	return mz_compressBound((mz_ulong)inputSize);
}