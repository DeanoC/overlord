#include "core/core.h"
#include "utils/boot_psi.h"
#include "platform/memory_map.h"
#include "hw_regs/smmu_reg.h"
#include "hw_regs/crl_apb.h"
#include "hw_regs/crf_apb.h"
#include "hw_regs/lpd_slcr.h"

static PSI_IWord const misc_init[] = {

	PSI_FAR_WRITE_MASKED_32(SMMU_REG, IER_0, 0x8000001FU, 0x8000001FU),
	PSI_FAR_WRITE_MASKED_32(CRF_APB, RST_FPD_TOP, 0x00001F80U, 0x00000000U),
	PSI_FAR_WRITE_MASKED_32(CRL_APB, RST_LPD_TOP, 0x00080000U, 0x00000000U),
	PSI_FAR_WRITE_MASKED_32(LPD_SLCR, AFI_FS, 0x00000300U, 0x00000000U),

	PSI_END_PROGRAM
};


void miscRunInitProgram(void)
{
	psi_RunRegisterProgram(misc_init);
}