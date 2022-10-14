//
// Created by deano on 8/21/22.
//
#include "core/core.h"
#include "dbg/print.h"
#include "platform/cache.h"
#include "platform/reg_access.h"
#include "platform/memory_map.h"
#include "platform/registers/zdma.h"
#include "platform/registers/pmu_global.h"
#include "platform/registers/rpu.h"
#include "platform/registers/crl_apb.h"
#include "utils/busy_sleep.h"
#include "r5f.hpp"

#define XFSBL_ECC_INIT_VAL_WORD 0xDEADBEEFU
#define ZDMA_TRANSFER_MAX_LEN (0x3FFFFFFFU - 7U)

void PowerUpIsland( uint32_t PwrIslandMask ) {

	/* There is a single island for both R5_0 and R5_1 */
	if((PwrIslandMask & PMU_GLOBAL_PWR_STATE_R5_1_MASK) == PMU_GLOBAL_PWR_STATE_R5_1_MASK) {
		PwrIslandMask &= ~(PMU_GLOBAL_PWR_STATE_R5_1_MASK);
		PwrIslandMask |= PMU_GLOBAL_PWR_STATE_R5_0_MASK;
	}

	/* Power up request enable */
	HW_REG_WRITE1( PMU_GLOBAL, REQ_PWRUP_INT_EN, PwrIslandMask );

	/* Trigger power up request */
	HW_REG_WRITE1( PMU_GLOBAL, REQ_PWRUP_TRIG, PwrIslandMask );

	/* Poll for Power up complete */
	do { ;// empty
	} while((HW_REG_READ1( PMU_GLOBAL, REQ_PWRUP_STATUS ) & PwrIslandMask) != 0x0U);
}

void EccInit( uint64_t DestAddr, uint64_t LengthBytes ) {
	uint32_t RegVal;
	uint32_t Length;
	uint64_t StartAddr = DestAddr;
	uint64_t NumBytes = LengthBytes;

	Cache_DCacheDisable();

	while(NumBytes > 0U) {
		if(NumBytes > ZDMA_TRANSFER_MAX_LEN) {
			Length = ZDMA_TRANSFER_MAX_LEN;
		} else {
			Length = (uint32_t) NumBytes;
		}

		// Wait until the DMA is in idle state
		do {
			RegVal = HW_REG_GET_FIELD( HW_REG_GET_ADDRESS( LPD_DMA_CH0 ), ZDMA, ZDMA_CH_STATUS, STATE );
		} while((RegVal != 0U) && (RegVal != 3U)); // OK or ERR need enum support in overlord

		// Enable Simple (Write Only) Mode

		HW_REG_RMW( HW_REG_GET_ADDRESS( LPD_DMA_CH0 ), ZDMA,
		            ZDMA_CH_CTRL0,
		            HW_REG_FIELD_MASK( ZDMA, ZDMA_CH_CTRL0, POINT_TYPE ) | HW_REG_FIELD_MASK( ZDMA, ZDMA_CH_CTRL0, MODE ),
		            ZDMA_ZDMA_CH_CTRL0_MODE_WRITE_ONLY );


		// Fill in the data to be written
		HW_REG_WRITE( HW_REG_GET_ADDRESS( LPD_DMA_CH0 ), ZDMA, ZDMA_CH_WR_ONLY_WORD0, XFSBL_ECC_INIT_VAL_WORD );
		HW_REG_WRITE( HW_REG_GET_ADDRESS( LPD_DMA_CH0 ), ZDMA, ZDMA_CH_WR_ONLY_WORD1, XFSBL_ECC_INIT_VAL_WORD );
		HW_REG_WRITE( HW_REG_GET_ADDRESS( LPD_DMA_CH0 ), ZDMA, ZDMA_CH_WR_ONLY_WORD2, XFSBL_ECC_INIT_VAL_WORD );
		HW_REG_WRITE( HW_REG_GET_ADDRESS( LPD_DMA_CH0 ), ZDMA, ZDMA_CH_WR_ONLY_WORD3, XFSBL_ECC_INIT_VAL_WORD );

		// Write Destination Address (64 bit address though we know this address is <$GiB)
		HW_REG_WRITE( HW_REG_GET_ADDRESS( LPD_DMA_CH0 ), ZDMA, ZDMA_CH_DST_DSCR_WORD0, (uint32_t) (StartAddr & 0xFFFFFFFF));
		HW_REG_WRITE( HW_REG_GET_ADDRESS( LPD_DMA_CH0 ), ZDMA, ZDMA_CH_DST_DSCR_WORD1, (uint32_t) ((StartAddr >> 32U) & 0X0001FFFFU));

		// Size to be Transferred. Recommended to set both src and dest sizes
		HW_REG_WRITE( HW_REG_GET_ADDRESS( LPD_DMA_CH0 ), ZDMA, ZDMA_CH_SRC_DSCR_WORD2, Length );
		HW_REG_WRITE( HW_REG_GET_ADDRESS( LPD_DMA_CH0 ), ZDMA, ZDMA_CH_DST_DSCR_WORD2, Length );

		// DMA Enable
		HW_REG_SET_BIT( HW_REG_GET_ADDRESS( LPD_DMA_CH0 ), ZDMA, ZDMA_CH_CTRL2, EN );

		// Check the status of the transfer by polling on DMA Done
		do { ;// empty
		} while(!HW_REG_GET_BIT( HW_REG_GET_ADDRESS( LPD_DMA_CH0 ), ZDMA, ZDMA_CH_ISR, DMA_DONE ));

		// Clear DMA status
		HW_REG_CLR_BIT( HW_REG_GET_ADDRESS( LPD_DMA_CH0 ), ZDMA, ZDMA_CH_ISR, DMA_DONE );

		// Read the channel status for errors
		if(HW_REG_GET_FIELD( HW_REG_GET_ADDRESS( LPD_DMA_CH0 ), ZDMA, ZDMA_CH_STATUS, STATE ) == 0x3) {
			Cache_DCacheEnable();
			debug_print( "LPD_DMA_CH0 ZDMA Error!" );
			return;
		}

		NumBytes -= Length;
		StartAddr += Length;
	}

	Cache_DCacheEnable();
	// TODO add this to zdma register toml (undocumented feature)
#define ZDMA_ZDMA_CH_CTRL0_TOTAL_BYTE_OFFSET 0X0188U

	// Restore reset values for the DMA registers used
	HW_REG_WRITE( HW_REG_GET_ADDRESS( LPD_DMA_CH0 ), ZDMA, ZDMA_CH_CTRL0, 0x00000080U );
	HW_REG_WRITE( HW_REG_GET_ADDRESS( LPD_DMA_CH0 ), ZDMA, ZDMA_CH_WR_ONLY_WORD0, 0x00000000U );
	HW_REG_WRITE( HW_REG_GET_ADDRESS( LPD_DMA_CH0 ), ZDMA, ZDMA_CH_WR_ONLY_WORD1, 0x00000000U );
	HW_REG_WRITE( HW_REG_GET_ADDRESS( LPD_DMA_CH0 ), ZDMA, ZDMA_CH_WR_ONLY_WORD2, 0x00000000U );
	HW_REG_WRITE( HW_REG_GET_ADDRESS( LPD_DMA_CH0 ), ZDMA, ZDMA_CH_WR_ONLY_WORD3, 0x00000000U );
	HW_REG_WRITE( HW_REG_GET_ADDRESS( LPD_DMA_CH0 ), ZDMA, ZDMA_CH_DST_DSCR_WORD0, 0x00000000U );
	HW_REG_WRITE( HW_REG_GET_ADDRESS( LPD_DMA_CH0 ), ZDMA, ZDMA_CH_DST_DSCR_WORD1, 0x00000000U );
	HW_REG_WRITE( HW_REG_GET_ADDRESS( LPD_DMA_CH0 ), ZDMA, ZDMA_CH_SRC_DSCR_WORD2, 0x00000000U );
	HW_REG_WRITE( HW_REG_GET_ADDRESS( LPD_DMA_CH0 ), ZDMA, ZDMA_CH_DST_DSCR_WORD2, 0x00000000U );
	HW_REG_WRITE( HW_REG_GET_ADDRESS( LPD_DMA_CH0 ), ZDMA, ZDMA_CH_CTRL0_TOTAL_BYTE, 0x00000000U );

	debug_printf( "Address 0x%0x%08x, Length 0x%0x%08x, ECC initialized \r\n",
	                  (uint32_t) (DestAddr >> 32U), (uint32_t) DestAddr,
	                  (uint32_t) (LengthBytes >> 32U), (uint32_t) LengthBytes );
}

