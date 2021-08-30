// SPDX-License-Identifier: MIT

#include "core/core.h"
#include "dbg/raw_print.h"
#include "dbg/ansi_escapes.h"
#include "hw/reg_access.h"
#include "hw/cache.h"
#include "hw/aarch64/intrinsics_gcc.h"
#include "hw/memory_map.h"
#include "hw_regs/uart.h"
#include "hw_regs/csu.h"
#include "hw_regs/ipi.h"
#include "hw_regs/pmu_global.h"
#include "hw_regs/gic400.h"
#include "hw_regs/zdma.h"
#include "hw_regs/rpu.h"
#include "hw_regs/crl_apb.h"

void PrintBanner(void);
void EnablePSToPL(void);
void ClearPendingInterrupts(void);
void MarkDdrAsMemory(void);

void EccInit(uint64_t DestAddr, uint64_t LengthBytes);
void TcmInit(void);

void USleep(uint64_t useconds);
void PmuSleep();
void PmuDownload();
void PmuWakeup();

extern void mioRunInitProgram();
extern void pllRunInitProgram();
extern void clockRunInitProgram();
extern void ddrRunInitProgram();
extern void peripheralsRunInitProgram();
extern void serdesRunInitProgram();
extern void miscRunInitProgram();
extern void ddrQosRunInitProgram();
extern void Handoff();


extern uint8_t HeapBase[];
extern uint8_t HeapLimit[];

extern uint8_t MMUTableL1[];
extern uint8_t MMUTableL2[];

#define CPU_CORTEXA53_0_CPU_CLK_FREQ_HZ 1199988037
#define CPU_CORTEXA53_0_TIMESTAMP_CLK_FREQ 33333000

#define PSSYSMON_ANALOG_BUS_OFFSET		0x114U
#define ZDMA_ZDMA_CH_CTRL0_TOTAL_BYTE_OFFSET 0X0188U

int main(void)
{
	HW_REG_SET(CSU, AES_RESET, CSU_AES_RESET_RESET);
	HW_REG_SET(CSU, SHA_RESET, CSU_SHA_RESET_RESET);

	write_counter_timer_frequency_register(CPU_CORTEXA53_0_TIMESTAMP_CLK_FREQ);

	DCacheEnable();
	ICacheEnable();

	EnablePSToPL();
	ClearPendingInterrupts();

	// Register data is prefilled into the heap, so don't use
	// the heap until you have programmed the registers!
	mioRunInitProgram();
	pllRunInitProgram();
	clockRunInitProgram();
	peripheralsRunInitProgram();

	raw_debug_printf("\nUART ready to be used\n");
	PrintBanner();

	ddrRunInitProgram();
	serdesRunInitProgram();
	miscRunInitProgram();

	ddrQosRunInitProgram();

	// this is a silicon bug fix
	HW_REG_SET(AMS_PS_SYSMON, ANALOG_BUS, 0X00003210U);

	MarkDdrAsMemory();
	TcmInit();

	raw_debug_printf("Initialization Done\n");

	/*
	 * Write 1U to PMU GLOBAL general storage register 5 to indicate
	 * PMU Firmware that FSBL completed execution
	 */
//	HW_REG_MERGE(PMU_GLOBAL, GLOBAL_GEN_STORAGE5, 0x1, 0x1);

	raw_debug_printf("Boot program Finished\n");

	raw_debug_printf("PMU Download Start\n");

	// clear heap
	memset(HeapBase, 0, HeapLimit - HeapBase);
	PmuSleep();
	PmuDownload();
	raw_debug_printf("Switching to PMU\n");
	PmuWakeup();

	Handoff();
	return 0;
}
void PrintBanner(void )
{
	raw_debug_printf(DEBUG_CLR_SCREEN DEBUG_YELLOW_PEN "IKUY Boot Loader\n"DEBUG_RESET_COLOURS);
	raw_debug_printf("Silicon Version %d\n", HW_REG_GET_FIELD(CSU, VERSION, PS_VERSION)+1);
	raw_debug_printf( "A53 L1 Cache Size %dKiB, LineSize %d, Number of Ways %d, Number of Sets %d\n",
								(GetDCacheLineSizeInBytes(1) * GetDCacheNumWays(1) * GetDCacheNumSets(1)) / 1024,
								GetDCacheLineSizeInBytes(1),
								GetDCacheNumWays(1),
								GetDCacheNumSets(1) );
	raw_debug_printf( "A53 L2 Cache Size %dKiB, LineSize %d, Number of Ways %d, Number of Sets %d\n",
								(GetDCacheLineSizeInBytes(2) * GetDCacheNumWays(2) * GetDCacheNumSets(2)) / 1024,
								GetDCacheLineSizeInBytes(2),
								GetDCacheNumWays(2),
								GetDCacheNumSets(2) );

	raw_debug_print( "~`TEST`~\n");
}

