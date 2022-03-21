#include "data_utils/lz4.h"
#include "dbg/print.h"
#include "dbg/assert.h"
#include "lz4.h"
#include "lz4hc.h"
#include "lz4frame.h"
#include "xxhash.h"
#include "miniz.h"
#include "lz4frame.h"
#include "memory/memory.h"


typedef struct LZ4F_CompressionContext {
		LZ4F_cctx* lz4Ctx;
		Memory_Allocator * allocator;
		void* chunkMemory;
		LZ4_ChunkSize chunkSize;
		size_t outCapacity;
		VFile_Handle destFH;
} LZ4F_CompressionContext;

size_t LZ4_BlockCompress(uint8_t const* src, size_t srcSize, uint8_t* dst, size_t dstCapacity) {
	return LZ4_compress_default((char const*)src, (char*)dst, (int)srcSize, (int)dstCapacity);
}
size_t LZ4_BlockCompressHigh (uint8_t const* src, size_t srcSize, uint8_t * dst, size_t dstCapacity, int compressionLevel) {
	return LZ4_compress_HC((char const*)src, (char*)dst, (int)srcSize, (int)dstCapacity, compressionLevel );
}

size_t LZ4_BlockDecompressSafe(uint8_t const* src, size_t compressedSize, uint8_t* dst, size_t dstCapacity) {
	return LZ4_decompress_safe((char const*)src, (char*)dst, (int)compressedSize, (int)dstCapacity);
}

size_t LZ4_BlockCompressionBoundFromInputSize(size_t inputSize) {
	return LZ4_compressBound((int)inputSize);
}

static const LZ4F_preferences_t kPrefs = {
		{
				LZ4F_max64KB,
			LZ4F_blockLinked,
			LZ4F_noContentChecksum,
			LZ4F_frame,
		  0, // unknown content size
			 0,
			 LZ4F_noBlockChecksum
			 },
		0,   // compression level; 0 == default
		0,   // autoflush
		1,   // favor decompression speed
		{ 0, 0, 0 },  // reserved, must be set to 0
};

LZ4_FrameCompressionContext LZ4_CreateFrameCompressor(VFile_Handle dest_, LZ4_ChunkSize chunkSize_, Memory_Allocator* allocator_) {
	LZ4F_CompressionContext* ctx = (LZ4F_CompressionContext*) MALLOC(allocator_, sizeof(LZ4F_CompressionContext));
	if(!ctx) { return nullptr; }

	ctx->allocator = allocator_;
	size_t result = LZ4F_createCompressionContext(&ctx->lz4Ctx, LZ4F_VERSION);
	if(LZ4F_isError(result)) {
		debug_printf("LZ4 context error %zu\n", result);
		MFREE(allocator_, ctx);
		return nullptr;
	}
	// how big a compression buffer for any input < chunkSiz
	ctx->chunkSize = chunkSize_;
	ctx->outCapacity = LZ4F_compressBound(ctx->chunkSize, &kPrefs);
	ctx->chunkMemory = MALLOC(allocator_, ctx->outCapacity);
	ctx->destFH = dest_;
	size_t const headerSize = LZ4F_compressBegin(ctx->lz4Ctx, ctx->chunkMemory, ctx->outCapacity, &kPrefs);
	if (LZ4F_isError(headerSize)) {
		debug_printf("LZ4 header error %zu\n", headerSize);
		goto errorExit;
	}
	VFile_Write(ctx->destFH, ctx->chunkMemory, headerSize);

	return ctx;
errorExit:
	MFREE(allocator_, ctx->chunkMemory);
	MFREE(allocator_, ctx);
	return nullptr;
}

bool LZ4_FrameCompressNextChunk(LZ4_FrameCompressionContext ctx_, void* chunk_, size_t size_) {
	assert(size_ <= ctx_->chunkSize);
	size_t const compressedSize = LZ4F_compressUpdate(ctx_->lz4Ctx,
	                                                  ctx_->chunkMemory, ctx_->outCapacity,
	                                                  chunk_, size_,
	                                                  nullptr);
	if (LZ4F_isError(compressedSize)) {
		debug_printf("LZ4 Compression failed: error %zu\n", compressedSize);
		return false;
	}
	// 0 == just buffered
	if(compressedSize == 0) return true;
	VFile_Write(ctx_->destFH, ctx_->chunkMemory, compressedSize);
	return true;
}

