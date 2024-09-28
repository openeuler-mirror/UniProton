/***********************************************************/
/* Configure memory regions */

OUTPUT_ARCH(riscv)
ENTRY(__text_start)

_stack_size = 0x10000;
_heap_size =  0xb0000;

MEMORY
{
	SRAM (rwx) : ORIGIN = 0x83000000, LENGTH = 0x100000
}

SECTIONS
{
    .start_bspinit :
    {
        __text_start = .;
        KEEP(*(.text.bspinit))
    } > SRAM

    .text :
    {
        __text_start__ = .;
        *(.text)
	*(.text.*)
	*(*.text)
        __text_end__ = .;
    } > SRAM

    .rodata : 
    {
	__rodata_start__ = .;
        . = ALIGN(16);
        *(.srodata .srodata.*) /* do not need to distinguish this from .rodata */
        . = ALIGN(16);
        *(.rodata .rodata.*)
	__rodata_end__ = .;
    } > SRAM
	
    .data :
    {
        . = ALIGN(16);
        __data_start__ = .;
        *(.sdata .sdata.*)
        . = ALIGN(16);
        *(.data .data.*)
	. = ALIGN(16);
	uniproton_table_start = .;
	*(.uniproton.table.shellcmd.begin)
        *(.uniproton.table.shellcmd.data)
        *(.uniproton.table.shellcmd.finish)
	uniproton_table_end = .;
        __data_end__ = .;
    } > SRAM

    .bss :
    {
        . = ALIGN(16);
        __bss_start__ = .;
        *(.sbss .sbss.*)
        . = ALIGN(16);
        *(.bss .bss.*)
        __bss_end__ = .;
	. = ALIGN(16);
    } > SRAM

    .stack : ALIGN(16)
    {
        __os_sys_sp_start = .;
	. += _stack_size;
        __os_sys_sp_end = .;
    } > SRAM

    .heap : ALIGN(16)
    {
        heap_start = .;
        PROVIDE (__HEAP_INIT = .);
        . += _heap_size;
        PROVIDE (__HEAP_END = .);
        heap_end = .;
    } > SRAM

    .resource_table : ALIGN(8)
    {
        KEEP(*(.resource_table*))
    } > SRAM  
}

/***********************************************************/
