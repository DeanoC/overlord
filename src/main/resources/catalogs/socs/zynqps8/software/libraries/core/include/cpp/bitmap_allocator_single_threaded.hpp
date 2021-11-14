#pragma once

#include "core/core.h"
#include "core/cpp/bitops.hpp"
#include "core/cpp/atomics.hpp"
#include "hw/cpu.h"
#include "dbg/raw_print.h"
#include "dbg/print.h"
#include "dbg/assert.h"

template<int BLOCKSIZE_IN_BYTES, int BMA_BLOCK_COUNT>
struct BitmapAllocator_SingleThreaded {
#if CPU_MAX_BITOP_TYPE_WIDTH >= 128
	typedef uint128_t BitMapType;
#elif CPU_MAX_BITOP_TYPE_WIDTH >= 64
	typedef uint64_t BitMapType;
#elif CPU_MAX_BITOP_TYPE_WIDTH >= 32
	typedef uint32_t BitMapType;
#else
#error Bitmap_Allocator requires at least 32 bitops
#endif

	static const int BitMapTypeBits = sizeof(BitMapType) * 8;
	static const int BitBlockCount = BitOp::PowerOfTwoContaining(BMA_BLOCK_COUNT / BitMapTypeBits);
	static const uint8_t AllocCountBoundary = (1 << 7);
	BitMapType bitmap[BitBlockCount];
	uint8_t allocCount[BitBlockCount];
	uintptr_t blockBaseAddr;
	uint32_t lastBitBlockWithSpace;

	void Init(uintptr_t baseBlockAddr);
	WARN_UNUSED_RESULT uintptr_t AllocOne();
	WARN_UNUSED_RESULT	uintptr_t Alloc(uint32_t blockCount);
	void Free(uintptr_t address);
	void Reset();
	void DebugDumpMasks();

};

template<int BLOCKSIZE_IN_BYTES, int BMA_BLOCK_COUNT>
void BitmapAllocator_SingleThreaded<BLOCKSIZE_IN_BYTES, BMA_BLOCK_COUNT>::Init(uintptr_t baseBlockAddr) {
	this->blockBaseAddr = baseBlockAddr;
	Reset();
}

template<int BLOCKSIZE_IN_BYTES, int BMA_BLOCK_COUNT>
void BitmapAllocator_SingleThreaded<BLOCKSIZE_IN_BYTES, BMA_BLOCK_COUNT>::Reset() {
	memset(&this->bitmap, 0xFF, sizeof(this->bitmap));
	memset(&this->allocCount, 0, sizeof(this->allocCount));
	this->lastBitBlockWithSpace = 0;
}

template<int BLOCKSIZE_IN_BYTES, int BMA_BLOCK_COUNT>
uintptr_t BitmapAllocator_SingleThreaded<BLOCKSIZE_IN_BYTES, BMA_BLOCK_COUNT>::AllocOne() {
	for(unsigned int i = this->lastBitBlockWithSpace; i < BitBlockCount; ++i) {
		unsigned int currentBitBlock = (i + this->lastBitBlockWithSpace) & BitOp::PowerOfTwoToMask(BitBlockCount);

		if(this->bitmap[currentBitBlock] == 0) continue;

		const auto first = BitOp::FindFirstStringOfOnes<BitMapType>(this->bitmap[currentBitBlock], 0x1);
		if(first != ~0UL) {
			// found update bitmap and return address of block
			this->bitmap[currentBitBlock] = this->bitmap[currentBitBlock] & ~(0x1 << first);
			const BitMapType blockIndex = currentBitBlock * BitMapTypeBits;
			const BitMapType index =  blockIndex + (BitMapTypeBits - (first + 1));
			const auto address = this->blockBaseAddr + (index * BLOCKSIZE_IN_BYTES);
			this->allocCount[index] = 1;
			if(this->bitmap[currentBitBlock] != 0) this->lastBitBlockWithSpace = currentBitBlock;
			else this->lastBitBlockWithSpace = (currentBitBlock + 1) & BitOp::PowerOfTwoToMask(BitBlockCount);

//			debug_printf("ALLOC bitmap 0x%08lx index %ld\n", this->bitmap[i], ( (sizeof(BitMapType) * 8) - (first + 1)) );
//			debug_printf("ALLOC address 0x%lx blockBaseAddr 0x%x index %ld i %d first %ld\n", address, this->blockBaseAddr, index, i, first);

			return address;
		}
	}
	return ~0;
}

