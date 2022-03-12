// SPDX-License-Identifier: MIT

#include "core/core.h"
#include "dbg/raw_print.h"
#include "dbg/print.h"
#include "dbg/ansi_escapes.h"
#include "platform/reg_access.h"
#include "platform/cache.h"
#include "platform/aarch64/intrinsics_gcc.h"
#include "platform/memory_map.h"
#include "hw_regs/csu.h"
#include "hw_regs/ipi.h"
#include "hw_regs/pmu_global.h"
#include "hw_regs/gic400.h"
#include "hw_regs/zdma.h"
#include "hw_regs/rpu.h"
#include "hw_regs/crl_apb.h"
#include "hw_regs/a53/a53_system.h"
#include "zynqps8/display_port/display.hpp"
#include "osservices/osservices.h"
#include "utils/busy_sleep.h"
#include "core/snprintf.h"

void PrintBanner(void);
void EnablePSToPL(void);
void ClearPendingInterrupts(void);
void MarkDdrAsMemory(void);

void EccInit(uint64_t DestAddr, uint64_t LengthBytes);
void PmuSleep();
void PmuWakeup();
void PmuSafeMemcpy(void * destination, const void* source, size_t num_in_bytes );

EXTERN_C {
	extern char _binary_pmu_monitor_bin_start[];
	extern char _binary_pmu_monitor_bin_end[];

	extern void RegisterBringUp(void);


	extern uint8_t HeapBase[];
	extern uint8_t HeapLimit[];

	extern uint8_t MMUTableL1[];
	extern uint8_t MMUTableL2[];
}

#define CPU_CORTEXA53_0_TIMESTAMP_CLK_FREQ 33333000

#define PSSYSMON_ANALOG_BUS_OFFSET		0x114U
#define ZDMA_ZDMA_CH_CTRL0_TOTAL_BYTE_OFFSET 0X0188U

// TODO get from memory map
#define PMU_RAM_ADDR 0xFFDC0000


DisplayPort::Display::Connection link;
DisplayPort::Display::Display display;
DisplayPort::Display::Mixer mixer;

void BringUpDisplayPort();

uintptr_lo_t videoBlock;
uintptr_lo_t FrameBuffer;

EXTERN_C int main(void)
{
	debug_force_raw_print(true);

	HW_REG_SET(CSU, AES_RESET, CSU_AES_RESET_RESET);
	HW_REG_SET(CSU, SHA_RESET, CSU_SHA_RESET_RESET);

	write_counter_timer_frequency_register(CPU_CORTEXA53_0_TIMESTAMP_CLK_FREQ);

	Cache_DCacheEnable();
	Cache_ICacheEnable();

	ClearPendingInterrupts();

	if(!(HW_REG_GET(PMU_GLOBAL, GLOBAL_GEN_STORAGE0) & OS_GLOBAL0_BOOT_COMPLETE)) {
		EnablePSToPL();

		RegisterBringUp();

		PrintBanner();

		// this is a silicon bug fix
		HW_REG_SET(AMS_PS_SYSMON, ANALOG_BUS, 0X00003210U);
	}

	MarkDdrAsMemory();

//	TcmInit();

	if(!(HW_REG_GET(PMU_GLOBAL, GLOBAL_GEN_STORAGE0) & OS_GLOBAL0_BOOT_COMPLETE)) {
		debug_printf("PMU size = %lu\n", (size_t) _binary_pmu_monitor_bin_end - (size_t) _binary_pmu_monitor_bin_start);
		debug_printf("PMU load started\n");
		PmuSleep();

		PmuSafeMemcpy((void *) PMU_RAM_ADDR,
									(void *) _binary_pmu_monitor_bin_start,
									(size_t) _binary_pmu_monitor_bin_end - (size_t) _binary_pmu_monitor_bin_start);
		PmuWakeup();

		debug_printf("Wait for PMU\n");
		// stall until pmu says it loaded and ready to go
		while (!(HW_REG_GET(PMU_GLOBAL, GLOBAL_GEN_STORAGE0) & OS_GLOBAL0_PMU_READY)) {
		}
		debug_printf("PMU Ready\n");
	}

	BootData bootData = {
			.frameBufferWidth = 1280,
			.frameBufferHeight = 720,
			.frameBufferHertz = 60,
			.videoBlockSizeInMB = 4,
			.bootCodeSize = OCM_0_SIZE_IN_BYTES,
			.videoBlock = 0,
			.bootCodeStart = OCM_0_BASE_ADDR,
	};

	if(!(HW_REG_GET(PMU_GLOBAL, GLOBAL_GEN_STORAGE0) & OS_GLOBAL0_BOOT_COMPLETE)) {
		debug_force_raw_print(false);
		BringUpDisplayPort();
		bootData.videoBlock = videoBlock;
	} else {
		OsService_FetchBootData(&bootData);
		videoBlock = bootData.videoBlock;
	}
	debug_force_raw_print(false);

	Cache_DCacheCleanAndInvalidate();
	if(!(HW_REG_GET(PMU_GLOBAL, GLOBAL_GEN_STORAGE0) & OS_GLOBAL0_BOOT_COMPLETE)) {
		debug_printf("Hard Boot Complete VideoBlock @ %#010x\n", videoBlock);
		OsService_BootComplete(&bootData);
		HW_REG_SET(PMU_GLOBAL, GLOBAL_GEN_STORAGE0,
							 HW_REG_GET(PMU_GLOBAL, GLOBAL_GEN_STORAGE0) | OS_GLOBAL0_BOOT_COMPLETE);
	} else {
		debug_printf("Soft Boot Finished VideoBlock @ %#010x\n", videoBlock);
	}

	while(1) {
	}
}

