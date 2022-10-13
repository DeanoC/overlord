/******************************************************************************
*
* Copyright (c) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/
.globl Handoff

Handoff:
	mov x30, x0 /* move the destination address into x30 register */

	tlbi ALLE3   /* invalidate All E3 translation tables */
	ic IALLU   /* invalidate I Cache All to PoU, Inner Shareable */

	dsb sy
	isb 			 /* make sure it completes */

	mrs x5, SCTLR_EL3	 /* Read control register */
	mov x6, #0x1005  	 /* D, I , M bits disable */
    bic x5, x5, x6 	 /* Disable MMU, L1 and L2 I/D cache */
    msr SCTLR_EL3, x5

	isb
LoopWFE:
	wfe				    /* wait for event */
	b LoopWFE

.end