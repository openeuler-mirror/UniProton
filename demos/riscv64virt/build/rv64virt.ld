/***********************************************************/
/* Configure memory regions */
MEMORY
{
    IROM (rwx)          : ORIGIN = 0x80000000,   LENGTH = 0x04200000
    SRAM (rwx)          : ORIGIN = 0x84200000,   LENGTH = 0x0BFF0000
    SYSSTACK (rwx)      : ORIGIN = 0x901F0000,   LENGTH = 0x00010000
}

ENTRY(_start)

SECTIONS
{
    .start :
    {
        _start_system = .;
        *(.text._start)
    } > IROM

    .text :
    {
        __text_start__ = .;
        *(.text .text.*)
        . = ALIGN(16);
        *(.rodata .rodata.*)
        . = ALIGN(16);
        *(.srodata .srodata.*)
        __text_end__ = .;
    } > IROM

    .data :
    {
        . = ALIGN(16);
        __data_start__ = .;
        *(.sdata .sdata.*)
        . = ALIGN(16);
        *(.data .data.*)
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
    } > SRAM

    .os_stack :
    {
        . = ALIGN(16);
        __os_sys_sp_start = .;
        sys_stackStart = .;
        . = . + 0x10000;
        sys_stackEnd = .;
        __os_sys_sp_end = .;
    } > SYSSTACK
    . += 0x10;
    PROVIDE(rmem_start = .);
}

/***********************************************************/