void PrintBanner(void )
{
	debug_printf(ANSI_CLR_SCREEN ANSI_YELLOW_PEN "IKUY Boot Loader\n" ANSI_RESET_ATTRIBUTES);
	debug_printf("Silicon Version %d\n", HW_REG_GET_FIELD(CSU, VERSION, PS_VERSION)+1);
	debug_printf( "A53 L1 Cache Size %dKiB, LineSize %d, Ways %d, Sets %d\n",
										(Cache_GetDCacheLineSizeInBytes(1) * Cache_GetDCacheNumWays(1) * Cache_GetDCacheNumSets(1)) / 1024,
										Cache_GetDCacheLineSizeInBytes(1),
										Cache_GetDCacheNumWays(1),
										Cache_GetDCacheNumSets(1) );
	debug_printf( "A53 L2 Cache Size %dKiB, LineSize %d, Ways %d, Sets %d\n",
										(Cache_GetDCacheLineSizeInBytes(2) * Cache_GetDCacheNumWays(2) * Cache_GetDCacheNumSets(2)) / 1024,
										Cache_GetDCacheLineSizeInBytes(2),
										Cache_GetDCacheNumWays(2),
										Cache_GetDCacheNumSets(2) );
	debug_printf("A53 NEON = %d, FMA = %d FP = 0x%x\n", __ARM_NEON, __ARM_FEATURE_FMA, __ARM_FP);
	uint32_t mmfr0 = read_ID_AA64MMFR0_EL1_register();
	debug_printf("MMU 4K Page = %d, MMU 16K Page = %d, MMU 64K Page = %d\n",
									 (mmfr0 & (1 << 28)) == 0,
									 (mmfr0 & (1 << 20)) != 0,
									 (mmfr0 & (1 << 24)) == 0);

}