template<int BLOCKSIZE_IN_BYTES, int BMA_BLOCK_COUNT>
uintptr_t BitmapAllocator_SingleThreaded<BLOCKSIZE_IN_BYTES, BMA_BLOCK_COUNT>::Alloc(const uint32_t blockCount) {
	if(blockCount == 1) {
		return AllocOne();
	}
	assert(blockCount < 128);
	assert(blockCount < sizeof(BitMapType) * 8);

	const auto mask = BitOp::CountToRightmostMask<uint32_t>(blockCount);
	const auto maskWidth = BitOp::Pop(mask);

	for(unsigned int i = this->lastBitBlockWithSpace; i < BitBlockCount; ++i) {
		unsigned int currentBitBlock = (i + this->lastBitBlockWithSpace) & BitOp::PowerOfTwoToMask(BitBlockCount);

		if(this->bitmap[currentBitBlock] == 0) continue;

		const auto first = BitOp::FindFirstStringOfOnes<BitMapType>(this->bitmap[currentBitBlock], blockCount);
//		debug_printf("ALLOC bitmap 0x%08lx mask 0x%08lx, first %ld\n", this->bitmap[i], mask, first);

		if(first != ~0UL) {
			// found update bitmap and return address of block
			this->bitmap[currentBitBlock] = this->bitmap[currentBitBlock] & ~(mask << first);
			const BitMapType blockIndex = currentBitBlock * BitMapTypeBits;
			const BitMapType index =  blockIndex + (BitMapTypeBits - (first + maskWidth));
			const auto address = this->blockBaseAddr + (index * BLOCKSIZE_IN_BYTES);
			this->allocCount[index] = blockCount;
			if(this->bitmap[currentBitBlock] != 0) this->lastBitBlockWithSpace = currentBitBlock;
			else this->lastBitBlockWithSpace = (currentBitBlock + 1) & BitOp::PowerOfTwoToMask(BitBlockCount);

//			debug_printf("ALLOC bitmap 0x%08lx index %ld\n", this->bitmap[i], ( (sizeof(BitMapType) * 8) - (first + maskWidth)) );
//			debug_printf("ALLOC address 0x%lx blockBaseAddr 0x%x index %ld i %d first %ld\n", address, this->blockBaseAddr, index, i, first);
			return address;
		} else {
			if(currentBitBlock+1 >= BitBlockCount) return ~0;
			// if bottom half of this bitmap is 0, no need to do cross boundary stuff
			if(!(this->bitmap[currentBitBlock] & BitOp::CountToRightmostMask(BitMapTypeBits/2))) continue;

			// form a bitmap of lower N bits of i and upper N bits of i+1
			BitMapType combined = (this->bitmap[currentBitBlock] << (BitMapTypeBits/2)) | (this->bitmap[currentBitBlock+1] >> (BitMapTypeBits/2));
			const auto cfirst = BitOp::FindFirstStringOfOnes<BitMapType>(combined, blockCount);
//			debug_printf("ALLOC combined 0x%08x mask 0x%08x, cfirst %d\n", combined, mask, cfirst);
			if(cfirst != ~0UL){
				assert(cfirst < BitMapTypeBits/2); // can't happen as this, we've skipped this part if lower bits of block is 0

				combined = combined & ~(mask << cfirst);
//				debug_printf("ALLOC combined 0x%08x\n", combined);
				this->bitmap[currentBitBlock] = (this->bitmap[currentBitBlock] & ~BitOp::CountToRightmostMask(BitMapTypeBits/2)) | (combined >> (BitMapTypeBits/2));
				this->bitmap[currentBitBlock+1] = (this->bitmap[currentBitBlock+1] & BitOp::CountToRightmostMask(BitMapTypeBits/2)) | (combined << (BitMapTypeBits/2));

				const BitMapType blockIndex = currentBitBlock * BitMapTypeBits;
				const BitMapType index =  blockIndex + (BitMapTypeBits - (cfirst + maskWidth))  + (BitMapTypeBits/2);
				const auto address = this->blockBaseAddr + (index * BLOCKSIZE_IN_BYTES);
				this->allocCount[index] = blockCount | AllocCountBoundary;

				if(this->bitmap[currentBitBlock+1] != 0) this->lastBitBlockWithSpace = currentBitBlock+1;
				else this->lastBitBlockWithSpace = (currentBitBlock + 2) & BitOp::PowerOfTwoToMask(BitBlockCount);
				//				debug_printf("ALLOC bitmapT 0x%08x bitmapB 0x%08x index %d\n", this->bitmap[i], this->bitmap[i+1], index);
//				debug_printf("ALLOC address 0x%x blockBaseAddr 0x%x index %d i %d cfirst %d\n", address, this->blockBaseAddr, index, i, cfirst);
				return address;

			}
		}
	}
	return ~0;
}

