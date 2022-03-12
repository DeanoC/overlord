#include "data_utils/lz4.h"
#include "lz4.h"
#include "lz4hc.h"
#include "xxhash.h"

size_t LZ4_Compress(uint8_t const* src, uint8_t* dst, size_t srcSize, size_t dstCapacity) {
	return LZ4_compress_default((char const*)src, (char*)dst, (int)srcSize, (int)dstCapacity);
}
size_t LZ4_CompressHigh (uint8_t const* src, uint8_t * dst, size_t srcSize, size_t dstCapacity, int compressionLevel) {
	return LZ4_compress_HC((char const*)src, (char*)dst, (int)srcSize, (int)dstCapacity, compressionLevel );
}
size_t LZ4_Decompress(uint8_t const* src, uint8_t* dst, size_t compressedSize, size_t dstCapacity) {
	return LZ4_decompress_safe((char const*)src, (char*)dst, (int)compressedSize, (int)dstCapacity);
}
size_t LZ4_CompressionBoundFromInputSize(size_t inputSize) {
	return LZ4_compressBound((int)inputSize);
}
uint64_t XXHash_Compute(uint64_t seed, void const* src, size_t length ) {
	return XXH64(src, length, seed);
}