typedef enum {
	PDS_WaitingForCommand = 0,
	PDS_Receive,
	PDS_Done
} PmuDownloadState;

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
void pmu_safe_memcpy(void * destination, const void* source, size_t num_in_bytes ) {
	uint32_t copy_count = 0;
	uint32_t* dst = (uint32_t*)destination;
	const uint32_t* src = (uint32_t*)source;

	// copy in 32 bit words chunks into PMU_RAM
	for(copy_count = 0; copy_count < num_in_bytes/4; copy_count++) {
		*(dst + copy_count) = *(src + copy_count);
	}
}

void PmuDownload() {
	PmuDownloadState state = PDS_WaitingForCommand;

// TODO get from memory map
#define PMU_RAM_ADDR 0xFFDC0000

	uint8_t* buffer = HeapBase;

	uint8_t* argv[8] = { 0 };
	uint8_t* last_arg_end = buffer;
	uint32_t argc = 0;

	uint32_t buffer_size = 4096;
	uint8_t* download_addr = (uint8_t*) PMU_RAM_ADDR;
	uint32_t download_size = 0;
	uint32_t current_count = 0;

	raw_debug_printf("Heap Address 0x%p\n", (void*)buffer);
	raw_debug_printf( "~`START_PMU_DOWNLOAD 0x%x`~\n", buffer_size);

	DCacheDisable();

	while(true) {
		if(!HW_REG_GET_BIT(UART0, CHANNEL_STS, REMPTY)) {
			switch (state) {
				case PDS_WaitingForCommand:; {
					*buffer = (uint8_t) HW_REG_GET(UART0, TX_RX_FIFO);
					if(*buffer == 0) {
						buffer = HeapBase;
						while(*buffer != 0) {
							if(*buffer == ' ') {
								*buffer = 0; // space to 0
								argv[argc++] = last_arg_end;
								last_arg_end = buffer+1;
							}
							buffer++;
						}

						argv[argc++] = last_arg_end;
						buffer_size = atoi((char*) argv[1]);
						download_size = atoi((char*) argv[2]);
						buffer = HeapBase;
						state = PDS_Receive;
					} else {
						buffer++;
					}
				}
				break;
				case PDS_Receive: {
					*buffer = (uint8_t) HW_REG_GET(UART0, TX_RX_FIFO);
					buffer++;
					current_count++;
					if(current_count == download_size) {
						pmu_safe_memcpy(download_addr, HeapBase, (uint32_t)(buffer - HeapBase));
						buffer = HeapBase;
						state = PDS_Done;
						raw_debug_print("END");
					} else if(buffer - HeapBase == buffer_size) {
						pmu_safe_memcpy(download_addr, HeapBase, buffer_size);
						download_addr += buffer_size;
						buffer = HeapBase;
						raw_debug_print("NEXT");
					}
					break;
				}
				case PDS_Done: {
					*buffer = (uint8_t) HW_REG_GET(UART0, TX_RX_FIFO);
					if(*buffer == 0) {
						if( memcmp(HeapBase, "PMU_DONE", 7) != 0) {
							raw_debug_printf("Host sent %s rather then PMU_DONE", buffer);
						}
						DCacheEnable();
						return;
					}
				}
			}
		}
	}

#undef PMU_RAM_ADDR
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
#define NUM_BLOCKS_A53_HIGH (DDR_DDR4_1_SIZE_IN_BYTES / BLOCK_SIZE_A53_HIGH)
#define ATTRIB_MEMORY_A53 0x705U
#define BLOCK_SIZE_2MB 0x200000U
#define BLOCK_SIZE_1GB 0x40000000U
#define ADDRESS_LIMIT_4GB 0x100000000UL


void SetTlbAttributes(intptr_t Addr, uintptr_t attrib)
{
	intptr_t *ptr;
	intptr_t section;
	uint64_t block_size;
	/* if region is less than 4GB MMUTable level 2 need to be modified */
	if(Addr < ADDRESS_LIMIT_4GB){
		/* block size is 2MB for addressed < 4GB*/
		block_size = BLOCK_SIZE_2MB;
		section = Addr / block_size;
		ptr = (intptr_t*)MMUTableL2 + section;
	}
	/* if region is greater than 4GB MMUTable level 1 need to be modified */
	else{
		/* block size is 1GB for addressed > 4GB */
		block_size = BLOCK_SIZE_1GB;
		section = Addr / block_size;
		ptr = (intptr_t*)MMUTableL1 + section;
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
		SetTlbAttributes(DDR_DDR4_0_BASE_ADDR + BlockNum * BLOCK_SIZE_A53, ATTRIB_MEMORY_A53);
	}
	for (BlockNum = 0; BlockNum < NUM_BLOCKS_A53_HIGH; BlockNum++) {
		SetTlbAttributes(DDR_DDR4_1_BASE_ADDR + BlockNum * BLOCK_SIZE_A53_HIGH, ATTRIB_MEMORY_A53);
	}

	DCacheCleanAndInvalidate();
}

#define TIMESTAMP_COUNTS_PER_SECOND     CPU_CORTEXA53_0_TIMESTAMP_CLK_FREQ
#define TIMESTAMP_COUNTS_PER_USECOND  ((TIMESTAMP_COUNTS_PER_SECOND + 500000)/1000000)

void USleep(uint64_t useconds)
{
	uint64_t tEnd, tCur;

	tCur = read_counter_timer_register();
	tEnd = tCur + (useconds * TIMESTAMP_COUNTS_PER_USECOND);
	do
	{
		tCur = read_counter_timer_register();
	} while (tCur < tEnd);
}

#define XFSBL_ECC_INIT_VAL_WORD 0xDEADBEEFU
#define ZDMA_TRANSFER_MAX_LEN (0x3FFFFFFFU - 7U)

void EccInit(uint64_t DestAddr, uint64_t LengthBytes)
{
	uint32_t RegVal;
	uint32_t Length;
	uint64_t StartAddr = DestAddr;
	uint64_t NumBytes = LengthBytes;

	DCacheDisable();

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
				0U);
		// TODO enum support these equal 0
//		(ADMA_CH0_ZDMA_CH_CTRL0_POINT_TYPE_NORMAL | ADMA_CH0_ZDMA_CH_CTRL0_MODE_WR_ONLY);


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
			DCacheEnable();
			raw_debug_print("LPD_DMA_CH0 ZDMA Error!");
			return;
		}

		NumBytes -= Length;
		StartAddr += Length;
	}

	DCacheEnable();

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
	USleep(0x50U);

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

void SynchronousInterrupt(void) {
	raw_debug_printf("SynchronousInterrupt\n");
	asm volatile("wfe");
}

void IRQInterrupt(void) {
	raw_debug_printf("IRQInterrupt\n");
	asm volatile("wfe");
}

void FIQInterrupt(void) {
	raw_debug_printf("FIQInterrupt\n");
	asm volatile("wfe");
}
void SErrorInterrupt(void) {
	raw_debug_printf("SErrorInterruptHandler\n");
	asm volatile("wfe");
}