void BringUpDisplayPort()
{
	debug_printf(ANSI_CLR_SCREEN ANSI_YELLOW_PEN "BringUpDisplayPort\n" ANSI_RESET_ATTRIBUTES);
	using namespace DisplayPort::Display;

	Init(&display);
	Init(&mixer);

	CopyStandardVideoMode(DisplayPort::Display::StandardVideoMode::VM_1280_720_60, &display.videoTiming);
	if(videoBlock == 0) videoBlock = OsService_DdrLoBlockAlloc(4);
	auto dmaDesc = (DMADescriptor*) (uintptr_t)videoBlock;
	FrameBuffer = (uintptr_lo_t)(uintptr_t)(videoBlock + 4096);

	Init(dmaDesc);
	dmaDesc->enableDescriptorUpdate = true;
	dmaDesc->transferSize = 1280 * 720 * 4;
	dmaDesc->width = (1280 * 4);
	dmaDesc->stride = (1280 * 4) >> 4;
	dmaDesc->nextDescriptorAddress = (uint32_t)(uintptr_t)dmaDesc;
	dmaDesc->nextDescriptorAddressExt = (uint32_t)(((uintptr_t)dmaDesc) >> 32ULL);
	dmaDesc->sourceAddress = (uint32_t)FrameBuffer;
	dmaDesc->sourceAddressExt = (uint32_t)(((uintptr_t)FrameBuffer) >> 32ULL);

	mixer.function = DisplayPort::Display::MixerFunction::GFX;
	mixer.globalAlpha = 0x80;

	mixer.videoPlane.source = DisplayPort::Display::DisplayVideoPlane::Source::TEST_GENERATOR;
	mixer.videoPlane.format = DisplayPort::Display::DisplayVideoPlane::Format::RGBX8;
	mixer.videoPlane.simpleDescPlane0Address = (uintptr_t)dmaDesc;

	mixer.gfxPlane.source = DisplayPort::Display::DisplayGfxPlane::Source::BUFFER;
	mixer.gfxPlane.format = DisplayPort::Display::DisplayGfxPlane::Format::RGBA8;
	mixer.gfxPlane.simpleDescBufferAddress = (uintptr_t)dmaDesc;
	Cache_DCacheCleanAndInvalidateRange(videoBlock, 4 * 1024 * 1024);

	if(videoBlock == 0) {
		if (!IsDisplayConnected(&link))
			return;
	}

	Init(&link);
	SetDisplay(&link, &display, &mixer);

/*
 * #define DP_AV_BUF_PALETTE_MEMORY_OFFSET 0x0000b400U
	for(int i = 0; i < 256;i++) {
		hw_RegWrite(DP_BASE_ADDR, DP_REGISTER(AV_BUF_PALETTE_MEMORY) + (i*4*3)+0, i<<4); // blue
		hw_RegWrite(DP_BASE_ADDR, DP_REGISTER(AV_BUF_PALETTE_MEMORY) + (i*4*3)+4, i<<4); // green
		hw_RegWrite(DP_BASE_ADDR, DP_REGISTER(AV_BUF_PALETTE_MEMORY) + (i*4*3)+8, 0<<4); // red
		debug_printf("CLUT[%03x] = %#010x\n", i, hw_RegRead(DP_BASE_ADDR, DP_REGISTER(AV_BUF_PALETTE_MEMORY) + (i*4*3)+0));
		debug_printf("CLUT[%03x] = %#010x\n", i, hw_RegRead(DP_BASE_ADDR, DP_REGISTER(AV_BUF_PALETTE_MEMORY) + (i*4*3)+4));
		debug_printf("CLUT[%03x] = %#010x\n", i, hw_RegRead(DP_BASE_ADDR, DP_REGISTER(AV_BUF_PALETTE_MEMORY) + (i*4*3)+8));
	}

	// [0xb700 - 0xb713]
	for(int i = 0; i < 0x13;i++) {
//		hw_RegWrite(DP_BASE_ADDR, DP_REGISTER(AV_BUF_PALETTE_MEMORY) + (i*4)+(256*12), i);
		debug_printf("CLUT[%03x] = %#010x\n", i, hw_RegRead(DP_BASE_ADDR, DP_REGISTER(AV_BUF_PALETTE_MEMORY) + (i*4)+(256*12)));
	}
*/
}

void PmuSleep() {
	// Enable PMU_0 IPI
	HW_REG_SET_BIT(IPI, PMU_0_IER, CH3);

	// Trigger PMU0 IPI in PMU IPI TRIG Reg
	HW_REG_SET_BIT(IPI, PMU_0_TRIG, CH3);

	// Wait until PMU Microblaze goes to sleep state,
	// before starting firmware download to PMU RAM
	while(!HW_REG_GET_BIT(PMU_GLOBAL, GLOBAL_CNTRL, MB_SLEEP)) {
		// stall
	}

	HW_REG_CLR_BIT(PMU_GLOBAL, GLOBAL_CNTRL, FW_IS_PRESENT);
}

void PmuWakeup() {

	HW_REG_SET_BIT(PMU_GLOBAL, GLOBAL_CNTRL, DONT_SLEEP);

	// pmu firmware set the FW_IS_PRESENT flag once it running
	while(HW_REG_GET_BIT(PMU_GLOBAL, GLOBAL_CNTRL, FW_IS_PRESENT) == 0) {
		// stall
	}
}

// pmu ram only accepts 32 bit access, this ensure thats true
void PmuSafeMemcpy(void * destination, const void* source, size_t num_in_bytes ) {
	uint32_t copy_count = 0;
	uint32_t* dst = (uint32_t*)destination;
	const uint32_t* src = (uint32_t*)source;

	// copy in 32 bit words chunks into PMU_RAM
	for(copy_count = 0; copy_count < (num_in_bytes+3)/4; copy_count++) {
		*(dst + copy_count) = *(src + copy_count);
	}
}

void EnablePSToPL(void)
{
	HW_REG_SET_BIT(CSU, PCAP_PROG, PCFG_PROG_B);
	HW_REG_SET_BIT(PMU_GLOBAL, PS_CNTRL, PROG_GATE);
}


