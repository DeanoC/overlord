/******************************************************************************
* Copyright (c) 2004 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
// Enable exceptions on microblaze.
	.text
	.globl	microblaze_enable_exceptions
	.ent	microblaze_enable_exceptions
	.align 2
microblaze_enable_exceptions:
	rtsd    r15, 8;
  msrset  r0, 0x100
.end microblaze_enable_exceptions