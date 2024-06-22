/***********************************************************/
/* Configure memory regions */
MEMORY
{
    SHMEM (rwx)		: ORIGIN = 0x90200000,   LENGTH = 0x00020000
    SHMEM_SELF (rwx)    : ORIGIN = 0x90220000,   LENGTH = 0x00001000
    IROM (rwx)          : ORIGIN = 0x92C21000,   LENGTH = 0x02000000
    SRAM (rwx)          : ORIGIN = 0x94C21000,   LENGTH = 0x08FF0000
    SYSSTACK (rwx)      : ORIGIN = 0x9DC11000,   LENGTH = 0x00010000
    HEAP (rwx)		: ORIGIN = 0x9DC21000,   LENGTH = 0x01000000
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
    
    .sh_mem :
    {
	. = ALIGN(16);
	rpmsg_lite_base = .;
	. = . + 0x20000;
	rpmsg_lite_end = .;
    } > SHMEM

    .sh_mem_self :
    {
        . = ALIGN(16);
        shmem_start = .;
        . = . + 0x1000;
        shmem_end = .;
    } > SHMEM_SELF

    .heap :
    {
	. = ALIGN(16);
	rmem_start = .;
	. = . + 0x01000000;
	rmem_end = .;
    } > HEAP
}

/***********************************************************/
