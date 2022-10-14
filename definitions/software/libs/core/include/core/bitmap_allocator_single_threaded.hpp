#pragma once

#include "core/core.h"
#include "core/bitops.hpp"
#include "platform/cpu.h"
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
	BitMapType bitmap[BitBlockCount];
	uintptr_t blockBaseAddr;
	uint32_t lastBitBlockWithSpace;

	void Init(uintptr_t baseBlockAddr);
	WARN_UNUSED_RESULT	uintptr_t Alloc(uint32_t blockCount);
	void Free(uintptr_t address, uint32_t blockCount);
	void Reset();
	void DebugDumpMasks(unsigned int blocksToDump_ = BitBlockCount);

};

template<int BLOCKSIZE_IN_BYTES, int BMA_BLOCK_COUNT>
void BitmapAllocator_SingleThreaded<BLOCKSIZE_IN_BYTES, BMA_BLOCK_COUNT>::Init(uintptr_t baseBlockAddr) {
	this->blockBaseAddr = baseBlockAddr;
	Reset();
}

template<int BLOCKSIZE_IN_BYTES, int BMA_BLOCK_COUNT>
void BitmapAllocator_SingleThreaded<BLOCKSIZE_IN_BYTES, BMA_BLOCK_COUNT>::Reset() {
	memset(&this->bitmap, 0xFF, sizeof(this->bitmap));
	this->lastBitBlockWithSpace = 0;
}


template<int BLOCKSIZE_IN_BYTES, int BMA_BLOCK_COUNT>
uintptr_t BitmapAllocator_SingleThreaded<BLOCKSIZE_IN_BYTES, BMA_BLOCK_COUNT>::Alloc(uint32_t blockCount_) {
	//debug_printf("Request for %lu blocks from bitmap allocator\n", blockCount_);

	uintptr_t retAddr = uintptr_t(~0);
	while(blockCount_ > 0) {
		uint32_t const blocks = blockCount_ >= CPU_MAX_BITOP_TYPE_WIDTH ? CPU_MAX_BITOP_TYPE_WIDTH : blockCount_;
		blockCount_ -= blocks;
		const auto mask = BitOp::CountToRightmostMask<BitMapType>(blocks);
		const auto maskWidth = BitOp::Pop(mask);
//		debug_printf("blockCount_ %i blocks %i mask %x maskWidth %i\n", blockCount_, blocks, mask, maskWidth);

		// start from last bit block that has some space and loop back with modulo
		for(unsigned int i = this->lastBitBlockWithSpace; i < BitBlockCount; ++i) {
			unsigned int currentBitBlock = i & BitOp::PowerOfTwoToMask(BitBlockCount);
			if (this->bitmap[currentBitBlock] == 0)
				continue;

			BitMapType const first = BitOp::FindFirstStringOfOnes<BitMapType>(this->bitmap[currentBitBlock], blocks);
//			debug_printf("ALLOC bitmap 0x%08lx mask 0x%08lx maskWidth %u, first %u Clz %u\n", this->bitmap[i], mask, maskWidth, first, BitOp::Clz(this->bitmap[currentBitBlock]));
			if (first < sizeof(BitMapType) * 8) {
				// found update bitmap and return address of block
				this->bitmap[currentBitBlock] = this->bitmap[currentBitBlock] & ~(mask << ((BitBlockCount-maskWidth) - first));
				if (this->bitmap[currentBitBlock] != 0)
					this->lastBitBlockWithSpace = currentBitBlock;
				else
					this->lastBitBlockWithSpace = (currentBitBlock + 1) & BitOp::PowerOfTwoToMask(BitBlockCount);

				const BitMapType blockIndex = currentBitBlock * BitMapTypeBits;
				const BitMapType index = blockIndex + first;
				const auto address = this->blockBaseAddr + (index * BLOCKSIZE_IN_BYTES);

				if(retAddr == uintptr_t(~0)) {
					retAddr = address;
	//				debug_printf("redAddr %p\n", retAddr);
				}
				//			debug_printf("ALLOC bitmap 0x%08lx index %ld\n", this->bitmap[i], ( (sizeof(BitMapType) * 8) - (first + maskWidth)) );
				//			debug_printf("ALLOC address 0x%lx blockBaseAddr 0x%x index %ld i %d first %ld\n", address, this->blockBaseAddr, index, i, first);
//				debug_printf("address %0x blockCount %i index %i blockIndex %i\n", address, blockCount_, index, blockIndex);
//				DebugDumpMasks(5);
				break;
			} else {
				// handle crossing the boundary of the memory space we control
				if (((currentBitBlock + 1) & BitOp::PowerOfTwoToMask(BitBlockCount)) != (currentBitBlock + 1)) {
					continue;
				}

				// if bottom half of this bitmap is 0, no need to do cross boundary stuff
				if (!(this->bitmap[currentBitBlock] & BitOp::CountToRightmostMask(BitMapTypeBits / 2)))
					continue;

				// form a bitmap of lower N bits of i and upper N bits of i+1
				BitMapType combined = (this->bitmap[currentBitBlock] << (BitMapTypeBits / 2))
						| (this->bitmap[currentBitBlock + 1] >> (BitMapTypeBits / 2));
				const auto cfirst = BitOp::FindFirstStringOfOnes<BitMapType>(combined, blocks);
//				debug_printf("ALLOC combined 0x%08x mask 0x%08x, cfirst %d\n", combined, mask, cfirst);
				if (cfirst != ~0U) {
					// this can't happen, we've skipped this part if lower bits of block is 0
					assert( cfirst < BitMapTypeBits / 2);

					combined = combined & ~(mask << (cfirst - maskWidth));
					//				debug_printf("ALLOC combined 0x%08x\n", combined);
					this->bitmap[currentBitBlock] =
							(this->bitmap[currentBitBlock] & ~BitOp::CountToRightmostMask(BitMapTypeBits / 2))
									| (combined >> (BitMapTypeBits / 2));
					this->bitmap[currentBitBlock + 1] =
							(this->bitmap[currentBitBlock + 1] & BitOp::CountToRightmostMask(BitMapTypeBits / 2))
									| (combined << (BitMapTypeBits / 2));

					if (this->bitmap[currentBitBlock + 1] != 0)
						this->lastBitBlockWithSpace = currentBitBlock + 1;
					else
						this->lastBitBlockWithSpace = (currentBitBlock + 2) & BitOp::PowerOfTwoToMask(BitBlockCount);

					const BitMapType blockIndex = currentBitBlock * BitMapTypeBits;
					const BitMapType index = blockIndex + (BitMapTypeBits - cfirst) + (BitMapTypeBits / 2);
					const auto address = this->blockBaseAddr + (index * BLOCKSIZE_IN_BYTES);
					if(retAddr == uintptr_t(~0)) {
						retAddr = address;
					}

					//				debug_printf("ALLOC bitmapT 0x%08x bitmapB 0x%08x index %d\n", this->bitmap[i], this->bitmap[i+1], index);
					//				debug_printf("ALLOC address 0x%x blockBaseAddr 0x%x index %d i %d cfirst %d\n", address, this->blockBaseAddr, index, i, cfirst);
					//DebugDumpMasks(5);
					break;
				}
			}
		}
	}
	if(retAddr != uintptr_t(~0)) {
		return retAddr;
	} else {
		debug_printf("ALLOC Out of bitblocks!\n");
		return ~0;
	}
}

