#pragma once

#include "core/core.h"
#include "core/cpp/bitops.h"
#include "core/cpp/atomics.h"
#include "platform/cpu.h"
#include "dbg/raw_print.h"
#include "dbg/print.h"
#include "dbg/assert.h"

template<int BLOCKSIZE_IN_BYTES, int BMA_BLOCK_COUNT>
struct BitmapAllocator {
#if CPU_MAX_ATOMIC_WIDTH >= 128 && CPU_MAX_BITOP_TYPE_WIDTH >= 128
	typedef uint128_t BitMapType;
#elif CPU_MAX_ATOMIC_WIDTH == 64 && CPU_MAX_BITOP_TYPE_WIDTH >= 64
	typedef uint64_t BitMapType;
#elif CPU_MAX_ATOMIC_WIDTH == 32 && CPU_MAX_BITOP_TYPE_WIDTH >= 32
	typedef uint32_t BitMapType;
#else
#error Bitmap_Allocator requires at least 32 bit atomics
#endif

	static const int BitBlockCount = ((BMA_BLOCK_COUNT/(CPU_MAX_BITOP_TYPE_WIDTH/2))+1);
	static const uint8_t AllocMaskBoundrary = (1 << 7);
	uintptr_t blockBaseAddr;
	BitMapType bitmap[BitBlockCount];
	uint8_t allocMasks[BitBlockCount];
	uint32_t lowMemorySearchMutex;

	void Init(uintptr_t baseBlockAddr);
	uintptr_t AllocOne();
	uintptr_t Alloc(uint32_t blockCount);
	void Free(uintptr_t address);

};

template<int BLOCKSIZE_IN_BYTES, int BMA_BLOCK_COUNT>
void BitmapAllocator<BLOCKSIZE_IN_BYTES, BMA_BLOCK_COUNT>::Init(uintptr_t baseBlockAddr) {
	this->blockBaseAddr = baseBlockAddr;
	memset(&this->bitmap, 0xFF, sizeof(this->bitmap));
	memset(&this->allocMasks, 0, sizeof(this->allocMasks));
}

template<int BLOCKSIZE_IN_BYTES, int BLOCK_COUNT>
uintptr_t BitmapAllocator<BLOCKSIZE_IN_BYTES, BLOCK_COUNT>::AllocOne() {
	const auto mask = 0x1;
	unsigned int i = 0;
	for(; i < BitBlockCount;++i) {
		restart:;
		const BitMapType blockBitmap = this->bitmap[i];
		const auto first = BitOp::Clz<BitMapType>(blockBitmap);
		raw_debug_printf("bitmap 0x%x mask 0x%x\n", this->bitmap[i], mask);

		if(first != sizeof(BitMapType)) {
			const auto maskShift = ((sizeof(BitMapType) * 8) - first);
			// found update bitmap and return address of block
			if( Atomic::CompareExchange<BitMapType>(&this->bitmap[i], &blockBitmap, blockBitmap & ~(mask << maskShift)) == false)
				goto restart;
			else {
				const BitMapType index = (i * sizeof(BitMapType)) + first;
				this->allocMasks[index] = mask;
				raw_debug_printf("ALLOC address 0x%x blockBaseAddr 0x%x index %d blockIndex %d first %d\n", this->blockBaseAddr + (index * BLOCKSIZE_IN_BYTES), this->blockBaseAddr, index, i, first);
				return this->blockBaseAddr + (index * BLOCKSIZE_IN_BYTES);
			}
		}
	}
	return ~0;
}

