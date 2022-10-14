/******************************************************************************
* Copyright (c) 2020 - 2021 Dean Calver.  All rights reserved.
* Copyright (c) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

_STACK_SIZE = DEFINED(_STACK_SIZE) ? _STACK_SIZE : 4K;

/* Define Memories in the system */
MEMORY
{
	PMU_ROM (rx) : ORIGIN = 0xFFD00000, LENGTH = 256K
	PMU_RAM (rwx): ORIGIN = 0xFFDC0000, LENGTH = 0x1F6E0	/* ~125.7 k */
	/* Unknown gap here */
	PMU_RAM_EXT_TABLE (rwx): ORIGIN = 0xFFDDF6E0, LENGTH = 1K	/* 1 k */
}

/* Specify the default entry point to the program */
ENTRY(_boot)

/* Define the sections, and where they are mapped in memory */

SECTIONS
{
.text : {
   KEEP (*(.vectors.reset))
   KEEP (*(.vectors.sw_exception))
   KEEP (*(.vectors.interrupt))
   KEEP (*(.vectors.break))
   KEEP (*(.vectors.hw_exception))

/* technically upto 0x50 is reserved but as this is a hard core the range from 0x28 to 0x50 is usable */
   . = ALIGN(0x28);

   *(.text)
   *(.text.*)
   *(.gnu.linkonce.t.*)
} > PMU_RAM

.note.gnu.build-id : {
    PROVIDE(g_note_build_id = .);
    KEEP (*(.note.gnu.build-id))
} > PMU_RAM

.rodata : {
   __rodata_start = .;
   *(.rodata)
   *(.rodata.*)
   *(.gnu.linkonce.r.*)
   __rodata_end = .;
} > PMU_RAM

.data : {
   . = ALIGN(4);
   __data_start = .;
   *(.data)
   *(.data.*)
   *(.gnu.linkonce.d.*)
   __data_end = .;
} > PMU_RAM

.bss (NOLOAD) : {
   . = ALIGN(4);
   __bss_start = .;
   *(.bss)
   *(.bss.*)
   *(.gnu.linkonce.b.*)
   *(COMMON)
   . = ALIGN(4);
   __bss_end = .;
} > PMU_RAM

/DISCARD/ : {
   *(.tdata)
   *(.tdata.*)
   *(.gnu.linkonce.td.*)
   *(.tbss)
   *(.tbss.*)
   *(.gnu.linkonce.tb.*)
}

/* rw small data section */
.sdata : {
   . = ALIGN(8);
   __sdata_start = .;
   *(.sdata)
   *(.sdata.*)
   *(.gnu.linkonce.s.*)
   __sdata_end = .;
} > PMU_RAM

.sbss (NOLOAD) : {
   . = ALIGN(4);
   __sbss_start = .;
   *(.sbss)
   *(.sbss.*)
   *(.gnu.linkonce.sb.*)
   . = ALIGN(8);
   __sbss_end = .;
} > PMU_RAM
/* must be halfway between contigous sdata and sbdd */
_RW_SMALL_DATA_BASE_ = __sdata_start + ((__sbss_end - __sdata_start) / 2 );

/* ro small data section */
.sdata2 : {
   . = ALIGN(8);
   __sdata2_start = .;
   *(.sdata2)
   *(.sdata2.*)
   *(.gnu.linkonce.s2.*)
   __sdata2_end = .;
} > PMU_RAM

.sbss2 (NOLOAD) : {
   . = ALIGN(4);
   __sbss2_start = .;
   *(.sbss2)
   *(.sbss2.*)
   *(.gnu.linkonce.sb2.*)
   . = ALIGN(8);
   __sbss2_end = .;
} > PMU_RAM
/* must be halfway between contigous sdata and sbdd */
_RO_SMALL_DATA_BASE_ = __sdata2_start + ((__sbss2_end - __sdata2_start) / 2 );


/* Generate Stack definitions */
.stack (NOLOAD) : {
   _stack_end = .;
   . += _STACK_SIZE;
   . = ALIGN(8);
   _stack = .;
   __stack = _stack;
} > PMU_RAM

.romexttable (NOLOAD) : {
   __romexttable_start = .;
   KEEP (*(.RomExtensionTable))
   __romexttable_end = .;
} > PMU_RAM_EXT_TABLE

_end = .;

RomServiceTable = ORIGIN(PMU_ROM) + 0x7bc8;

}