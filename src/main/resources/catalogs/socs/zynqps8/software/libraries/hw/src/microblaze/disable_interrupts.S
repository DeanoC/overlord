/******************************************************************************
* Copyright (c) 2004 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
// Disable interrupts on the microblaze.

	.text
	.globl	microblaze_disable_interrupts
	.ent	microblaze_disable_interrupts
	.align	2
microblaze_disable_interrupts:
	rtsd	r15, 8
	msrclr  r0, 0x2
	.end	microblaze_disable_interrupts