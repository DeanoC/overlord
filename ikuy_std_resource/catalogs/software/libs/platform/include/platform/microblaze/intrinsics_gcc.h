/******************************************************************************
* Copyright (c) 2004 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
#pragma once

#define mbar(mask) __extension__ ({ asm volatile ("mbar\t" #mask ); })

// Return the current value of the MSR.
#define mfmsr() __extension__ ({ uintptr_t _rval = 0U; asm volatile ( "mfs\t%0,rmsr\n" : "=d"(_rval) ); _rval; })

// Return the current value of the Exception Address Register (EAR).
#define mfear() __extension__ ({ uintptr_t _rval = 0U; asm volatile ("mfs\t%0,rear\n" : "=d"(_rval) ); _rval; })
#define mfeare() __extension__ ({ uintptr_t _rval = 0U; asm volatile ("mfse\t%0,rear\n" : "=d"(_rval)); _rval; })

// Return the current value of the Exception Status Register (ESR).
#define mfesr() __extension__ ({ uintptr_t _rval = 0U; asm volatile ("mfs\t%0,resr\n" : "=d"(_rval)); _rval; })

#define mfpvr(rn) __extension__ ({ uintptr_t _rval = 0U; asm volatile ("mfs\t%0,rpvr" #rn "\n" : "=d"(_rval)); _rval; })

#define mfpvre(rn) __extension__ ({ uintptr_t _rval = 0U; asm volatile ( "mfse\t%0,rpvr" #rn "\n" : "=d"(_rval) ); _rval; })

#define mfbtr() __extension__ ({ uintptr_t _rval = 0U; asm volatile ( "mfs\t%0,rbtr\n" : "=d"(_rval)); _rval; })

#define mfedr() __extension__ ({ uintptr_t _rval = 0U; asm volatile ("mfs\t%0,redr\n" : "=d"(_rval)); _rval; })

#define mfpid() __extension__ ({ uintptr_t _rval = 0U; asm volatile ("mfs\t%0,rpid\n" : "=d"(_rval)); _rval; })

#define mfzpr() __extension__ ({ uintptr_t _rval = 0U; asm volatile ("mfs\t%0,rzpr\n" : "=d"(_rval)); _rval; })

#define mftlbx() __extension__ ({ uintptr_t _rval = 0U; asm volatile ("mfs\t%0,rtlbx\n" : "=d"(_rval)); _rval; })

#define mftlblo() __extension__ ({ uintptr_t _rval = 0U; asm volatile ("mfs\t%0,rtlblo\n" : "=d"(_rval)); _rval; })

#define mftlbhi() __extension__ ({ uintptr_t _rval = 0U; asm volatile ("mfs\t%0,rtlbhi\n" : "=d"(_rval)); _rval; })

#define mfslr() __extension__ ({ uintptr_t _rval = 0U; asm volatile ("mfs\t%0,rslr\n" : "=d"(_rval)); _rval; })

#define mfshr() __extension__ ({ uintptr_t _rval = 0U; asm volatile ("mfs\t%0,rshr\n" : "=d"(_rval)); _rval; })

#define mtgpr(rn, v) ({ asm volatile ("or\t" #rn ",r0,%0\n" :: "d" (v)); })

#define mtmsr(v) ({ asm volatile ("mts\trmsr,%0\n\tnop\n" :: "d" (v)); })

#define mtfsr(v) ({ asm volatile ("mts\trfsr,%0\n\tnop\n" :: "d" (v)); })

#define mtpid(v) ({ asm volatile ("mts\trpid,%0\n\tnop\n" :: "d" (v)); })

#define mtzpr(v) ({ asm volatile ("mts\trzpr,%0\n\tnop\n" :: "d" (v)); })

#define mtslr(v) ({ asm volatile ("mts\trslr,%0\n\tnop\n" :: "d" (v)); })

#define mtshr(v) ({ asm volatile ("mts\trshr,%0\n\tnop\n" :: "d" (v) ); })

#ifdef __cplusplus
EXTERN_C
{
#endif

	void microblaze_enable_exceptions(void);
	void microblaze_disable_exceptions(void);
	void microblaze_enable_interrupts(void);
	void microblaze_disable_interrupts(void);

#ifdef __cplusplus
}
#endif