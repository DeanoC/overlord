#include "core/core.h"
#include "platform/reg_access.h"
#include "utils/boot_psi.h"
//#include "platform/memory_map.h"
//#include "platform/registers/crl_apb.h"
//#include "platform/registers/crf_apb.h"

#include "psu_init.h"

unsigned long psu_ddr_qos_init_data(void)
{
	/*
	* AFI INTERCONNECT QOS CONFIGURATION
	*/
	/*
	* Register : AFIFM_RDQoS @ 0XFD360008

	* Sets the level of the QoS field to be used for the read channel 4'b0000:
	*  Lowest Priority' ' '4'b1111: Highest Priority
	*  PSU_AFIFM0_AFIFM_RDQOS_VALUE                                0

	* QoS Read Channel Register
	* (OFFSET, MASK, VALUE)      (0XFD360008, 0x0000000FU ,0x00000000U)
	*/
	PSU_Mask_Write(AFIFM0_AFIFM_RDQOS_OFFSET, 0x0000000FU, 0x00000000U);
	/*##################################################################### */

	/*
	* Register : AFIFM_WRQoS @ 0XFD36001C

	* Sets the level of the QoS field to be used for the write channel 4'b0000
	* : Lowest Priority' ' '4'b1111: Highest Priority
	*  PSU_AFIFM0_AFIFM_WRQOS_VALUE                                0

	* QoS Write Channel Register
	* (OFFSET, MASK, VALUE)      (0XFD36001C, 0x0000000FU ,0x00000000U)
	*/
	PSU_Mask_Write(AFIFM0_AFIFM_WRQOS_OFFSET, 0x0000000FU, 0x00000000U);
	/*##################################################################### */

	/*
	* Register : AFIFM_RDQoS @ 0XFD370008

	* Sets the level of the QoS field to be used for the read channel 4'b0000:
	*  Lowest Priority' ' '4'b1111: Highest Priority
	*  PSU_AFIFM1_AFIFM_RDQOS_VALUE                                0

	* QoS Read Channel Register
	* (OFFSET, MASK, VALUE)      (0XFD370008, 0x0000000FU ,0x00000000U)
	*/
	PSU_Mask_Write(AFIFM1_AFIFM_RDQOS_OFFSET, 0x0000000FU, 0x00000000U);
	/*##################################################################### */

	/*
	* Register : AFIFM_WRQoS @ 0XFD37001C

	* Sets the level of the QoS field to be used for the write channel 4'b0000
	* : Lowest Priority' ' '4'b1111: Highest Priority
	*  PSU_AFIFM1_AFIFM_WRQOS_VALUE                                0

	* QoS Write Channel Register
	* (OFFSET, MASK, VALUE)      (0XFD37001C, 0x0000000FU ,0x00000000U)
	*/
	PSU_Mask_Write(AFIFM1_AFIFM_WRQOS_OFFSET, 0x0000000FU, 0x00000000U);
	/*##################################################################### */

	/*
	* Register : AFIFM_RDQoS @ 0XFD380008

	* Sets the level of the QoS field to be used for the read channel 4'b0000:
	*  Lowest Priority' ' '4'b1111: Highest Priority
	*  PSU_AFIFM2_AFIFM_RDQOS_VALUE                                0

	* QoS Read Channel Register
	* (OFFSET, MASK, VALUE)      (0XFD380008, 0x0000000FU ,0x00000000U)
	*/
	PSU_Mask_Write(AFIFM2_AFIFM_RDQOS_OFFSET, 0x0000000FU, 0x00000000U);
	/*##################################################################### */

	/*
	* Register : AFIFM_WRQoS @ 0XFD38001C

	* Sets the level of the QoS field to be used for the write channel 4'b0000
	* : Lowest Priority' ' '4'b1111: Highest Priority
	*  PSU_AFIFM2_AFIFM_WRQOS_VALUE                                0

	* QoS Write Channel Register
	* (OFFSET, MASK, VALUE)      (0XFD38001C, 0x0000000FU ,0x00000000U)
	*/
	PSU_Mask_Write(AFIFM2_AFIFM_WRQOS_OFFSET, 0x0000000FU, 0x00000000U);
	/*##################################################################### */

	/*
	* Register : AFIFM_RDQoS @ 0XFD390008

	* Sets the level of the QoS field to be used for the read channel 4'b0000:
	*  Lowest Priority' ' '4'b1111: Highest Priority
	*  PSU_AFIFM3_AFIFM_RDQOS_VALUE                                0

	* QoS Read Channel Register
	* (OFFSET, MASK, VALUE)      (0XFD390008, 0x0000000FU ,0x00000000U)
	*/
	PSU_Mask_Write(AFIFM3_AFIFM_RDQOS_OFFSET, 0x0000000FU, 0x00000000U);
	/*##################################################################### */

	/*
	* Register : AFIFM_WRQoS @ 0XFD39001C

	* Sets the level of the QoS field to be used for the write channel 4'b0000
	* : Lowest Priority' ' '4'b1111: Highest Priority
	*  PSU_AFIFM3_AFIFM_WRQOS_VALUE                                0

	* QoS Write Channel Register
	* (OFFSET, MASK, VALUE)      (0XFD39001C, 0x0000000FU ,0x00000000U)
	*/
	PSU_Mask_Write(AFIFM3_AFIFM_WRQOS_OFFSET, 0x0000000FU, 0x00000000U);
	/*##################################################################### */

	/*
	* Register : AFIFM_RDQoS @ 0XFD3A0008

	* Sets the level of the QoS field to be used for the read channel 4'b0000:
	*  Lowest Priority' ' '4'b1111: Highest Priority
	*  PSU_AFIFM4_AFIFM_RDQOS_VALUE                                0

	* QoS Read Channel Register
	* (OFFSET, MASK, VALUE)      (0XFD3A0008, 0x0000000FU ,0x00000000U)
	*/
	PSU_Mask_Write(AFIFM4_AFIFM_RDQOS_OFFSET, 0x0000000FU, 0x00000000U);
	/*##################################################################### */

	/*
	* Register : AFIFM_WRQoS @ 0XFD3A001C

	* Sets the level of the QoS field to be used for the write channel 4'b0000
	* : Lowest Priority' ' '4'b1111: Highest Priority
	*  PSU_AFIFM4_AFIFM_WRQOS_VALUE                                0

	* QoS Write Channel Register
	* (OFFSET, MASK, VALUE)      (0XFD3A001C, 0x0000000FU ,0x00000000U)
	*/
	PSU_Mask_Write(AFIFM4_AFIFM_WRQOS_OFFSET, 0x0000000FU, 0x00000000U);
	/*##################################################################### */

	/*
	* Register : AFIFM_RDQoS @ 0XFD3B0008

	* Sets the level of the QoS field to be used for the read channel 4'b0000:
	*  Lowest Priority' ' '4'b1111: Highest Priority
	*  PSU_AFIFM5_AFIFM_RDQOS_VALUE                                0

	* QoS Read Channel Register
	* (OFFSET, MASK, VALUE)      (0XFD3B0008, 0x0000000FU ,0x00000000U)
	*/
	PSU_Mask_Write(AFIFM5_AFIFM_RDQOS_OFFSET, 0x0000000FU, 0x00000000U);
	/*##################################################################### */

	/*
	* Register : AFIFM_WRQoS @ 0XFD3B001C

	* Sets the level of the QoS field to be used for the write channel 4'b0000
	* : Lowest Priority' ' '4'b1111: Highest Priority
	*  PSU_AFIFM5_AFIFM_WRQOS_VALUE                                0

	* QoS Write Channel Register
	* (OFFSET, MASK, VALUE)      (0XFD3B001C, 0x0000000FU ,0x00000000U)
	*/
	PSU_Mask_Write(AFIFM5_AFIFM_WRQOS_OFFSET, 0x0000000FU, 0x00000000U);
	/*##################################################################### */

	/*
	* Register : AFIFM_RDQoS @ 0XFF9B0008

	* Sets the level of the QoS field to be used for the read channel 4'b0000:
	*  Lowest Priority' ' '4'b1111: Highest Priority
	*  PSU_AFIFM6_AFIFM_RDQOS_VALUE                                0

	* QoS Read Channel Register
	* (OFFSET, MASK, VALUE)      (0XFF9B0008, 0x0000000FU ,0x00000000U)
	*/
	PSU_Mask_Write(AFIFM6_AFIFM_RDQOS_OFFSET, 0x0000000FU, 0x00000000U);
	/*##################################################################### */

	/*
	* Register : AFIFM_WRQoS @ 0XFF9B001C

	* Sets the level of the QoS field to be used for the write channel 4'b0000
	* : Lowest Priority' ' '4'b1111: Highest Priority
	*  PSU_AFIFM6_AFIFM_WRQOS_VALUE                                0

	* QoS Write Channel Register
	* (OFFSET, MASK, VALUE)      (0XFF9B001C, 0x0000000FU ,0x00000000U)
	*/
	PSU_Mask_Write(AFIFM6_AFIFM_WRQOS_OFFSET, 0x0000000FU, 0x00000000U);
	/*##################################################################### */


	return 1;
}
