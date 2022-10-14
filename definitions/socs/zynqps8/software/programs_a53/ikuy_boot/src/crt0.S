.file	"crt0.S"
.section .boot,"ax"
.align	2

.text
.globl	_startup

_startup:
	mov	x0, #0
	// clear sbss
	ldr x1, =__sbss_start
	ldr	x2, =__sbss_end
.loopsbss:
	cmp	x1, x2
	bge	.endsbss		/* If no SBSS, no clearing required */
	str	x0, [x1], #8
	b	.loopsbss

.endsbss:
	// clear bss
	ldr	x1, =__bss_start__
	ldr	x2, =__bss_end__
.loopbss:
	cmp	x1,x2
	bge	.endbss
	str	x0, [x1], #8
	b	.loopbss

.endbss:
	// make sure argc and argv are valid and jump to main
	mov	x0, #0
	mov	x1, #0
	b	main
