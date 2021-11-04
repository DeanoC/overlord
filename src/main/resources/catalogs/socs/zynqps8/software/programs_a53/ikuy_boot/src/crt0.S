/******************************************************************************
* Copyright (C) 2014 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/*****************************************************************************/
/**
* @file xil-crt0.S
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------
* 5.00	pkp  05/21/14 Initial version
* 5.04  pkp  12/18/15 Initialized global constructor for C++ applications
* 5.04	pkp  01/05/16 Set the reset vector register RVBAR equivalent to
*		      vector table base address
* 6.02  pkp  01/22/17 Added support for EL1 non-secure
* 6.6   srm  10/18/17 Added timer configuration using XTime_StartTTCTimer API.
*		      Now the TTC instance as specified by the user will be
*	              started.
* 6.6   mus  01/29/18 Initialized the xen PV console for Cortexa53 64 bit
*                     EL1 NS BSP.
* 7.2   sd   02/23/20 Clock Init is called
* 7.2   sd   02/23/20 Clock code added under XCLOCKING flag
* </pre>
*
* @note
*
* None.
*
******************************************************************************/
	.file	"crt0.S"
	.section ".got2","aw"
	.align	2

	.text
.Lsbss_start:
	.quad	__sbss_start

.Lsbss_end:
	.quad	__sbss_end

.Lbss_start:
	.quad	__bss_start__

.Lbss_end:
	.quad	__bss_end__


.set APU_PWRCTL,	0xFD5C0090

	.globl	_startup
_startup:

	mov	x0, #0
	/* clear sbss */
	ldr 	x1,.Lsbss_start		/* calculate beginning of the SBSS */
	ldr	x2,.Lsbss_end		/* calculate end of the SBSS */
.Lloop_sbss:
	cmp	x1,x2
	bge	.Lenclsbss		/* If no SBSS, no clearing required */
	str	x0, [x1], #8
	b	.Lloop_sbss

.Lenclsbss:
	/* clear bss */
	ldr	x1,.Lbss_start		/* calculate beginning of the BSS */
	ldr	x2,.Lbss_end		/* calculate end of the BSS */

.Lloop_bss:
	cmp	x1,x2
	bge	.Lenclbss		/* If no BSS, no clearing required */
	str	x0, [x1], #8
	b	.Lloop_bss

.Lenclbss:

	/* make sure argc and argv are valid */
	mov	x0, #0
	mov	x1, #0

	bl	main			/* Jump to main C code */

.Lexit:
	b .Lexit

.Lstart:
	.size	_startup,.Lstart-_startup