//
// Created by deano on 8/21/22.
//
#include "core/core.h"
#include "dbg/assert.h"
#include "platform/aarch64/intrinsics_gcc.h"
#include "platform/memory_map.h"
#include "platform/reg_access.h"
#include "platform/registers/a53_system.h"
#include "osservices/osservices.h"
#include "dbg/print.h"
#include "dbg/ansi_escapes.h"
#include "zynqps8/mmu/mmu.hpp"


// upto 8 different memory access types
// 0 = b01000100 (0x44) = Normal, Inner/Outer Non-Cacheable
// 1 = b11111111 (0xFF) = Normal, Inner/Outer WB/WA/RA
// 2 = b00000000 (0x00) = Device-nGnRnE
// 3 = b00000100 (0x04) = Device-nGnRE
// 4 = b10111011 (0xBB) = Normal, Inner/Outer WT/WA/RA
#define TT_NormalNoCachable (0x0UL << 2)
#define TT_Normal           (0x1UL << 2)
#define TT_Device_nGnRnEl   (0x2UL << 2)
#define TT_Device_nGnRE     (0x3UL << 2)
#define TT_Normal_WT        (0x4UL << 2)
#define TT_AF               (0x1UL << 10)
#define TT_XN               (0x1UL << 54)
#define TT_NON_SHARED       (0x0UL << 8)
#define TT_INNER_SHARED     (0x3UL << 8)
#define TT_OUTER_SHARED     (0x2UL << 8)

// all descriptors are 64 bit
// fault at any level
// | ignored | 0x0|
#define TT_FAULT            0x0
// block L1, L2
// | Upper attibutes | | Output block address | | Lower attributes | 0x1 |
#define TT_BLOCK            0x1
// table descriptor L0, 1 and 2
// | Attributes |    | Next level table address |      | 0x3|
#define TT_TABLE            0x3
// page descriptor L3 only
// | Upper attibutes | | Output block address | | Lower attributes | 0x3 |
#define TT_PAGE             0x3
constexpr static uint32_t TT_TYPE_MASK = 0x3;
constexpr static uint32_t TT_ADDRESS_MASK = ~0x3;

#define FAULT     TT_FAULT
#define MEMORY    (TT_AF | TT_OUTER_SHARED  | TT_Normal | TT_BLOCK)
#define NON_CACHABLE_MEMORY    (TT_AF | TT_OUTER_SHARED  | TT_NormalNoCachable | TT_BLOCK)
#define DEVICE    (TT_XN | TT_AF | TT_Device_nGnRnEl | TT_BLOCK)

#define NUM_ENTRIES_PER_TABLE (512)
#define KB 1024ULL
#define MB 1048576ULL
#define GB 1073741824ULL

