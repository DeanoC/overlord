/* needs to have 8K space free during early MMU function */
_STACK_SIZE = DEFINED(_STACK_SIZE) ? _STACK_SIZE : 9K;

/* Enough for registers + PMU Monitor */
_HEAP_SIZE = DEFINED(_HEAP_SIZE) ? _HEAP_SIZE : 40K ;

MEMORY
{
   ocm_sram_BASEADDR : ORIGIN = 0xFFFC0000, LENGTH = 256K
}

ENTRY(_vector_table)

SECTIONS
{
.text : {
   KEEP (*(.vectors))
   *(.boot)
   *(.text)
   *(.text.*)
   *(.gnu.linkonce.t.*)
   *(.plt)
   *(.gnu_warning)
   *(.gcc_execpt_table)
   *(.glue_7)
   *(.glue_7t)
   *(.ARM.extab)
   *(.gnu.linkonce.armextab.*)
} > ocm_sram_BASEADDR

.note.gnu.build-id : {
   KEEP (*(.note.gnu.build-id))
} > ocm_sram_BASEADDR

.init (ALIGN(64)): {
   KEEP (*(.init))
} > ocm_sram_BASEADDR

.fini (ALIGN(64)): {
   KEEP (*(.fini))
} > ocm_sram_BASEADDR

.interp : {
   __interp_start = .;
   KEEP (*(.interp))
   __interp_end = .;
} > ocm_sram_BASEADDR

.note-ABI-tag : {
   __note-ABI-tag_start = .;
   KEEP (*(.note-ABI-tag))
   __note-ABI-tag_end = .;
} > ocm_sram_BASEADDR

.rodata (ALIGN(64)): {
   __rodata_start = .;
   *(.rodata)
   *(.rodata.*)
   *(.gnu.linkonce.r.*)
   __rodata_end = .;
} > ocm_sram_BASEADDR

.rodata1 (ALIGN(64)): {
   __rodata1_start = .;
   *(.rodata1)
   *(.rodata1.*)
   __rodata1_end = .;
} > ocm_sram_BASEADDR

.sys_cfg_data (ALIGN(64)): {
   *(.sys_cfg_data)
} > ocm_sram_BASEADDR

.eh_frame : {
  *(.eh_frame)
} > ocm_sram_BASEADDR

.eh_framehdr : {
   __eh_framehdr_start = .;
   *(.eh_framehdr)
   __eh_framehdr_end = .;
} > ocm_sram_BASEADDR

.gcc_except_table : {
   *(.gcc_except_table)
} > ocm_sram_BASEADDR

.data (ALIGN(64)): {
   __data_start = .;
   *(.data)
   *(.data.*)
   *(.gnu.linkonce.d.*)
   *(.jcr)
   *(.got)
   *(.got.plt)
   __data_end = .;

} > ocm_sram_BASEADDR

.data1 (ALIGN(64)): {
   __data1_start = .;
   *(.data1)
   *(.data1.*)
   __data1_end = .;
} > ocm_sram_BASEADDR

.tdata (ALIGN(64)): {
   __tdata_start = .;
   *(.tdata)
   *(.tdata.*)
   *(.gnu.linkonce.td.*)
   __tdata_end = .;
} > ocm_sram_BASEADDR

.got : {
   *(.got)
} > ocm_sram_BASEADDR

.got1 : {
   *(.got1)
} > ocm_sram_BASEADDR

.got2 : {
   *(.got2)
} > ocm_sram_BASEADDR

.fixup : {
   __fixup_start = .;
   *(.fixup)
   __fixup_end = .;
} > ocm_sram_BASEADDR

.ARM.exidx : {
   __exidx_start = .;
   *(.ARM.exidx*)
   *(.gnu.linkonce.armexidix.*.*)
   __exidx_end = .;
} > ocm_sram_BASEADDR

.preinit_array (ALIGN(64)): {
   __preinit_array_start = .;
   KEEP (*(SORT(.preinit_array.*)))
   KEEP (*(.preinit_array))
   __preinit_array_end = .;
} > ocm_sram_BASEADDR

.ARM.attributes : {
   __ARM.attributes_start = .;
   *(.ARM.attributes)
   __ARM.attributes_end = .;
} > ocm_sram_BASEADDR

.sdata (ALIGN(64)): {
    __sdata_start = .;
   *(.sdata)
   *(.sdata.*)
   *(.gnu.linkonce.s.*)
   __sdata_end = .;
} > ocm_sram_BASEADDR

.sdata2 (ALIGN(64)): {
   __sdata2_start = .;
   *(.sdata2)
   *(.sdata2.*)
   *(.gnu.linkonce.s2.*)
   __sdata2_end = .;
} > ocm_sram_BASEADDR

.sbss (NOLOAD) : {
   __sbss_start = .;
 . = ALIGN(64);
   *(.sbss)
   *(.sbss.*)
   *(.gnu.linkonce.sb.*)
    . = ALIGN(64);
   __sbss_end = .;
} > ocm_sram_BASEADDR

.sbss2 (ALIGN(64)): {
   __sbss2_start = .;
   *(.sbss2)
   *(.sbss2.*)
   *(.gnu.linkonce.sb2.*)
   __sbss2_end = .;
} > ocm_sram_BASEADDR

.tbss (ALIGN(64)): {
   __tbss_start = .;
   *(.tbss)
   *(.tbss.*)
   *(.gnu.linkonce.tb.*)
   __tbss_end = .;
} > ocm_sram_BASEADDR

.bss (NOLOAD) : {
   . = ALIGN(64);
   __bss_start__ = .;
   *(.bss)
   *(.bss.*)
   *(.gnu.linkonce.b.*)
   *(COMMON)
   . = ALIGN(64);
   __bss_end__ = .;
} > ocm_sram_BASEADDR

_SDA_BASE_ = __sdata_start + ((__sbss_end - __sdata_start) / 2 );

_SDA2_BASE_ = __sdata2_start + ((__sbss2_end - __sdata2_start) / 2 );

/* Generate Stack and Heap definitions */
.heap : {
   . = ALIGN(64);
   _heap = .;
   HeapBase = .;
   _heap_start = .;
   *(.hwregs)
   *(.programs)
   _heap_tmp = .;
   . += (_HEAP_SIZE - (_heap_tmp - _heap_start));
   _heap_end = .;
   HeapLimit = .;
} > ocm_sram_BASEADDR

.stack (NOLOAD) : {
   . = ALIGN(4096);
   _el3_stack_end = .;
   . += _STACK_SIZE;
   __el3_stack = .;
   __el2_stack = .;
   __el1_stack = .;
   __el0_stack = .;
} > ocm_sram_BASEADDR

_end = .;
}