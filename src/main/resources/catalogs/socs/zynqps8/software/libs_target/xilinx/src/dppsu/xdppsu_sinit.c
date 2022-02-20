/*******************************************************************************
* Copyright (C) 2017 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/******************************************************************************/
/**
 *
 * @file xdppsu_sinit.c
 *
 * This file contains static initialization methods for the XDpPsu driver.
 *
 * @note	None.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.0   aad  05/17/17 Initial release.
 * </pre>
 *
*******************************************************************************/

/******************************* Include Files ********************************/

#include "dppsu/xdppsu.h"
#include "xparameters.h"

#define XPAR_PSU_DP_DEVICE_ID 0
#define XPAR_PSU_DP_BASEADDR 0xfd4a0000
/*************************** Variable Declarations ****************************/
/*************************** Constant Declarations ****************************/

/**
 * A table of configuration structures containing the configuration information
 * for each DisplayPort TX core in the system.
 */
XDpPsu_Config XDpPsu_ConfigTable[1] = {
		{ XPAR_PSU_DP_DEVICE_ID, XPAR_PSU_DP_BASEADDR }
};

/**************************** Function Definitions ****************************/

/******************************************************************************/
/**
 * This function looks for the device configuration based on the unique device
 * ID. The table XDpPsu_ConfigTable[] contains the configuration information for
 * each device in the system.
 *
 * @param	DeviceId is the unique device ID of the device being looked up.
 *
 * @return	A pointer to the configuration table entry corresponding to the
 *		given device ID, or NULL if no match is found.
 *
 * @note	None.
 *
*******************************************************************************/
XDpPsu_Config *XDpPsu_LookupConfig(u16 DeviceId)
{
	XDpPsu_Config *CfgPtr;
	u32 Index;

	for (Index = 0; Index < 1; Index++) {
		if (XDpPsu_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XDpPsu_ConfigTable[Index];
			break;
		}
	}

	return CfgPtr;
}