void ClearPendingInterrupts(void)
{
	// Clear pending peripheral interrupts
	HW_REG_SET(ACPU_GIC, GICD_ICENABLER0, 0xFFFFFFFFU);
	HW_REG_SET(ACPU_GIC, GICD_ICPENDR0, 0xFFFFFFFFU);
	HW_REG_SET(ACPU_GIC, GICD_ICACTIVER0, 0xFFFFFFFFU);

	HW_REG_SET(ACPU_GIC, GICD_ICENABLER1, 0xFFFFFFFFU);
	HW_REG_SET(ACPU_GIC, GICD_ICPENDR1, 0xFFFFFFFFU);
	HW_REG_SET(ACPU_GIC, GICD_ICACTIVER1, 0xFFFFFFFFU);

	HW_REG_SET(ACPU_GIC, GICD_ICENABLER2, 0xFFFFFFFFU);
	HW_REG_SET(ACPU_GIC, GICD_ICPENDR2, 0xFFFFFFFFU);
	HW_REG_SET(ACPU_GIC, GICD_ICACTIVER2, 0xFFFFFFFFU);

	HW_REG_SET(ACPU_GIC, GICD_ICENABLER3, 0xFFFFFFFFU);
	HW_REG_SET(ACPU_GIC, GICD_ICPENDR3, 0xFFFFFFFFU);
	HW_REG_SET(ACPU_GIC, GICD_ICACTIVER3, 0xFFFFFFFFU);

	HW_REG_SET(ACPU_GIC, GICD_ICENABLER4, 0xFFFFFFFFU);
	HW_REG_SET(ACPU_GIC, GICD_ICPENDR4, 0xFFFFFFFFU);
	HW_REG_SET(ACPU_GIC, GICD_ICACTIVER4, 0xFFFFFFFFU);

	HW_REG_SET(ACPU_GIC, GICD_ICENABLER5, 0xFFFFFFFFU);
	HW_REG_SET(ACPU_GIC, GICD_ICPENDR5, 0xFFFFFFFFU);
	HW_REG_SET(ACPU_GIC, GICD_ICACTIVER5, 0xFFFFFFFFU);

	// Clear active software generated interrupts, if any
	HW_REG_SET(ACPU_GIC, GICC_EOIR, HW_REG_GET(ACPU_GIC, GICC_IAR));

	// Clear pending software generated interrupts
	HW_REG_SET(ACPU_GIC, GICD_CPENDSGIR0, 0xFFFFFFFFU);
	HW_REG_SET(ACPU_GIC, GICD_CPENDSGIR1, 0xFFFFFFFFU);
	HW_REG_SET(ACPU_GIC, GICD_CPENDSGIR2, 0xFFFFFFFFU);
	HW_REG_SET(ACPU_GIC, GICD_CPENDSGIR3, 0xFFFFFFFFU);

}

void PowerUpIsland(uint32_t PwrIslandMask)
{

	/* There is a single island for both R5_0 and R5_1 */
	if ((PwrIslandMask & PMU_GLOBAL_PWR_STATE_R5_1_MASK) == PMU_GLOBAL_PWR_STATE_R5_1_MASK) {
		PwrIslandMask &= ~(PMU_GLOBAL_PWR_STATE_R5_1_MASK);
		PwrIslandMask |= PMU_GLOBAL_PWR_STATE_R5_0_MASK;
	}

	/* Power up request enable */
	HW_REG_SET(PMU_GLOBAL, REQ_PWRUP_INT_EN, PwrIslandMask);

	/* Trigger power up request */
	HW_REG_SET(PMU_GLOBAL, REQ_PWRUP_TRIG, PwrIslandMask);

	/* Poll for Power up complete */
	do {
		;// empty
	} while ((HW_REG_GET(PMU_GLOBAL, REQ_PWRUP_STATUS) & PwrIslandMask) != 0x0U);
}

#define BLOCK_SIZE_A53 0x200000U
#define NUM_BLOCKS_A53 0x400U
#define BLOCK_SIZE_A53_HIGH 0x40000000U
#define NUM_BLOCKS_A53_HIGH (MAINDDR4_1_SIZE_IN_BYTES / BLOCK_SIZE_A53_HIGH)
#define ATTRIB_MEMORY_A53 0x705U
#define BLOCK_SIZE_2MB 0x200000U
#define BLOCK_SIZE_1GB 0x40000000U
#define ADDRESS_LIMIT_4GB 0x100000000UL


void SetTlbAttributes(uintptr_t Addr, uintptr_t attrib)
{
	uintptr_t *ptr;
	uintptr_t section;
	uint64_t block_size;
	/* if region is less than 4GB MMUTable level 2 need to be modified */
	if(Addr < ADDRESS_LIMIT_4GB){
		/* block size is 2MB for addressed < 4GB*/
		block_size = BLOCK_SIZE_2MB;
		section = Addr / block_size;
		ptr = (uintptr_t*)MMUTableL2 + section;
	}
	/* if region is greater than 4GB MMUTable level 1 need to be modified */
	else{
		/* block size is 1GB for addressed > 4GB */
		block_size = BLOCK_SIZE_1GB;
		section = Addr / block_size;
		ptr = (uintptr_t*)MMUTableL1 + section;
	}
	*ptr = (Addr & (~(block_size-1))) | attrib;

	invalid_all_TLB();
	dsb(); // ensure completion of the BP and TLB invalidation
	isb(); // synchronize context on this processor
}

