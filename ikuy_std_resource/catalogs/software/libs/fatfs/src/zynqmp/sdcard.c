#include "core/core.h"
#include "dbg/print.h"
#include "library_defines/library_defines.h"

#if IKUY_HAVE_LIB_XILINX_ZYNQPS8
#include "sdps/xsdps.h"
#include "sdps/xsdps_core.h"

static XSdPs SdInstance;

bool FATFS_ZynqMPSDCardInit() {
	XSdPs_Config *SdConfig;
	int Status;

	debug_print("SD Card Start\n");
	SdConfig = XSdPs_LookupConfig(XPAR_XSDPS_0_DEVICE_ID);
	if (nullptr == SdConfig) {
		return false;
	}

	Status = XSdPs_CfgInitialize(&SdInstance, SdConfig,
															 SdConfig->BaseAddress);
	if (Status != XST_SUCCESS) {
		return false;
	}

	Status = XSdPs_CardInitialize(&SdInstance);
	if (Status != XST_SUCCESS) {
		return false;
	}

	return true;
}

bool FATFS_ZynqMPSDCardRead(uint32_t startSector_, void * buffer_, uint32_t sectors_) {
	return(XSdPs_ReadPolled(&SdInstance, startSector_, sectors_, buffer_) == XST_SUCCESS);
}
bool FATFS_ZynqMPSDCardWrite(uint32_t startSector_, void const * buffer_, uint32_t sectors_) {
	return(XSdPs_WritePolled(&SdInstance, startSector_, sectors_, buffer_) == XST_SUCCESS);
}

void FATFS_ZynqMPSDCardIdle() {
	XSdPs_Idle(&SdInstance);
}

#endif