namespace Mmu {
// each block (64KB) supports 8 tables (L2 or L3)
struct Block {
	uintptr_lo_t blockAddr;
	uint16_t allocBitMask;
};

constexpr static uintptr_all_t L1_POST_MASK = 64 - 1;
constexpr static uintptr_all_t L2_POST_MASK = 512 - 1;
constexpr static uintptr_all_t L3_POST_MASK = 512 - 1;
constexpr static uintptr_all_t L1_LSHIFT = 30;
constexpr static uintptr_all_t L2_LSHIFT = 21;
constexpr static uintptr_all_t L3_LSHIFT = 12;

struct ManagerHeader {
	uintptr_all_t L1Table[64]; // L1 covers 64 GB (36 GB)
	uint32_t highBlock;
	uint32_t curBlock;
	uint32_t tablesFromHigh: 1;
};

struct Manager : public ManagerHeader {
	constexpr static int BlockCount = ((64 * 1024) - sizeof( ManagerHeader ));
	Block blocks[BlockCount / sizeof( Block )];
};
static_assert( sizeof( Manager ) == (64 * 1024));

ALWAYS_INLINE WARN_UNUSED_RESULT uintptr_all_t *GetL1Entry( Manager *mmu, uintptr_all_t va ) {
	assert( va < 64 * GB );
	return &mmu->L1Table[(va >> L1_LSHIFT) & L1_POST_MASK];
}

ALWAYS_INLINE WARN_UNUSED_RESULT uintptr_all_t *GetL2Entry( Manager *mmu, uintptr_all_t va ) {
	auto const l1Entry = GetL1Entry( mmu, va );
	if((*l1Entry & TT_TYPE_MASK) != TT_TABLE) return nullptr;
	auto const l2Table = (uintptr_all_t *) (*l1Entry & TT_ADDRESS_MASK);
	return &l2Table[(va >> L2_LSHIFT) & L2_POST_MASK];
}

ALWAYS_INLINE WARN_UNUSED_RESULT uintptr_all_t *GetL3Entry( Manager *mmu, uintptr_all_t va ) {
	auto const l2Entry = GetL2Entry( mmu, va );
	if((*l2Entry & TT_TYPE_MASK) != TT_TABLE) return nullptr;
	auto const l3Table = (uintptr_all_t *) (*l2Entry & TT_ADDRESS_MASK);
	return &l3Table[(va >> L3_LSHIFT) & L3_POST_MASK];
}

ALWAYS_INLINE WARN_UNUSED_RESULT uintptr_all_t *GetEntry( Manager *mmu, uintptr_all_t va ) {
	auto const l3Entry = GetL3Entry( mmu, va );
	if(l3Entry != nullptr) return l3Entry;
	auto const l2Entry = GetL2Entry( mmu, va );
	if(l2Entry != nullptr) return l2Entry;
	return GetL1Entry( mmu, va );
}

//
Manager *Init( bool tablesFromHigh ) {
	if(tablesFromHigh) {
		assert( sizeof( uintptr_t ) == sizeof( uint64_t ));
	}

	auto mmu = (Manager *) ((tablesFromHigh) ? OsService_DdrHiBlockAlloc( 1, OS_SERVICE_TAG('M', 'M', 'U', ' ') ) : OsService_DdrLoBlockAlloc( 1, OS_SERVICE_TAG('M', 'M', 'U', ' ') ));
	memset( mmu, 0, 64 * 1024 );
	mmu->tablesFromHigh = tablesFromHigh;
	mmu->curBlock = 0;
	mmu->highBlock = 0;
	return mmu;
}

uintptr_all_t AllocatePageTable( Manager *mmu ) {
	// search curBlock upto highBlock see if any spare pages are available
	while(mmu->curBlock < mmu->highBlock) {
		if(mmu->blocks[mmu->curBlock].allocBitMask == 0) mmu->curBlock++;
		else goto blockFound;
	}

	// no block found so a new block
	mmu->blocks[mmu->highBlock].blockAddr = (mmu->tablesFromHigh) ? OsService_DdrHiBlockAlloc( 1, OS_SERVICE_TAG('M', 'M', 'U', ' ') ) : OsService_DdrLoBlockAlloc( 1, OS_SERVICE_TAG('M', 'M', 'U', ' ') );
	mmu->blocks[mmu->highBlock].allocBitMask = 0xFFFF;
	mmu->curBlock = mmu->highBlock;
	mmu->highBlock++;
	blockFound:
	Block *block = mmu->blocks + mmu->curBlock;
	assert( block->allocBitMask != 0 );
	for(int i = 0; i < 16; ++i) {
		// is page free?
		if(block->allocBitMask & (1 << i)) {
			// claim it
			block->allocBitMask &= ~(1 << i);
			uintptr_all_t address = block->blockAddr + (i * 4096);
			memset((void *) address, 0, 4096 );
			return address;
		}
	}

	// should never happen!
	assert( false );
	return 0;
}

void ReleasePageTable( Manager *mmu, uintptr_all_t pageTableEntry ) {
	auto const blockAddr = (pageTableEntry & ~0xFFFF);
	uint32_t blockIndex = ~0;
	Block *block = nullptr;
	for(uint32_t i = 0; i < mmu->highBlock; ++i) {
		if(blockAddr == mmu->blocks[i].blockAddr) {
			block = &mmu->blocks[i];
			blockIndex = i;
			break;
		}
	}
	if(block == nullptr) {
		debug_printf( "ERROR: Page table block not found\n" );
		IKUY_DEBUG_BREAK();
		return;
	}

	uint8_t index = (pageTableEntry & 0xFFFF) >> 12;
	assert((block->allocBitMask & (1 << index)) == 0 );

	block->allocBitMask |= (1 << index);
	if(block->allocBitMask == 0xFFFF) {
		// free block
		if((mmu->highBlock - blockIndex) != 0)
			memcpy( block, block + 1, (mmu->highBlock - blockIndex) * sizeof( Block ));

		if(mmu->tablesFromHigh) OsService_DdrHiBlockFree( blockAddr, 1, OS_SERVICE_TAG('M', 'M', 'U', ' ') );
		else OsService_DdrLoBlockFree( blockAddr, 1, OS_SERVICE_TAG('M', 'M', 'U', ' ') );

		mmu->highBlock = mmu->highBlock - 1;
		if(mmu->curBlock > mmu->highBlock) mmu->curBlock = mmu->highBlock;
	}

}

void InstallPageTable( Manager *mmu,
                       uintptr_all_t va,
                       uintptr_all_t *tableEntry,
                       uint64_t const parentPASize ) {
	assert( tableEntry );
	// allocate a new page and set it up, point to it from the parent table
	auto const newPageTable = AllocatePageTable( mmu );
	if((*tableEntry & TT_TYPE_MASK) == TT_BLOCK) {
		// copy parent attributes to new table
		auto curL = (uintptr_all_t *) newPageTable;
		auto lva = va;
		// PA == VA so XOR out the address leaves the attributes
		auto const attributes = *tableEntry ^ va;
		// apply the attribute with the correct PA for each entry
		for(int i = 0; i < 512; ++i) {
			*curL = lva | attributes;
			curL++;
			lva += parentPASize / 512;
		}
	} else {
		assert((*tableEntry & TT_TYPE_MASK) == TT_FAULT );
		memset((void *) newPageTable, 0x0, 4096 );
	}

	auto cleanAddr = (uintptr_all_t *) ((uintptr_all_t) tableEntry & TT_ADDRESS_MASK);
	*cleanAddr = newPageTable | TT_TABLE;
}

// will install an L2 if one doesn't exist for this va
uintptr_all_t *InstallOrFetchL2( Manager *mmu, uintptr_all_t va ) {
	// if L2 exists for this address return it
	auto l2Entry = GetL2Entry( mmu, va );
	if(!l2Entry) {
		auto const l1Entry = GetL1Entry( mmu, va );
		assert((*l1Entry & TT_TYPE_MASK) != TT_TABLE );
		InstallPageTable( mmu, va, l1Entry, GB );
		l2Entry = GetL2Entry( mmu, va );
		assert( l2Entry );
	}
	return l2Entry;
}

// will install an L3 if one doesn't exist for this va
uintptr_all_t *InstallOrFetchL3( Manager *mmu, uintptr_all_t va ) {
	// if L3 exists for this address return it
	auto l3Entry = GetL3Entry( mmu, va );
	if(!l3Entry) {
		auto l2Entry = InstallOrFetchL2( mmu, va );
		assert((*l2Entry & TT_TYPE_MASK) != TT_TABLE );
		InstallPageTable( mmu, va, l2Entry, 2 * MB );
		l3Entry = GetL3Entry( mmu, va );
		assert( l3Entry );
	}
	return l3Entry;

}

void UninstallL2( struct Manager *mmu, uintptr_all_t va ) {
	auto l1entry = GetL1Entry( mmu, va );
	if((!l1entry || (*l1entry & TT_TYPE_MASK) != TT_TABLE)) return;

	// L2 might have L3 sub-tables, walk the table and release them if found
	auto *l2entry = (uintptr_all_t *) (*l1entry & TT_ADDRESS_MASK);
	for(int i = 0; i < 512; i++) {
		if((*l2entry & TT_TYPE_MASK) == TT_TABLE)
			ReleasePageTable( mmu, (*l2entry & TT_ADDRESS_MASK));
		l2entry++;
	}

	ReleasePageTable( mmu, (*l1entry & TT_ADDRESS_MASK));

}

void UninstallL3( struct Manager *mmu, uintptr_all_t va ) {
	auto l2entry = GetL2Entry( mmu, va );
	if((!l2entry || (*l2entry & TT_TYPE_MASK) != TT_TABLE)) return;
	// L3 can't have any sub tables so we can just release the sub table
	ReleasePageTable( mmu, *l2entry );
}

PageType GetPageType( Manager *mmu, uintptr_all_t va ) {
	uintptr_all_t *entry = GetEntry( mmu, va );
	if(!entry) return PageType::Fault;
	uint8_t e = *entry & (0x7 << 2); // 3 bits for type
	switch(e) {
		case TT_Normal: return PageType::NormalCached;
		case TT_NormalNoCachable: return PageType::NormalUnCached;
		case TT_Device_nGnRE:
		case TT_Device_nGnRnEl: return PageType::Device;
		default: return PageType::Fault;
	}
}

uintptr_all_t PageTableToEntry( uintptr_all_t pa, PageType pageType ) {
	switch(pageType) {
		case PageType::NormalCached: return pa | MEMORY;
		case PageType::NormalUnCached: return pa | NON_CACHABLE_MEMORY;
		case PageType::Device: return pa | DEVICE;
		default:
		case PageType::Fault: return FAULT;
	}
}

static constexpr uintptr_all_t Mask4K = 0xFFF;
static constexpr uintptr_all_t Mask2M = 0x1F'FFFF;
static constexpr uintptr_all_t Mask1G = 0x3FFF'FFFF;

void SetPageTypeRange( struct Manager *mmu, uintptr_all_t va, size_t sizeInBytes, PageType pageType ) {
	assert( sizeInBytes != 0 );
	assert((sizeInBytes & Mask4K) == 0 );
	assert((va & Mask4K) == 0 );

	// align front of address range
	while((va & Mask2M) != 0 && sizeInBytes > 4 * 1024) {
		// if there isn't a L3 install one
		auto entry = InstallOrFetchL3( mmu, va );
		*entry = PageTableToEntry( va, pageType ) | TT_PAGE;
		va += 4096;
		sizeInBytes -= 4096;
	}

	while((va & Mask1G) != 0 && sizeInBytes > 2 * 1024 * 1024) {
		// if there isn't a L2 install one
		auto entry = InstallOrFetchL2( mmu, va );
		// if the L2 entry has a L3 table uninstall it
		if(*entry & TT_TABLE) UninstallL3( mmu, *entry );
		*entry = PageTableToEntry( va, pageType );
		va += 2 * 1024 * 1024;
		sizeInBytes -= 2 * 1024 * 1024;
	}

	// -----
	// at this point we are guarenteed va is aligned to 1G, 2M or 4K
	// -----

	// use the largest pages we can, uninstall any existing tables when we replace them with larger tables
	while(sizeInBytes >= 1024 * 1024 * 1024) {
		auto entry = GetL1Entry( mmu, va );
		// if the L1 entry has a L2 table, uninstall it
		if(*entry & TT_TABLE) UninstallL2( mmu, *entry & TT_ADDRESS_MASK );
		*entry = PageTableToEntry( va, pageType );
		va += (1024 * 1024 * 1024);
		sizeInBytes -= (1024 * 1024 * 1024);
	}

	while(sizeInBytes >= 2 * 1024 * 1024) {
		// if there isn't a L2 install one
		auto entry = InstallOrFetchL2( mmu, va );
		// if the L2 entry has a L3 table uninstall it
		if(*entry & TT_TABLE) UninstallL3( mmu, *entry );
		*entry = PageTableToEntry( va, pageType );
		va += (2 * 1024 * 1024);
		sizeInBytes -= (2 * 1024 * 1024);
	}

	while(sizeInBytes >= 4 * 1024) {
		auto entry = InstallOrFetchL3( mmu, va );
		*entry = PageTableToEntry( va, pageType ) | TT_PAGE;
		va += (4 * 1024);
		sizeInBytes -= (4 * 1024);
	}

	assert( sizeInBytes == 0 );
}

char const * DumpType(uintptr_all_t entry) {
	if((entry & 0x3) == 0) return "";

	uint8_t e = entry & (0x7 << 2); // 3 bits for type
	switch(e) {
		case TT_Normal: return "Normal";
		case TT_NormalNoCachable: return "NormalNoCachable";
		case TT_Device_nGnRE: return "Device_nGnRE";
		case TT_Device_nGnRnEl: return "Device_nGnRnEl";
	}
	return "UNKNOWN";
}
char const * DumpEntry(uintptr_all_t entry, bool l3 ) {
	uint8_t e = entry & 0x3;
	switch(e) {
		case TT_FAULT: return "Fault";
 		case TT_TABLE: if(l3) return "Page"; else return "Table";
		case TT_BLOCK: return "Block";
		default: return "Fault";
	}
}


void DumpTlb(Manager * mmu, uintptr_all_t va) {
	auto const l1Entry = GetL1Entry( mmu, va );
	if(!l1Entry) {
		debug_printf(ANSI_RED_PEN "DumpTlb: L1 lookup failure of %p" ANSI_WHITE_PEN "\n", (void*)va);
		return;
	}
	debug_printf("TLB at %p:\n", (void*) va);
	debug_printf("  L1 %s %s (%#018lx)\n", DumpEntry(*l1Entry, false), DumpType(*l1Entry), *l1Entry );
	if((*l1Entry & TT_TYPE_MASK) == TT_TABLE) {
		auto const l2Entry = GetL2Entry( mmu, va );
		debug_printf("    L2 %s %s (%#018lx)\n", DumpEntry(*l2Entry, false), DumpType(*l2Entry), *l2Entry );
		if((*l2Entry & TT_TYPE_MASK) == TT_TABLE) {
			auto const l3Entry = GetL3Entry( mmu, va );
			debug_printf("      L3 %s %s (%#018lx) pa - %lu\n", DumpEntry(*l3Entry, true),
									 DumpType(*l3Entry),
									 *l3Entry,
									 ((*l3Entry >> L3_LSHIFT) & L3_POST_MASK) << L3_LSHIFT);
		}
	}
}

void Enable( Manager * mmu ) {
	// disable MMU
	write_SCTLR_EL3_register( HW_REG_ENCODE_FIELD( A53_SYSTEM, SCTLR_EL3, M, 0 ) );

	uint64_t tcr = 28;        // T0SZ = 28 = 64 - 36 for 36 bit VA space
	tcr = tcr | (2 << 16);    // PS = 1 = 36 bit PA space
	tcr = tcr | (1 << 20);    // TBI0 = 1 - ignore top byte
	write_TCR_EL3_register( tcr );
	isb();
	dsb();

	// upto 8 different memory access types
	// 0 = b01000100 (0x44) = Normal, Inner/Outer Non-Cacheable
	// 1 = b11111111 (0xFF) = Normal, Inner/Outer WB/WA/RA
	// 2 = b00000000 (0x00) = Device-nGnRnE
	// 3 = b00000100 (0x04) = Device-nGnRE
	// 4 = b10111011 (0xBB) = Normal, Inner/Outer WT/WA/RA
	write_MAIR_EL3_register( 0x0000'00BB'0400'FF44 );
	write_TTBR0_EL3_register( mmu->L1Table );

	write_SCTLR_EL3_register(
		HW_REG_ENCODE_FIELD( A53_SYSTEM, SCTLR_EL3, SA, 1 ) |    // SP alignment check
		HW_REG_ENCODE_FIELD( A53_SYSTEM, SCTLR_EL3, A, 0 ) |     // alignment check
		HW_REG_ENCODE_FIELD( A53_SYSTEM, SCTLR_EL3, M, 1 )       // enable MMU
	);

	invalid_all_TLB();
	dsb();
	isb();
}

} // end Mmu namespace
