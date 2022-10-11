/*
* Copyright (c) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 */

/**
 *
 * PMU_ROM SERVICE EXTENSION IDCODES
 *
 *  All Services implemented in the ROM can be extended or overridden by
 *  firmware loaded into the PMU_RAM. Including the IRQ routing infrastructure.
 *  Aside from the PMU_ROM Service Functions, other extendable/overridable
 *  `hooks` are provided to the firmware. These PMU_ROM Service Hooks are
 *  included in the Extension Table but also have their IDCODES highlighted
 *  below.
 *
 * Before calling the default implementation of these services, ROM checks the
 * index indicated below in the Service Extension Table for a function address.
 * If a function pointer is found, ROM will call that function /instead/ of the
 * default ROM function. However, as an argument to the FW function, is a
 * callback to the default ROM function, thus allowing the overriding FW to
 * implmentent wrapping logic around existing ROM behavior.
 *
 * @note: These Identifiers are also used to identify the service mode
 *        error.
 */
#pragma once

#include "core/core.h"
enum RomExtension_Name {
	REN_TBL_BASE                    = 0U,
	// RESERVED                       1U
	// RESERVED                       2U
	REN_PIT0                        = 3U,
	REN_PIT1                        = 4U,
	REN_PIT2                        = 5U,
	REN_PIT3                        = 6U,
	// RESERVED                       7U
	// RESERVED                       8U
	// RESERVED                       9U
	// RESERVED                       10U
	REN_TMRFAULT                    = 11U,
	REN_GPI1                        = 12U,
	REN_GPI2                        = 13U,
	REN_GPI3                        = 14U,
	// RESERVED                       15U
	REN_COR_ECC_HANDLER             = 16U,
	REN_RTCEVERYSECOND              = 17U,
	REN_RTCALARM                    = 18U,
	REN_IPI0                        = 19U,
	REN_IPI1                        = 20U,
	REN_IPI2                        = 21U,
	REN_IPI3                        = 22U,
	REN_FW_REQS                     = 23U,
	REN_ISO_REQS                    = 24U,
	REN_HWRST                       = 25U,
	REN_SWRST_REQS                  = 26U,
	REN_PWRUP_REQS                  = 27U,
	REN_PWRDN_REQS                  = 28U,
	REN_INVADDR                     = 29U,
	// RESERVED                       30U
	// RESERVED                       31U
	REN_ACPU0WAKE                   = 32U,
	REN_ACPU1WAKE                   = 33U,
	REN_ACPU2WAKE                   = 34U,
	REN_ACPU3WAKE                   = 35U,
	REN_R5F0WAKE                     = 36U,
	REN_R5F1WAKE                     = 37U,
	REN_USB0WAKE                    = 38U,
	REN_USB1WAKE                    = 39U,
	REN_DAPFPDWAKE                  = 40U,
	REN_DAPRPUWAKE                  = 41U,
	REN_MIO0WAKE                    = 42U,
	REN_MIO1WAKE                    = 43U,
	REN_MIO2WAKE                    = 44U,
	REN_MIO3WAKE                    = 45U,
	REN_MIO4WAKE                    = 46U,
	REN_MIO5WAKE                    = 47U,
	REN_FPDGICPROXYWAKE             = 48U,
	// RESERVED                       49U
	// RESERVED                       50U
	// RESERVED                       51U
	REN_ACPU0DBGPWRUP               = 52U,
	REN_ACPU1DBGPWRUP               = 53U,
	REN_ACPU2DBGPWRUP               = 54U,
	REN_ACPU3DBGPWRUP               = 55U,
	// RESERVED                       56U
	// RESERVED                       57U
	// RESERVED                       58U
	// RESERVED                       59U
	REN_ERROR1                      = 60U,
	REN_ERROR2                      = 61U,
	REN_AXIAIBERR                   = 62U,
	REN_APBAIBERR                   = 63U,
	REN_ACPU0SLEEP                  = 64U,
	REN_ACPU1SLEEP                  = 65U,
	REN_ACPU2SLEEP                  = 66U,
	REN_ACPU3SLEEP                  = 67U,
	REN_R5F0SLEEP                    = 68U,
	REN_R5F1SLEEP                    = 69U,
	// RESERVED                       70U
	// RESERVED                       71U
	REN_RCPU0_DBG_RST               = 72U,
	REN_RCPU1_DBG_RST               = 73U,
	// RESERVED                       74U
	// RESERVED                       75U
	// RESERVED                       76U
	// RESERVED                       77U
	// RESERVED                       78U
	// RESERVED                       79U
	REN_ACPU0_CP_RST                = 80U,
	REN_ACPU1_CP_RST                = 81U,
	REN_ACPU2_CP_RST                = 82U,
	REN_ACPU3_CP_RST                = 83U,
	REN_ACPU0_DBG_RST               = 84U,
	REN_ACPU1_DBG_RST               = 85U,
	REN_ACPU2_DBG_RST               = 86U,
	REN_ACPU3_DBG_RST               = 87U,
	// RESERVED                       88U
	// RESERVED                       89U
	// RESERVED                       90U
	// RESERVED                       91U
	// RESERVED                       92U
	REN_VCCAUX_DISCONNECT           = 93U,
	REN_VCCINT_DISCONNECT           = 94U,
	REN_VCCINTFP_DISCONNECT         = 95U,
	REN_PWRUPACPU0                  = 96U,
	REN_PWRUPACPU1                  = 97U,
	REN_PWRUPACPU2                  = 98U,
	REN_PWRUPACPU3                  = 99U,
	REN_PWRUPPP0                    = 100U,
	REN_PWRUPPP1                    = 101U,
	// RESERVED                       102U
	REN_PWRUPL2BANK0                = 103U,
	// RESERVED                       104U
	// RESERVED                       105U
	REN_PWRUPRPU                    = 106U,
	// RESERVED                       107U
	REN_PWRUPTCM0A                  = 108U,
	REN_PWRUPTCM0B                  = 109U,
	REN_PWRUPTCM1A                  = 110U,
	REN_PWRUPTCM1B                  = 111U,
	REN_PWRUPOCMBANK0               = 112U,
	REN_PWRUPOCMBANK1               = 113U,
	REN_PWRUPOCMBANK2               = 114U,
	REN_PWRUPOCMBANK3               = 115U,
	REN_PWRUPUSB0                   = 116U,
	REN_PWRUPUSB1                   = 117U,
	REN_PWRUPFPD                    = 118U,
	REN_PWRUPPLD                    = 119U,
	// RESERVED                       120U
	// RESERVED                       121U
	// RESERVED                       122U
	// RESERVED                       123U
	// RESERVED                       124U
	// RESERVED                       125U
	// RESERVED                       126U
	// RESERVED                       127U
	REN_PWRDNACPU0                  = 128U,
	REN_PWRDNACPU1                  = 129U,
	REN_PWRDNACPU2                  = 130U,
	REN_PWRDNACPU3                  = 131U,
	REN_PWRDNPP0                    = 132U,
	REN_PWRDNPP1                    = 133U,
	// RESERVED                       134U
	REN_PWRDNL2BANK0                = 135U,
	// RESERVED                       136U
	// RESERVED                       137U
	REN_PWRDNRPU                    = 138U,
	// RESERVED                       139U
	REN_PWRDNTCM0A                  = 140U,
	REN_PWRDNTCM0B                  = 141U,
	REN_PWRDNTCM1A                  = 142U,
	REN_PWRDNTCM1B                  = 143U,
	REN_PWRDNOCMBANK0               = 144U,
	REN_PWRDNOCMBANK1               = 145U,
	REN_PWRDNOCMBANK2               = 146U,
	REN_PWRDNOCMBANK3               = 147U,
	REN_PWRDNUSB0                   = 148U,
	REN_PWRDNUSB1                   = 149U,
	REN_PWRDNFPD                    = 150U,
	REN_PWRDNPLD                    = 151U,
	// RESERVED                       152U
	// RESERVED                       153U
	// RESERVED                       154U
	// RESERVED                       155U
	// RESERVED                       156U
	// RESERVED                       157U
	// RESERVED                       158U
	// RESERVED                       159U
	REN_FPISOLATION                 = 160U,
	REN_PLISOLATION                 = 161U,
	REN_PLNONPCAPISO                = 162U,
	// RESERVED                       163U
	REN_FPLOCKISO                   = 164U,
	// RESERVED                       165U
	// RESERVED                       166U
	// RESERVED                       167U
	// RESERVED                       168U
	// RESERVED                       169U
	// RESERVED                       170U
	// RESERVED                       171U
	// RESERVED                       172U
	// RESERVED                       173U
	// RESERVED                       174U
	// RESERVED                       175U
	// RESERVED                       176U
	// RESERVED                       177U
	// RESERVED                       178U
	// RESERVED                       179U
	// RESERVED                       180U
	// RESERVED                       181U
	// RESERVED                       182U
	// RESERVED                       183U
	// RESERVED                       184U
	// RESERVED                       185U
	// RESERVED                       186U
	// RESERVED                       187U
	// RESERVED                       188U
	// RESERVED                       189U
	// RESERVED                       190U
	// RESERVED                       191U
	REN_RSTACPU0                    = 192U,
	REN_RSTACPU1                    = 193U,
	REN_RSTACPU2                    = 194U,
	REN_RSTACPU3                    = 195U,
	REN_RSTAPU                      = 196U,
	// RESERVED                       197U
	REN_RSTPP0                      = 198U,
	REN_RSTPP1                      = 199U,
	REN_RSTGPU                      = 200U,
	REN_RSTPCIE                     = 201U,
	REN_RSTSATA                     = 202U,
	// RESERVED                       203U
	REN_RSTDISPLAYPORT              = 204U,
	// RESERVED                       205U
	// RESERVED                       206U
	// RESERVED                       207U
	REN_RSTR50                      = 208U,
	REN_RSTR51                      = 209U,
	REN_RSTLSRPU                    = 210U,
	// RESERVED                       211U
	REN_RSTGEM0                     = 212U,
	REN_RSTGEM1                     = 213U,
	REN_RSTGEM2                     = 214U,
	REN_RSTGEM3                     = 215U,
	REN_RSTUSB0                     = 216U,
	REN_RSTUSB1                     = 217U,
	// RESERVED                       218U
	REN_RSTIOU                      = 219U,
	REN_RSTPSONLY                   = 220U,
	REN_RSTLPD                      = 221U,
	REN_RSTFPD                      = 222U,
	REN_RSTPLD                      = 223U,
	REN_FW_REQ_0                    = 224U,
	REN_FW_REQ_1                    = 225U,
	REN_FW_REQ_2                    = 226U,
	REN_FW_REQ_3                    = 227U,
	// RESERVED                       228U
	// RESERVED                       229U
	REN_FW_REQ_4                    = 230U,
	REN_FW_REQ_5                    = 231U,
	// RESERVED                       232U
	// RESERVED                       233U
	REN_FW_REQ_6                    = 234U,
	// RESERVED                       35U *
	REN_FW_REQ_7                    = 236U,
	REN_FW_REQ_8                    = 237U,
	// RESERVED                       238U
	// RESERVED                       239U
	REN_FW_REQ_9                    = 240U,
	REN_FW_REQ_10                   = 241U,
	// RESERVED                       242U
	// RESERVED                       243U
	// RESERVED                       244U
	// RESERVED                       245U
	// RESERVED                       246U
	// RESERVED                       247U
	// RESERVED                       248U
	// RESERVED                       249U
	REN_FPD_SUPPLYENABLE            = 250U,
	REN_FPD_SUPPLYDISABLE           = 251U,
	REN_FPD_SUPPLYCHECK             = 252U,
	REN_PLD_SUPPLYENABLE            = 253U,
	REN_PLD_SUPPLYDISABLE           = 254U,
	REN_PLD_SUPPLYCHECK             = 255U,
	REN_TBL_MAX                     = 256U
};

typedef uint32_t (*RomServiceHandler) (void);
typedef uint32_t (*RomServiceExtensionHandler) (RomServiceHandler RomHandler);

extern const RomServiceHandler RomServiceTable[REN_TBL_MAX];
extern RomServiceExtensionHandler RomExtensionTable[REN_TBL_MAX];