#define XFSBL_R50_HIGH_ATCM_START_ADDRESS  (0xFFE00000U)
#define XFSBL_R5_TCM_BANK_LENGTH      (0x10000U)

void TcmInit() {
	uint32_t LengthBytes;
	uint32_t ATcmAddr;
	uint32_t PwrStateMask;

	debug_printf( "Initializing TCM ECC\r\n" );

	// power it up
	PwrStateMask = (PMU_GLOBAL_PWR_STATE_R5_0_MASK |
	                PMU_GLOBAL_PWR_STATE_TCM0A_MASK |
	                PMU_GLOBAL_PWR_STATE_TCM0B_MASK |
	                PMU_GLOBAL_PWR_STATE_TCM1A_MASK |
	                PMU_GLOBAL_PWR_STATE_TCM1B_MASK);
	PowerUpIsland( PwrStateMask );

	// switch into rpu combined mode
	HW_REG_CLR_BIT1( RPU, RPU_GLBL_CNTL, SLSPLIT );
	HW_REG_SET_BIT1( RPU, RPU_GLBL_CNTL, SLCLAMP );
	HW_REG_SET_BIT1( RPU, RPU_GLBL_CNTL, TCM_COMB );

	// Place R5-0 and R5-1 in HALT state
	HW_REG_SET_BIT1( RPU, RPU0_CFG, NCPUHALT );
	HW_REG_SET_BIT1( RPU, RPU1_CFG, NCPUHALT );

	// Enable the clock
	HW_REG_SET_BIT1( CRL_APB, CPU_R5_CTRL, CLKACT );

	// Provide some delay, so that clock propagates properly.
	Utils_BusyMicroSleep( 0x50U );

	// Release reset to R5-0, R5-1 and amba
	HW_REG_CLR_BIT1( CRL_APB, RST_LPD_TOP, RPU_R50_RESET );
	HW_REG_CLR_BIT1( CRL_APB, RST_LPD_TOP, RPU_R51_RESET );
	HW_REG_CLR_BIT1( CRL_APB, RST_LPD_TOP, RPU_AMBA_RESET );

	// init all tcm in one go
	ATcmAddr = XFSBL_R50_HIGH_ATCM_START_ADDRESS;
	LengthBytes = XFSBL_R5_TCM_BANK_LENGTH * 4U;
	EccInit( ATcmAddr, LengthBytes );

	// power it down
	HW_REG_SET_BIT1( CRL_APB, RST_LPD_TOP, RPU_AMBA_RESET );
	HW_REG_SET_BIT1( CRL_APB, RST_LPD_TOP, RPU_R51_RESET );
	HW_REG_SET_BIT1( CRL_APB, RST_LPD_TOP, RPU_R50_RESET );

}