template<int BLOCKSIZE_IN_BYTES, int BMA_BLOCK_COUNT>
void BitmapAllocator_SingleThreaded<BLOCKSIZE_IN_BYTES, BMA_BLOCK_COUNT>::Free(uintptr_t const address_, uint32_t const blockCount_) {

	if( this->lastBitBlockWithSpace < ((address_ - this->blockBaseAddr) / BLOCKSIZE_IN_BYTES) / BitMapTypeBits) {
		this->lastBitBlockWithSpace = (address_ - this->blockBaseAddr) / BLOCKSIZE_IN_BYTES;
	}

#if 1
	BitMapType address = (address_ - this->blockBaseAddr) / BLOCKSIZE_IN_BYTES;
	for(uint32_t i = 0; i < blockCount_;i++) {
		uintptr_t const offset = address;
		BitMapType const blockIndex = offset / BitMapTypeBits;
		BitMapType const bitIndex = BitMapTypeBits - (offset % BitMapTypeBits) - 1;
//		debug_printf("offset %i index %i bitIndex %i\n", offset, blockIndex , bitIndex);
//		DebugDumpMasks(5);
		assert_msg(!(this->bitmap[blockIndex] & (1 << bitIndex)), "Double Free");
		this->bitmap[blockIndex] |= (1 << bitIndex);
		address++;
	}
#else
//	debug_printf("FREE address 0x%x blockBaseAddr 0x%x index %d blockIndex %d\n", address, this->blockBaseAddr, index, blockIndex);

	const auto mask = BitOp::CountToRightmostMask<BitMapType>(blockCount_);
	const auto maskWidth = BitOp::Pop(mask);
//		debug_printf("FREE mask 0x%x maskWidth %d\n", mask, maskWidth);

	const BitMapType first = ((sizeof(BitMapType) * 8) - ((index - (blockIndex * BitMapTypeBits))) - maskWidth);
//		debug_printf("FREE bitmap 0x%x blockIndex %d first %d, actual mask 0x%x\n", this->bitmap[blockIndex], blockIndex, first, (mask << first));
	assert_msg(!(this->bitmap[blockIndex] & (mask << first)), "Double Free");
	this->bitmap[blockIndex] |= (mask << first);
	this->lastBitBlockWithSpace = blockIndex;
//		debug_printf("FREE bitmap 0x%x mask 0x%x\n", this->bitmap[blockIndex], mask);
	} else {
		const auto mask = BitOp::CountToRightmostMask<BitMapType>(blockCount_);
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
#endif
}

template<int BLOCKSIZE_IN_BYTES, int BMA_BLOCK_COUNT>
void BitmapAllocator_SingleThreaded<BLOCKSIZE_IN_BYTES, BMA_BLOCK_COUNT>::DebugDumpMasks(unsigned int blocksToDump_) {
	unsigned int i = 0;
	debug_print("DebugDumpMasks");
	for(; i < blocksToDump_;++i) {
		if((i & 7) == 0) {
			debug_printf("\n%04d - ", i);
		}
		debug_printf("0x%08lx ", this->bitmap[i]);
	}
	debug_print("\n");
}