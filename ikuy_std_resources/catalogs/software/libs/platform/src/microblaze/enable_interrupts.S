/******************************************************************************
* Copyright (c) 2004 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
// Enable interrupts on the microblaze.
	.text
	.globl	microblaze_enable_interrupts
	.ent	microblaze_enable_interrupts
	.align	2
microblaze_enable_interrupts:
	rtsd	r15, 8
	msrset  r0, 0x2
	nop
	.end	microblaze_enable_interrupts