void MarkDdrAsMemory()
{
	uint64_t BlockNum;
	for (BlockNum = 0; BlockNum < NUM_BLOCKS_A53; BlockNum++) {
		SetTlbAttributes(MAINDDR4_0_BASE_ADDR + BlockNum * BLOCK_SIZE_A53, ATTRIB_MEMORY_A53);
	}
	for (BlockNum = 0; BlockNum < NUM_BLOCKS_A53_HIGH; BlockNum++) {
		SetTlbAttributes(MAINDDR4_1_BASE_ADDR + BlockNum * BLOCK_SIZE_A53_HIGH, ATTRIB_MEMORY_A53);
	}

	Cache_DCacheCleanAndInvalidate();
}

#define XFSBL_ECC_INIT_VAL_WORD 0xDEADBEEFU
#define ZDMA_TRANSFER_MAX_LEN (0x3FFFFFFFU - 7U)

void EccInit(uint64_t DestAddr, uint64_t LengthBytes)
{
	uint32_t RegVal;
	uint32_t Length;
	uint64_t StartAddr = DestAddr;
	uint64_t NumBytes = LengthBytes;

	Cache_DCacheDisable();

	while (NumBytes > 0U) {
		if (NumBytes > ZDMA_TRANSFER_MAX_LEN) {
			Length = ZDMA_TRANSFER_MAX_LEN;
		} else {
			Length = (uint32_t) NumBytes;
		}

		// Wait until the DMA is in idle state
		do {
			RegVal = HW_REG_GET_FIELD(LPD_DMA_CH0, ZDMA_CH_STATUS, STATE);
		} while ((RegVal != 0U) && (RegVal != 3U)); // OK or ERR need enum support in overlord

		// Enable Simple (Write Only) Mode

		HW_REG_MERGE(LPD_DMA_CH0,
				ZDMA_CH_CTRL0,
				HW_REG_FIELD_MASK(LPD_DMA_CH0, ZDMA_CH_CTRL0, POINT_TYPE) | HW_REG_FIELD_MASK(LPD_DMA_CH0, ZDMA_CH_CTRL0, MODE),
				ZDMA_ZDMA_CH_CTRL0_MODE_WRITE_ONLY);


		// Fill in the data to be written
		HW_REG_SET(LPD_DMA_CH0, ZDMA_CH_WR_ONLY_WORD0, XFSBL_ECC_INIT_VAL_WORD);
		HW_REG_SET(LPD_DMA_CH0, ZDMA_CH_WR_ONLY_WORD1, XFSBL_ECC_INIT_VAL_WORD);
		HW_REG_SET(LPD_DMA_CH0, ZDMA_CH_WR_ONLY_WORD2, XFSBL_ECC_INIT_VAL_WORD);
		HW_REG_SET(LPD_DMA_CH0, ZDMA_CH_WR_ONLY_WORD3, XFSBL_ECC_INIT_VAL_WORD);

		// Write Destination Address (64 bit address though we know this address is <$GiB)
		HW_REG_SET(LPD_DMA_CH0, ZDMA_CH_DST_DSCR_WORD0, (uint32_t)(StartAddr & 0xFFFFFFFF));
		HW_REG_SET(LPD_DMA_CH0, ZDMA_CH_DST_DSCR_WORD1, (uint32_t)((StartAddr >> 32U) & 0X0001FFFFU));

		// Size to be Transferred. Recommended to set both src and dest sizes
		HW_REG_SET(LPD_DMA_CH0, ZDMA_CH_SRC_DSCR_WORD2, Length);
		HW_REG_SET(LPD_DMA_CH0, ZDMA_CH_DST_DSCR_WORD2, Length);

		// DMA Enable
		HW_REG_SET_BIT(LPD_DMA_CH0, ZDMA_CH_CTRL2, EN);

		// Check the status of the transfer by polling on DMA Done
		do {
			;// empty
		} while (!HW_REG_GET_BIT(LPD_DMA_CH0, ZDMA_CH_ISR, DMA_DONE));

		// Clear DMA status
		HW_REG_CLR_BIT(LPD_DMA_CH0, ZDMA_CH_ISR, DMA_DONE);

		// Read the channel status for errors
		if (HW_REG_GET_FIELD(LPD_DMA_CH0, ZDMA_CH_STATUS, STATE) == 0x3) {
			Cache_DCacheEnable();
			raw_debug_print("LPD_DMA_CH0 ZDMA Error!");
			return;
		}

		NumBytes -= Length;
		StartAddr += Length;
	}

	Cache_DCacheEnable();

	// Restore reset values for the DMA registers used
	HW_REG_SET(LPD_DMA_CH0, ZDMA_CH_CTRL0, 0x00000080U);
	HW_REG_SET(LPD_DMA_CH0, ZDMA_CH_WR_ONLY_WORD0, 0x00000000U);
	HW_REG_SET(LPD_DMA_CH0, ZDMA_CH_WR_ONLY_WORD1, 0x00000000U);
	HW_REG_SET(LPD_DMA_CH0, ZDMA_CH_WR_ONLY_WORD2, 0x00000000U);
	HW_REG_SET(LPD_DMA_CH0, ZDMA_CH_WR_ONLY_WORD3, 0x00000000U);
	HW_REG_SET(LPD_DMA_CH0, ZDMA_CH_DST_DSCR_WORD0, 0x00000000U);
	HW_REG_SET(LPD_DMA_CH0, ZDMA_CH_DST_DSCR_WORD1, 0x00000000U);
	HW_REG_SET(LPD_DMA_CH0, ZDMA_CH_SRC_DSCR_WORD2, 0x00000000U);
	HW_REG_SET(LPD_DMA_CH0, ZDMA_CH_DST_DSCR_WORD2, 0x00000000U);
	HW_REG_SET(LPD_DMA_CH0, ZDMA_CH_CTRL0_TOTAL_BYTE,0x00000000U);

	raw_debug_printf( "Address 0x%0x%08x, Length 0x%0x%08x, ECC initialized \r\n",
		(uint32_t)(DestAddr >> 32U), (uint32_t)DestAddr,
		(uint32_t)(LengthBytes >> 32U), (uint32_t)LengthBytes);
}

