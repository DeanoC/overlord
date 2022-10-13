/******************************************************************************
* Copyright (c) 2014 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
.org 0
.text

.globl _boot
.globl _vector_table

.globl FIQInterrupt
.globl IRQInterrupt
.globl SErrorInterrupt
.globl SynchronousInterrupt

.macro saveregister
	stp	X0,X1, [sp,#-0x10]!
	stp	X2,X3, [sp,#-0x10]!
	stp	X4,X5, [sp,#-0x10]!
	stp	X6,X7, [sp,#-0x10]!
	stp	X8,X9, [sp,#-0x10]!
	stp	X10,X11, [sp,#-0x10]!
	stp	X12,X13, [sp,#-0x10]!
	stp	X14,X15, [sp,#-0x10]!
	stp	X16,X17, [sp,#-0x10]!
	stp X18,X19, [sp,#-0x10]!
	stp X29,X30, [sp,#-0x10]!
.endm

.macro restoreregister
	ldp X29,X30, [sp], #0x10
	ldp X18,X19, [sp], #0x10
	ldp	X16,X17, [sp], #0x10
	ldp	X14,X15, [sp], #0x10
	ldp	X12,X13, [sp], #0x10
	ldp	X10,X11, [sp], #0x10
	ldp	X8,X9, [sp], #0x10
	ldp	X6,X7, [sp], #0x10
	ldp	X4,X5, [sp], #0x10
	ldp	X2,X3, [sp], #0x10
	ldp	X0,X1, [sp], #0x10
.endm

.org 0

.section .vectors, "a"

_vector_table:
.set	VBAR, _vector_table
.org VBAR
	b	_boot
.org (VBAR + 0x200)
	b	SynchronousInterruptHandler

.org (VBAR + 0x280)
	b	IRQInterruptHandler

.org (VBAR + 0x300)
	b	FIQInterruptHandler

.org (VBAR + 0x380)
	b	SErrorInterruptHandler


SynchronousInterruptHandler:
	saveregister
	bl	SynchronousInterrupt
	eret

IRQInterruptHandler:

	saveregister
    /* Save the status of SPSR, ELR and CPTR to stack */
	mrs x0, CPTR_EL3
	mrs x1, ELR_EL3
	mrs	x2, SPSR_EL3
	stp	x0, x1, [sp,#-0x10]!
	str	x2, [sp,#-0x10]!

	bl	IRQInterrupt

    /* Restore the status of SPSR, ELR and CPTR from stack */
	ldr	x2,[sp],0x10
	ldp	x0, x1, [sp],0x10
	msr	CPTR_EL3, x0
	msr	ELR_EL3, x1
	msr	SPSR_EL3, x2
	restoreregister
	eret

FIQInterruptHandler:

	saveregister

	bl	FIQInterrupt

	restoreregister

	eret

SErrorInterruptHandler:

	saveregister

	bl      SErrorInterrupt

	restoreregister

    eret
.end