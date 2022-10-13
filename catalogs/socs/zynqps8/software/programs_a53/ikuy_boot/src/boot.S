
.global _boot

.global __el3_stack
.global _vector_table

.set EL3_stack,		__el3_stack

.set TT_S1_FAULT,	0x0
.set TT_S1_TABLE,	0x3

.set vector_base,	_vector_table
.set rvbar_base,	0xFD5C0040

.set MODE_EL1, 0x5
.set DAIF_BIT,	0x1C0

.section .boot,"ax"
_boot:
	// Set vector table base and reserve vector address
	ldr	    x1, =vector_base
	ldr	    w2, =rvbar_base
	msr	    VBAR_EL3,x1
	str     x1, [x2]

	// Define stack pointer for current exception level
	ldr	 x2,=EL3_stack
	mov	 sp,x2

	// Configure SCR_EL3
	mov      w1, #0              	// Initial value of register is unknown
	orr      w1, w1, #(1 << 11)  	// Set ST bit (Secure EL1 can access CNTPS_TVAL_EL1, CNTPS_CTL_EL1 & CNTPS_CVAL_EL1)
	orr      w1, w1, #(1 << 10)  	// Set RW bit (EL1 is AArch64, as this is the Secure world)
	orr      w1, w1, #(1 << 3)   	// Set EA bit (SError routed to EL3)
	orr      w1, w1, #(1 << 2)   	// Set FIQ bit (FIQs routed to EL3)
	orr      w1, w1, #(1 << 1)   	// Set IRQ bit (IRQs routed to EL3)
	msr      SCR_EL3, x1

	/*configure cpu auxiliary control register EL1 */
	ldr	x0,=0x80CA000 		// L1 Data prefetch control - 5, Enable device split throttle, 2 independent data prefetch streams
	msr	S3_1_C15_C2_0, x0 	//CPUACTLR_EL1

	/*Enable hardware coherency between cores*/
	mrs      x0, S3_1_c15_c2_1  	//Read EL1 CPU Extended Control Register
	orr      x0, x0, #(1 << 6)  	//Set the SMPEN bit
	msr      S3_1_c15_c2_1, x0  	//Write EL1 CPU Extended Control Register
	isb

	// Enable SError Exception for asynchronous abort
	mrs 	x1,DAIF
	bic	x1,x1,#(0x1<<8)
    msr	DAIF,x1

	mov      x1, #0                //Most of the SCTLR_EL3 bits are unknown at reset
	msr      SCTLR_EL3, x1
	dsb	 sy
	isb

	b 	 _startup		//jump to start

.end