#define XFSBL_R50_HIGH_ATCM_START_ADDRESS	(0xFFE00000U)
#define XFSBL_R5_TCM_BANK_LENGTH			(0x10000U)

void TcmInit()
{
	uint32_t LengthBytes;
	uint32_t ATcmAddr;
	uint32_t PwrStateMask;

	raw_debug_printf("Initializing TCM ECC\r\n");

	// power it up
	PwrStateMask = (PMU_GLOBAL_PWR_STATE_R5_0_MASK |
			PMU_GLOBAL_PWR_STATE_TCM0A_MASK |
			PMU_GLOBAL_PWR_STATE_TCM0B_MASK |
			PMU_GLOBAL_PWR_STATE_TCM1A_MASK |
			PMU_GLOBAL_PWR_STATE_TCM1B_MASK);
	PowerUpIsland(PwrStateMask);

	// switch into rpu combined mode
	HW_REG_CLR_BIT(RPU, RPU_GLBL_CNTL, SLSPLIT);
	HW_REG_SET_BIT(RPU, RPU_GLBL_CNTL, SLCLAMP);
	HW_REG_SET_BIT(RPU, RPU_GLBL_CNTL, TCM_COMB);

	// Place R5-0 and R5-1 in HALT state
	HW_REG_SET_BIT(RPU, RPU0_CFG, NCPUHALT);
	HW_REG_SET_BIT(RPU, RPU1_CFG, NCPUHALT);

	// Enable the clock
	HW_REG_SET_BIT(CRL_APB, CPU_R5_CTRL, CLKACT);

	// Provide some delay, so that clock propagates properly.
	Utils_BusyMicroSleep(0x50U);

	// Release reset to R5-0, R5-1 and amba
	HW_REG_CLR_BIT(CRL_APB, RST_LPD_TOP, RPU_R50_RESET);
	HW_REG_CLR_BIT(CRL_APB, RST_LPD_TOP, RPU_R51_RESET);
	HW_REG_CLR_BIT(CRL_APB, RST_LPD_TOP, RPU_AMBA_RESET);

	// init all tcm in one go
	ATcmAddr = XFSBL_R50_HIGH_ATCM_START_ADDRESS;
	LengthBytes = XFSBL_R5_TCM_BANK_LENGTH * 4U;
	EccInit(ATcmAddr, LengthBytes);

	// power it down
	HW_REG_SET_BIT(CRL_APB, RST_LPD_TOP, RPU_AMBA_RESET);
	HW_REG_SET_BIT(CRL_APB, RST_LPD_TOP, RPU_R51_RESET);
	HW_REG_SET_BIT(CRL_APB, RST_LPD_TOP, RPU_R50_RESET);

}

