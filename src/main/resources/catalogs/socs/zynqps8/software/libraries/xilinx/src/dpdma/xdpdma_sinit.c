/*******************************************************************************
* Copyright (C) 2017 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/******************************************************************************/
/**
 *
 * @file xdpdma_sinit.c
 * @addtogroup dpdma_v1_4
 * @{
 *
 * This file contains static initialization methods for the XDpDma driver.
 *
 * @note	None.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.0   aad  01/20/15 Initial release.
 * </pre>
 *
*******************************************************************************/

/******************************* Include Files ********************************/

#include "dpdma/xdpdma.h"
#include "xparameters.h"

#define XPAR_PSU_DPDMA_DEVICE_ID 0
#define XPAR_PSU_DPDMA_BASEADDR 0xFD4C0000

XDpDma_Config XDpDma_ConfigTable[1] = {
		XPAR_PSU_DPDMA_DEVICE_ID,
		XPAR_PSU_DPDMA_BASEADDR
};
/**************************** Function Definitions ****************************/

/******************************************************************************/
/**
 * This function looks for the device configuration based on the unique device
 * ID. The table XDpDma_ConfigTable[] contains the configuration information for
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
XDpDma_Config *XDpDma_LookupConfig(u16 DeviceId)
{
	XDpDma_Config *CfgPtr = NULL;
	u32 Index;

	for (Index = 0U; Index < (u32)1; Index++) {
		if (XDpDma_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XDpDma_ConfigTable[Index];
			break;
		}
	}

	return CfgPtr;
}
/** @} */
