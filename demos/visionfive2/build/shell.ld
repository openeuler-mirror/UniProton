/***********************************************************/
/* Configure memory regions */
MEMORY
{
    DDR (rwx)   : ORIGIN = 0x40000000,  LENGTH = 0x80000000
}

ENTRY(_start)

SECTIONS
{
    .start :
    {
        _start_system = .;
        *(.text._start)
    } > DDR

    .text :
    {
        __text_start__ = .;
        *(.text .text.*)
        . = ALIGN(16);
        *(.rodata .rodata.*)
        . = ALIGN(16);
        *(.srodata .srodata.*)
        __text_end__ = .;
    } > DDR

    .data :
    {
        . = ALIGN(16);
        __data_start__ = .;
        *(.sdata .sdata.*)
        . = ALIGN(16);
        *(.data .data.*)
	. = ALIGN(16);
	*(.uniproton.table.shellcmd.begin)
	*(.uniproton.table.shellcmd.data)
	*(.uniproton.table.shellcmd.finish)
        __data_end__ = .;
    } > DDR

    .bss :
    {
        . = ALIGN(16);
        __bss_start__ = .;
        *(.sbss .sbss.*)
        . = ALIGN(16);
        *(.bss .bss.*)
        __bss_end__ = .;
    } > DDR
    . = ALIGN(4096);
    .os_stack :
    {
        . = ALIGN(16);
        __os_sys_sp_start = .;
        sys_stackStart = .;
        . += 0x10000;
        sys_stackEnd = .;
        __os_sys_sp_end = .;
    } > DDR

    PROVIDE(rmem_start = .);
}