template<int BLOCKSIZE_IN_BYTES, int BMA_BLOCK_COUNT>
void BitmapAllocator_SingleThreaded<BLOCKSIZE_IN_BYTES, BMA_BLOCK_COUNT>::Free(uintptr_t address) {
	const BitMapType index = ((address - this->blockBaseAddr) / BLOCKSIZE_IN_BYTES);
	const BitMapType blockIndex = (index / BitMapTypeBits);
//	debug_printf("FREE address 0x%x blockBaseAddr 0x%x index %d blockIndex %d\n", address, this->blockBaseAddr, index, blockIndex);

	if((this->allocCount[index] & AllocCountBoundary) == 0) {
		const auto mask = BitOp::CountToRightmostMask<BitMapType>(this->allocCount[index]);
		const auto maskWidth = BitOp::Pop(mask);
//		debug_printf("FREE mask 0x%x maskWidth %d\n", mask, maskWidth);

		const BitMapType first = ((sizeof(BitMapType) * 8) - ((index - (blockIndex * BitMapTypeBits))) - maskWidth);
//		debug_printf("FREE bitmap 0x%x blockIndex %d first %d, actual mask 0x%x\n", this->bitmap[blockIndex], blockIndex, first, (mask << first));
		assert_msg(!(this->bitmap[blockIndex] & (mask << first)), "Double Free");
		this->bitmap[blockIndex] |= (mask << first);
		this->lastBitBlockWithSpace = blockIndex;
//		debug_printf("FREE bitmap 0x%x mask 0x%x\n", this->bitmap[blockIndex], mask);
	} else {
		const auto mask = BitOp::CountToRightmostMask<BitMapType>(this->allocCount[index]);
		const auto maskWidth = BitOp::Pop(mask);
		const BitMapType first = index - (blockIndex * BitMapTypeBits);
//		debug_printf("FREE first %d, mask 0x%x maskWidth %d\n", first, mask, maskWidth);

		const auto top = mask >> (maskWidth - (BitMapTypeBits - first));
		const auto bot = (mask >> ((BitMapTypeBits - first))) << BitOp::Clz(mask >> ((BitMapTypeBits - first)));
//		debug_printf("FREE top 0x%x bot 0x%x\n", top, bot);

		this->bitmap[blockIndex] |= top;
		this->bitmap[blockIndex+1] |= bot;
		this->lastBitBlockWithSpace = blockIndex;

//		debug_printf("FREE bitmapT 0x%08lx bitmapB 0x%08lx\n", this->bitmap[blockIndex], this->bitmap[blockIndex+1]);

	}
}

template<int BLOCKSIZE_IN_BYTES, int BMA_BLOCK_COUNT>
void BitmapAllocator_SingleThreaded<BLOCKSIZE_IN_BYTES, BMA_BLOCK_COUNT>::DebugDumpMasks() {
	unsigned int i = 0;
	debug_print("DebugDumpMasks\n");
	for(; i < BitBlockCount;++i) {
		if((i & 7) == 0) {
			debug_printf("\n%04d - ", i);
		}
		debug_printf("0x%08lx ", this->bitmap[i]);
	}
	debug_print("\n");
}