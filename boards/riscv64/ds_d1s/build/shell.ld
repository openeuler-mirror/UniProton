/***********************************************************/
/* Configure memory regions */
MEMORY
{
    DDR (rwx)          : ORIGIN = 0x40100000,   LENGTH = 0x9FF000
    SYSSTACK (rwx)      : ORIGIN = 0x40AFF000,   LENGTH = 4K
}

ENTRY(_start)

SECTIONS
{
    .text :
    {
        PROVIDE(__text_start__ = .);
        *(.text .text.*)
        . = ALIGN(16);
        *(.rodata .rodata.*)
        . = ALIGN(16);
        *(.srodata .srodata.*)
        PROVIDE(__text_end__ = .);
    } > DDR

    .data :
    {
        . = ALIGN(16);
        PROVIDE(__data_start__ = .);
        *(.sdata .sdata.*)
        . = ALIGN(16);
        *(.data .data.*)
	. = ALIGN(16);
	*(.uniproton.table.shellcmd.begin)
	*(.uniproton.table.shellcmd.data)
	*(.uniproton.table.shellcmd.finish)
        PROVIDE(__data_end__ = .);
    } > DDR
    .bss (NOLOAD) :
    {
        . = ALIGN(16);
        PROVIDE(__bss_start__ = .);
        *(.sbss .sbss.*)
        . = ALIGN(16);
        *(.bss .bss.*)
        PROVIDE(__bss_end__ = .);
    } > DDR

    .os_stack :
    {
        . = ALIGN(16);
        PROVIDE(__os_sys_sp_start = .);
        PROVIDE(sys_stackStart = .);
        . = . + 4096;
        PROVIDE(sys_stackEnd = .);
        PROVIDE(__os_sys_sp_end = .);
   	. = ALIGN(16);
    	PROVIDE(rmem_start = .);
    } > SYSSTACK
}

/***********************************************************/