template<int BLOCKSIZE_IN_BYTES, int BMA_BLOCK_COUNT>
uintptr_t BitmapAllocator<BLOCKSIZE_IN_BYTES, BMA_BLOCK_COUNT>::Alloc(const uint32_t blockCount) {
//	if(blockCount == 1) {
//		return AllocOne();
//	}
	assert(blockCount < 128);
	assert(blockCount < sizeof(BitMapType) * 8);

	const auto mask = BitOp::CountToRightmostMask<uint8_t>(blockCount);
	unsigned int i = 0;
	for(; i < BitBlockCount;++i) {
		restart:;
		const BitMapType blockBitmap = this->bitmap[i];
		const auto first = BitOp::FindFirstStringOfOnes<BitMapType>(blockBitmap, blockCount);
		debug_printf("ALLOC bitmap 0x%x mask 0x%x first %d\n", this->bitmap[i], mask, first);

		if(first != sizeof(BitMapType)) {
			const auto maskShift = 0;//BitOp::UpperShiftForFindFirstStringOfOnes<BitMapType>(blockBitmap, blockCount) - 1;
			// found update bitmap and return address of block
			if( Atomic::CompareExchange<BitMapType>(&this->bitmap[i], &blockBitmap, blockBitmap & ~(mask << maskShift)) == false) {
				goto restart;
			}
			else {
				const auto maskWidth = BitOp::PopulationCount<uint8_t>(mask);

				const BitMapType index = (i * sizeof(BitMapType)) + (first - maskWidth + 1);
				this->allocMasks[index] = mask;
				debug_printf("ALLOC bitmap 0x%x mask 0x%x\n", this->bitmap[i], mask);
				debug_printf("ALLOC address 0x%x blockBaseAddr 0x%x index %d blockIndex %d first %d maskShift %d\n", this->blockBaseAddr + (index * BLOCKSIZE_IN_BYTES), this->blockBaseAddr, index, i, first, maskShift);

				return this->blockBaseAddr + (index * BLOCKSIZE_IN_BYTES);
			}
		}
	}
#if 0 // TODO needs proving/testing first
	// if we get here all strings inside blocks have been searched and not found
	// BUT there could be some cross the boundary of the bitmaps. To search those
	// we use a mutex, so we only do this if the normal fast search when the first
	// search has failed
	const uint32_t zero = 0;
	while( Atomic::CompareExchange(&this->lowMemorySearchMutex, &zero, (uint32_t)1)) {
		// stall
	}

	const unsigned int halfBitWidth = sizeof(BitMapType) * (8/2);
	for(unsigned int j = 0; j < BitBlockCount; ++j) {
		BitMapType bitmap0 = this->bitmap[j];
		BitMapType bitmap1 = this->bitmap[j+1];
		auto halfMask = BitOp::CountToRightmostMask<BitMapType>(halfBitWidth);
		BitMapType bits = ((bitmap0 & halfMask)  >> halfBitWidth) |
				((bitmap1 & ~halfMask) >> halfBitWidth);

		const auto first = (unsigned int) BitOp::FindFirstStringOfOnes<BitMapType>(bits, blockCount);
		if(first != sizeof(BitMapType)) {
			const int topbit = ((int)first - (int)halfBitWidth);
			const int botbit = ((int)first - (int)blockCount - (int)halfBitWidth);

			const BitMapType topmask = (botbit < 0) ? mask >> -botbit : mask << botbit;
			const BitMapType botmask = (botbit < 0) ? mask << (sizeof(BitMapType) + botbit) : 0;

			this->bitmap[j] =  bitmap0 & ~topmask;
			this->bitmap[j+1] =  bitmap1 & ~botmask;
			BitMapType index = ((j * sizeof(BitMapType)) + topbit);
			this->allocMasks[index] = mask | AllocMaskBoundrary;

			Atomic::Store(&this->lowMemorySearchMutex, (uint32_t)0);
			return this->blockBaseAddr + (index * BLOCKSIZE_IN_BYTES);
		}
	}

	Atomic::Store(&this->lowMemorySearchMutex, (uint32_t)0);
#endif

	return ~0;
}

template<int BLOCKSIZE_IN_BYTES, int BMA_BLOCK_COUNT>
void BitmapAllocator<BLOCKSIZE_IN_BYTES, BMA_BLOCK_COUNT>::Free(uintptr_t address) {
	const BitMapType index = ((address - this->blockBaseAddr) / BLOCKSIZE_IN_BYTES);
	const BitMapType blockIndex = (index / sizeof(BitMapType));

	if((this->allocMasks[index] & AllocMaskBoundrary) == 0) {
		const auto mask = (BitMapType) this->allocMasks[index];
		const auto maskWidth = BitOp::PopulationCount<uint8_t>(mask);
		const BitMapType first = maskWidth + index - (blockIndex * sizeof(BitMapType)) - 1;
		const auto maskShift = (sizeof(BitMapType) * 8) - (first + maskWidth);
		raw_debug_printf("FREE address 0x%x blockBaseAddr 0x%x index %d blockIndex %d first %d maskShift %d\n", address, this->blockBaseAddr, index, blockIndex, first, maskShift);
		raw_debug_printf("FREE bitmap 0x%x blockIndex %d first %d, actual mask 0x%x\n", this->bitmap[blockIndex], blockIndex, first, (mask << maskShift));
		Atomic::Or(&this->bitmap[blockIndex], (mask << maskShift));
		raw_debug_printf("FREE bitmap 0x%x mask 0x%x maskShift %d\n", this->bitmap[blockIndex], mask, maskShift);
	}
	else {
/*
		const uint32_t zero = 0;
		while( Atomic::CompareExchange(&this->lowMemorySearchMutex, &zero, (uint32_t)1)) {
			// stall
		}

		const unsigned int halfBitWidth = sizeof(BitMapType) * (8/2);
		const uint8_t mask = this->allocMasks[index] & ~AllocMaskBoundrary;
		const uint8_t blockCount = BitOp::RightmostMaskToCount(mask);

		const int botbit = ((int)first - (int)blockCount - (int)halfBitWidth);
		const BitMapType topmask = (botbit < 0) ? mask >> -botbit : mask << botbit;
		const BitMapType botmask = (botbit < 0) ? mask << (sizeof(BitMapType) + botbit) : 0;

		this->bitmap[index] = this->bitmap[index] | topmask;
		this->bitmap[index+1] = this->bitmap[index+1] | botmask;

		Atomic::Store(&this->lowMemorySearchMutex, (uint32_t)0);
		*/
assert(false);
	}
}