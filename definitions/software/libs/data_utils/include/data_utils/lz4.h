// BSD-2 Licensed (full license and info at end of file)

#pragma once

#include "core/core.h"
#include "memory/memory.h"
#include "vfile/vfile.h"
#define LZ4_MAX_COMPRESSION_LEVEL 12


typedef enum LZ4_ChunkSize {
		LZ4CS_16K = 16 * 1024,
		LZ4CS_64K = 64 * 1024,
} LZ4_ChunkSize;

// must before called compressed (per core) and not changed during compression
EXTERN_C void LZ4_SetAllocator(Memory_Allocator* allocator);

// de/compression where input/output where memory isn't constrained
EXTERN_C size_t LZ4_BlockCompress(uint8_t const* src, size_t srcSize, uint8_t* dst, size_t dstCapacity);
EXTERN_C size_t LZ4_BlockCompressHigh (uint8_t const* src, size_t srcSize, uint8_t * dst, size_t dstCapacity, int compressionLevel);
EXTERN_C size_t LZ4_BlockDecompressSafe(uint8_t const* src, size_t compressedSize, uint8_t* dst, size_t dstCapacity);
EXTERN_C size_t LZ4_BlockCompressionBoundFromInputSize(size_t inputSize);

typedef struct LZ4F_CompressionContext* LZ4_FrameCompressionContext;

EXTERN_C LZ4_FrameCompressionContext LZ4_CreateFrameCompressor(VFile_Handle dest_, LZ4_ChunkSize chunkSize_, Memory_Allocator* allocator_);
EXTERN_C bool LZ4_FrameCompressNextChunk(LZ4_FrameCompressionContext ctx_, void* chunk_, size_t size_);
EXTERN_C void LZ4_FrameCompressFinishAndDestroy(LZ4_FrameCompressionContext ctx_);

EXTERN_C bool LZ4_FrameDecompress(VFile_Handle src_, void* dest, size_t destSize, void* chunkBuffer, LZ4_ChunkSize chunkSize_, Memory_Allocator * allocator);

/*
 Small modifications by Deano Calver to the code by Mark and Robert just compat stuff
 Includes lz4 by Yann Collet - LZ4 source repository : https://github.com/lz4/lz4

   BSD 2-Clause License (http://www.opensource.org/licenses/bsd-license.php)

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are
   met:

       * Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
       * Redistributions in binary form must reproduce the above
   copyright notice, this list of conditions and the following disclaimer
   in the documentation and/or other materials provided with the
   distribution.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/