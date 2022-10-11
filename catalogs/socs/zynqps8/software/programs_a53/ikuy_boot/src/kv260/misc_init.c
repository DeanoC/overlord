#include "core/core.h"
#include "dbg/raw_print.h"
#include "dbg/print.h"
#include "platform/reg_access.h"
#include "utils/boot_psi.h"
#include "platform/memory_map.h"
#include "platform/registers/crl_apb.h"
#include "platform/registers/crf_apb.h"
#include "platform/registers/fpd_slcr.h"

__attribute__((__section__(".hwregs")))
static PSI_IWord const afi_init[] = {
	PSI_FAR_WRITE_MASKED_32(CRF_APB, RST_FPD_TOP, 0x00001F80U, 0x00000000U),
	PSI_FAR_WRITE_MASKED_32(CRL_APB, RST_LPD_TOP, 0x00001F80U, 0x00000000U),
	PSI_FAR_WRITE_MASKED_32(FPD_SLCR, AFI_FS, 0x00000F00U, 0x00000A00U),

	PSI_END_PROGRAM
};
void afiRunInitProgram() {
	psi_RunRegisterProgram(afi_init);
}

#if 0
unsigned long psu_afi_config(void)
{
	/*
	* AFI RESET
	*/
	/*
	* Register : RST_FPD_TOP @ 0XFD1A0100

	* AF_FM0 block level reset
	*  PSU_CRF_APB_RST_FPD_TOP_AFI_FM0_RESET                       0

	* AF_FM1 block level reset
	*  PSU_CRF_APB_RST_FPD_TOP_AFI_FM1_RESET                       0

	* AF_FM2 block level reset
	*  PSU_CRF_APB_RST_FPD_TOP_AFI_FM2_RESET                       0

	* AF_FM3 block level reset
	*  PSU_CRF_APB_RST_FPD_TOP_AFI_FM3_RESET                       0

	* AF_FM4 block level reset
	*  PSU_CRF_APB_RST_FPD_TOP_AFI_FM4_RESET                       0

	* AF_FM5 block level reset
	*  PSU_CRF_APB_RST_FPD_TOP_AFI_FM5_RESET                       0

	* FPD Block level software controlled reset
	* (OFFSET, MASK, VALUE)      (0XFD1A0100, 0x00001F80U ,0x00000000U)
	*/
	PSU_Mask_Write(CRF_APB_RST_FPD_TOP_OFFSET, 0x00001F80U, 0x00000000U);
	/*##################################################################### */

	/*
	* Register : RST_LPD_TOP @ 0XFF5E023C

	* AFI FM 6
	*  PSU_CRL_APB_RST_LPD_TOP_AFI_FM6_RESET                       0

	* Software control register for the LPD block.
	* (OFFSET, MASK, VALUE)      (0XFF5E023C, 0x00080000U ,0x00000000U)
	*/
	PSU_Mask_Write(CRL_APB_RST_LPD_TOP_OFFSET, 0x00080000U, 0x00000000U);
	/*##################################################################### */

	/*
	* AFIFM INTERFACE WIDTH
	*/
	/*
	* Register : afi_fs @ 0XFD615000

	* Select the 32/64/128-bit data width selection for the Slave 0 00: 32-bit
	*  AXI data width (default) 01: 64-bit AXI data width 10: 128-bit AXI data
	*  width 11: reserved
	*  PSU_FPD_SLCR_AFI_FS_DW_SS0_SEL                              0x2

	* Select the 32/64/128-bit data width selection for the Slave 1 00: 32-bit
	*  AXI data width (default) 01: 64-bit AXI data width 10: 128-bit AXI data
	*  width 11: reserved
	*  PSU_FPD_SLCR_AFI_FS_DW_SS1_SEL                              0x2

	* afi fs SLCR control register. This register is static and should not be
	* modified during operation.
	* (OFFSET, MASK, VALUE)      (0XFD615000, 0x00000F00U ,0x00000A00U)
	*/
	PSU_Mask_Write(FPD_SLCR_AFI_FS_OFFSET, 0x00000F00U, 0x00000A00U);
	/*##################################################################### */


	return 1;
}

#endif
