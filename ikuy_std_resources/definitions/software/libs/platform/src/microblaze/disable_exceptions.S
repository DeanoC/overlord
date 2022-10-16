/******************************************************************************
* Copyright (c) 2004 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
// Disable exceptions on microblaze.
	.text
	.globl	microblaze_disable_exceptions
	.ent	microblaze_disable_exceptions
	.align 2
microblaze_disable_exceptions:
	rtsd    r15, 8
  msrclr  r0, 0x100
.end microblaze_disable_exceptions