void LZ4_FrameCompressFinishAndDestroy(LZ4_FrameCompressionContext ctx_) {
	if(ctx_) {
		size_t const compressedSize = LZ4F_compressEnd(ctx_->lz4Ctx,
		                                               ctx_->chunkMemory,   ctx_->outCapacity,
		                                               nullptr);
		if (LZ4F_isError(compressedSize)) {
			debug_printf("LZ4 Failed to end compression: error %zu \n",compressedSize);
		}
		VFile_Write(ctx_->destFH, ctx_->chunkMemory, compressedSize);
		LZ4F_freeCompressionContext(ctx_->lz4Ctx);
		MFREE(ctx_->allocator, ctx_->chunkMemory);
		MFREE(ctx_->allocator, ctx_);
	}
}

bool LZ4_FrameDecompress(VFile_Handle src_, void* dest, size_t destSize, void* chunkBuffer, LZ4_ChunkSize chunkSize_) {
	LZ4F_dctx* dctx = nullptr;
	size_t const dctxStatus = LZ4F_createDecompressionContext(&dctx, LZ4F_VERSION);
	if (LZ4F_isError(dctxStatus)) {
		debug_printf("LZ4 Frame Decompress context create error %zu \n",dctxStatus);
		return false;
	}

	int64_t const compressedFileStart = VFile_Tell(src_);

	if( VFile_Read(src_, chunkBuffer, LZ4F_MIN_SIZE_TO_KNOW_HEADER_LENGTH) == 0){
		debug_printf("LZ4 Frame header read fail error\n");
		LZ4F_freeDecompressionContext(dctx);
		return false;
	}
	VFile_Seek(src_, compressedFileStart, VFile_SD_Begin);
	size_t headerSize = LZ4F_headerSize(chunkBuffer, LZ4F_MIN_SIZE_TO_KNOW_HEADER_LENGTH);
	if(headerSize != VFile_Read(src_, chunkBuffer, headerSize)) {
		debug_printf("LZ4 Frame header read fail error\n");
		LZ4F_freeDecompressionContext(dctx);
		return false;
	}
	LZ4F_frameInfo_t info;
	size_t const fires = LZ4F_getFrameInfo(dctx, &info, chunkBuffer, &headerSize);
	if (LZ4F_isError(fires)) {
		debug_printf("LZ4F_getFrameInfo error: %s\n", LZ4F_getErrorName(fires));
		LZ4F_freeDecompressionContext(dctx);
		return false;
	}

	static const LZ4F_decompressOptions_t options = {
			.stableDst =1,
	};

	uint8_t * destPtr = (uint8_t *)dest;
	uint8_t * const destEndPtr = ((uint8_t *)dest) + destSize;
	size_t srcSizeHint = chunkSize_;
	while(destPtr < destEndPtr) {
		size_t const readSize = VFile_Read(src_, chunkBuffer, srcSizeHint);
		size_t decompressedSize = destEndPtr - destPtr;
		size_t srcSize = readSize;
		size_t ret = LZ4F_decompress(dctx, destPtr, &decompressedSize, chunkBuffer, &srcSize, &options);
		if (LZ4F_isError(ret)) {
			debug_printf("Decompression error: %s\n", LZ4F_getErrorName(ret));
			LZ4F_freeDecompressionContext(dctx);
			return false;
		}
		srcSizeHint = ret;
		//debug_printf("readSize %zu uncompressedLeft %zu, srcSize %zu decompressedSize %zu srcSizeHint %zu\n", readSize, destEndPtr - destPtr, srcSize, decompressedSize, srcSizeHint);
		destPtr += decompressedSize;
	}
	assert(srcSizeHint == 0);
	LZ4F_freeDecompressionContext(dctx);
	return true;
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