EXTERN_C void SynchronousInterrupt(void) {
	uint64_t const esr = read_ESR_EL3_register();
	uint64_t const elr = read_ELR_EL3_register();
	uint32_t const ec = HW_REG_DECODE_FIELD(A53_SYSTEM, ESR_EL3, EC, esr);
	uint64_t const far = read_FAR_EL3_register();
	switch(ec) {
		case A53_SYSTEM_ESR_EL3_EC_UNKNOWN:
		case A53_SYSTEM_ESR_EL3_EC_WFX:
		case A53_SYSTEM_ESR_EL3_EC_COP15_AARCH32:
		case A53_SYSTEM_ESR_EL3_EC_TRAPPED_MCRR_MRRC_AARCH32:
		case A53_SYSTEM_ESR_EL3_EC_COP14_AARCH32:
		case A53_SYSTEM_ESR_EL3_EC_LDC_SRC_AARCH32:
		case A53_SYSTEM_ESR_EL3_EC_SVE_SIMD_FP:
		case A53_SYSTEM_ESR_EL3_EC_PAUTH:
		case A53_SYSTEM_ESR_EL3_EC_MRRC_COP14_AARCH32:
		case A53_SYSTEM_ESR_EL3_EC_BTI:
		case A53_SYSTEM_ESR_EL3_EC_ILLEGAL_EXECUTION:
		case A53_SYSTEM_ESR_EL3_EC_SVC:
		case A53_SYSTEM_ESR_EL3_EC_HVC:
		case A53_SYSTEM_ESR_EL3_EC_SMC:
		case A53_SYSTEM_ESR_EL3_EC_MSR_MRS:
		case A53_SYSTEM_ESR_EL3_EC_SVE:
		case A53_SYSTEM_ESR_EL3_EC_FPAC:
		case A53_SYSTEM_ESR_EL3_EC_IMPLEMENTATION_DEFINED:
		case A53_SYSTEM_ESR_EL3_EC_PC_ALIGNMENT_FAULT:
		case A53_SYSTEM_ESR_EL3_EC_SP_ALIGNMENT_FAULT:
		case A53_SYSTEM_ESR_EL3_EC_FLOATING_POINT:
		case A53_SYSTEM_ESR_EL3_EC_SERROR:
		case A53_SYSTEM_ESR_EL3_EC_BRK:
			raw_debug_printf(ANSI_RED_PEN "SynchronousInterrupt %#010lx ec %d %#010lx\n" ANSI_RESET_ATTRIBUTES,esr, ec, far);
			break;
		case A53_SYSTEM_ESR_EL3_EC_INSTRUCTION_ABORT_FROM_LOWER_EL:
		case A53_SYSTEM_ESR_EL3_EC_INSTRUCTION_ABORT:
			raw_debug_printf(ANSI_RED_PEN "Instruction Abort %#010lx ec %d %#010lx\n" ANSI_RESET_ATTRIBUTES,esr, ec, far);
			break;
		case A53_SYSTEM_ESR_EL3_EC_DATA_ABORT_FROM_LOWER_EL:
		case A53_SYSTEM_ESR_EL3_EC_DATA_ABORT: {
			uint32_t iss = HW_REG_DECODE_FIELD(A53_SYSTEM, ESR_EL3, ISS, esr);
			uint32_t dfsc = HW_REG_DECODE_FIELD(A53_SYSTEM, ISS_DATA_ABORT, DFSC, iss);
			raw_debug_printf(ANSI_RED_PEN "Data Abort @ %#018lx from instruction @ %#018lx: ", far, elr);
			int level = 0;
			switch(dfsc) {
				case A53_SYSTEM_ISS_DATA_ABORT_DFSC_ADDRESS_SIZE_FAULT_L3: level++;
					[[fallthrough]];
				case A53_SYSTEM_ISS_DATA_ABORT_DFSC_ADDRESS_SIZE_FAULT_L2: level++;
					[[fallthrough]];
				case A53_SYSTEM_ISS_DATA_ABORT_DFSC_ADDRESS_SIZE_FAULT_L1: level++;
					[[fallthrough]];
				case A53_SYSTEM_ISS_DATA_ABORT_DFSC_ADDRESS_SIZE_FAULT_L0:
					raw_debug_printf(ANSI_RED_PEN "Address Size fault Level %d\n" ANSI_RESET_ATTRIBUTES, level);
					break;
				case A53_SYSTEM_ISS_DATA_ABORT_DFSC_TRANSLATION_FAULT_L3: level++;
					[[fallthrough]];
				case A53_SYSTEM_ISS_DATA_ABORT_DFSC_TRANSLATION_FAULT_L2: level++;
					[[fallthrough]];
				case A53_SYSTEM_ISS_DATA_ABORT_DFSC_TRANSLATION_FAULT_L1: level++;
					[[fallthrough]];
				case A53_SYSTEM_ISS_DATA_ABORT_DFSC_TRANSLATION_FAULT_L0:
					raw_debug_printf(ANSI_RED_PEN "Translation fault Level %d\n" ANSI_RESET_ATTRIBUTES, level);
					break;
				case A53_SYSTEM_ISS_DATA_ABORT_DFSC_ACCESS_FLAG_FAULT_L3: level++;
					[[fallthrough]];
				case A53_SYSTEM_ISS_DATA_ABORT_DFSC_ACCESS_FLAG_FAULT_L2: level++;
					[[fallthrough]];
				case A53_SYSTEM_ISS_DATA_ABORT_DFSC_ACCESS_FLAG_FAULT_L1:
					raw_debug_printf(ANSI_RED_PEN "Access Flag fault Level %d\n" ANSI_RESET_ATTRIBUTES, level);
					break;
				case A53_SYSTEM_ISS_DATA_ABORT_DFSC_PERMISSION_FLAG_FAULT_L3: level++;
					[[fallthrough]];
				case A53_SYSTEM_ISS_DATA_ABORT_DFSC_PERMISSION_FLAG_FAULT_L2: level++;
					[[fallthrough]];
				case A53_SYSTEM_ISS_DATA_ABORT_DFSC_PERMISSION_FLAG_FAULT_L1:
					raw_debug_printf(ANSI_RED_PEN "Permission Flag fault Level %d\n" ANSI_RESET_ATTRIBUTES, level);
					break;
				case A53_SYSTEM_ISS_DATA_ABORT_DFSC_SYNCHRONOUS_EXTERNAL_ABORT_NOT_TTW:
					raw_debug_print(ANSI_RED_PEN "Synchronous External Abort Not TTW\n" ANSI_RESET_ATTRIBUTES);
					break;
				case A53_SYSTEM_ISS_DATA_ABORT_DFSC_SYNCHRONOUS_EXTERNAL_ABORT_L3: level++;
					[[fallthrough]];
				case A53_SYSTEM_ISS_DATA_ABORT_DFSC_SYNCHRONOUS_EXTERNAL_ABORT_L2: level++;
					[[fallthrough]];
				case A53_SYSTEM_ISS_DATA_ABORT_DFSC_SYNCHRONOUS_EXTERNAL_ABORT_L1: level++;
					[[fallthrough]];
				case A53_SYSTEM_ISS_DATA_ABORT_DFSC_SYNCHRONOUS_EXTERNAL_ABORT_L0:
					raw_debug_printf(ANSI_RED_PEN "Synchronous External Abort Level %d\n" ANSI_RESET_ATTRIBUTES, level);
					break;
				case A53_SYSTEM_ISS_DATA_ABORT_DFSC_ECC_ERROR_ABORT_NOT_TTW:
					raw_debug_print(ANSI_RED_PEN "ECC Error Abort Not TTW\n" ANSI_RESET_ATTRIBUTES);
					break;
				case A53_SYSTEM_ISS_DATA_ABORT_DFSC_ECC_ERROR_ABORT_L3: level++;
					[[fallthrough]];
				case A53_SYSTEM_ISS_DATA_ABORT_DFSC_ECC_ERROR_ABORT_L2: level++;
					[[fallthrough]];
				case A53_SYSTEM_ISS_DATA_ABORT_DFSC_ECC_ERROR_ABORT_L1: level++;
					[[fallthrough]];
				case A53_SYSTEM_ISS_DATA_ABORT_DFSC_ECC_ERROR_ABORT_L0:
					raw_debug_printf(ANSI_RED_PEN "ECC Error Abort Level %d\n" ANSI_RESET_ATTRIBUTES, level);
					break;
				case A53_SYSTEM_ISS_DATA_ABORT_DFSC_ALIGNMENT_FAULT:
					raw_debug_print(ANSI_RED_PEN "Alignment Fault\n" ANSI_RESET_ATTRIBUTES);
					break;
				case A53_SYSTEM_ISS_DATA_ABORT_DFSC_TLB_CONFLICT:
					raw_debug_print(ANSI_RED_PEN "TLB Conflict\n" ANSI_RESET_ATTRIBUTES);
					break;
				case A53_SYSTEM_ISS_DATA_ABORT_DFSC_UNSUPPORTED_ATOMIC_HARDWARE_FAULT:
					raw_debug_print(ANSI_RED_PEN "Unsupported Atomic Hardware Fault\n" ANSI_RESET_ATTRIBUTES);
					break;
				case A53_SYSTEM_ISS_DATA_ABORT_DFSC_LOCKDOWN:
					raw_debug_print(ANSI_RED_PEN "Lockdown Fault\n" ANSI_RESET_ATTRIBUTES);
					break;
				case A53_SYSTEM_ISS_DATA_ABORT_DFSC_UNSUPPORTED_EXCLUSIVE_OR_ATOMIC:
					raw_debug_print(ANSI_RED_PEN "Unsupported Exclusive or Atomic Fault\n" ANSI_RESET_ATTRIBUTES);
					break;
			}
			break;
		}
	}
	while(true) {
	}
}

EXTERN_C void IRQInterrupt(void) {
	raw_debug_printf("IRQInterrupt\n");
	asm volatile("wfe");
}

EXTERN_C void FIQInterrupt(void) {
	raw_debug_printf("FIQInterrupt\n");
	asm volatile("wfe");
}
EXTERN_C void SErrorInterrupt(void) {
	raw_debug_printf("SErrorInterruptHandler\n");
	asm volatile("wfe");
}

