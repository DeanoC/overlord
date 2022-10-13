#include "core/core.h"
#include "platform/aarch64/intrinsics_gcc.h"
#include "platform/memory_map.h"
#include "platform/reg_access.h"
#include "platform/registers/a53_system.h"
#include "zynqps8/mmu/mmu.hpp"

EXTERN_C uint8_t _el3_stack_end[];

#define NUM_ENTRIES_PER_TABLE (512)
#define KB 1024ULL
#define MB 1048576ULL
#define GB 1073741824ULL

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

void EarlySetupMmu() {

	// we use 8K at the end of the stack to hold a L1 and L2 table
	// that is use during the PMU OS upload, after SetupMmu is called this
	// space returns to the stack

	uint64_t *l1TableAddr = (uint64_t *) _el3_stack_end;
	uint64_t *l2TableAddr = (uint64_t *) (_el3_stack_end + 4096);
	// L1 each entry 1GB, all but 1GB (upper GB of low memory) set to fault
	// 38GBs Cover DDR low and high and mmio
	for(int i = 0; i < 38; i++) {
		switch(i) {
			case 3: // 1GB Level2 PA: 3 to 4GB
				*l1TableAddr = (uintptr_all_t) l2TableAddr | TT_TABLE;
				break;
			default: // fault
				*l1TableAddr = FAULT;
				break;
		}
		l1TableAddr++;
	}

	// L2 each 2MB, device except for OCM/TCM entry
	for(int i = 0; i < 512; i++) {
		uint64_t const pa = 0xC000'0000 + (i * 2 * MB);
		*l2TableAddr = pa | ((i == 511) ? MEMORY : DEVICE);
		l2TableAddr++;
	}

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
	write_TTBR0_EL3_register( _el3_stack_end );
	isb();
	dsb();

	write_SCTLR_EL3_register(
		HW_REG_ENCODE_FIELD( A53_SYSTEM, SCTLR_EL3, SA, 1 ) |    // enable SP alignment check
		HW_REG_ENCODE_FIELD( A53_SYSTEM, SCTLR_EL3, M, 1 )       // enable MMU
	);

	dsb();
	isb();
}

Mmu::Manager * SetupMmu() {
	// map in 1GB memory DDR PA: 0 to 1GB
	*((uint64_t *) _el3_stack_end) = MEMORY;
	// invalid_all_TLB(); // this locks the machine up???
	dsb();
	isb();

	// with DDR memory access allowed we can set the real page tables that live in DDR
	// Mmu allocates blocks from PMU OS directly. 128KB are allocated at Init time
	auto mmu = Mmu::Init( false );

#define QSPIMEM_BASE_ADDR 0xC000'0000
#define QSPIMEM_SIZE_IN_BYTES (512 * MB)
#define LOWER_PCIE_BASE_ADDR 0xE000'0000
#define LOWER_PCIE_SIZE_IN_BYTES (256 * MB)
#define CORESIGHT_BASE_ADDR 0xF800'0000
#define CORESIGHT_SIZE_IN_BYTES (16 * MB)
#define RPU_LLP_BASE_ADDR 0xF900'0000
#define RPU_LLP_SIZE_IN_BYTES (1 * MB)
#define FPS_BASE_ADDR 0xFD00'0000
#define FPS_SIZE_IN_BYTES (16 * MB)
#define LPS_BASE_ADDR 0xFE00'0000
#define LPS_SIZE_IN_BYTES (28 * MB)
#define PMUCSU_BASE_ADDR 0xFFC0'0000
#define PMUCSU_SIZE_IN_BYTES (2 * MB)
#define OCMTCM_BASE_ADDR 0xFFE0'0000
#define OCMTCM_SIZE_IN_BYTES (2 * MB)

	Mmu::SetPageTypeRange( mmu, DDR_0_BASE_ADDR, MB, Mmu::PageType::Fault );
	Mmu::SetPageTypeRange( mmu, DDR_0_BASE_ADDR + MB, DDR_0_SIZE_IN_BYTES - MB, Mmu::PageType::NormalCached );
	Mmu::SetPageTypeRange( mmu, DDR_1_BASE_ADDR, DDR_1_SIZE_IN_BYTES, Mmu::PageType::NormalCached );

	Mmu::SetPageTypeRange( mmu, QSPIMEM_BASE_ADDR, QSPIMEM_SIZE_IN_BYTES, Mmu::PageType::Device );
	Mmu::SetPageTypeRange( mmu, LOWER_PCIE_BASE_ADDR, LOWER_PCIE_SIZE_IN_BYTES, Mmu::PageType::Device );
	Mmu::SetPageTypeRange( mmu, CORESIGHT_BASE_ADDR, CORESIGHT_SIZE_IN_BYTES, Mmu::PageType::Device );
	Mmu::SetPageTypeRange( mmu, RPU_LLP_BASE_ADDR, RPU_LLP_SIZE_IN_BYTES, Mmu::PageType::Device );
	Mmu::SetPageTypeRange( mmu, FPS_BASE_ADDR, FPS_SIZE_IN_BYTES, Mmu::PageType::Device );
	Mmu::SetPageTypeRange( mmu, LPS_BASE_ADDR, LPS_SIZE_IN_BYTES, Mmu::PageType::Device );
	Mmu::SetPageTypeRange( mmu, PMUCSU_BASE_ADDR, PMUCSU_SIZE_IN_BYTES, Mmu::PageType::Device );
	Mmu::SetPageTypeRange( mmu, OCMTCM_BASE_ADDR, OCMTCM_SIZE_IN_BYTES, Mmu::PageType::NormalCached );
	dsb();
	isb();

	Mmu::Enable(mmu);
	return mmu;
}
