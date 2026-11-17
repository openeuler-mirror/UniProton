ENTRY(__text_start)

_stack_size = 0x10000;
_heap_size = 0x10000;

MEMORY
{
    IMU_SRAM (rwx) : ORIGIN = 0x40000000, LENGTH = 0x1000000
}

SECTIONS
{
    /* zimage64_hdr header */
    .header : {
        LONG(0x14000010)          /* magic0: ZIMAGE64_MAGIC_V0 */
        LONG(0)                   /* res0: Reserved */
        QUAD(0)                   /* text_offset: Adjust based on your rambase (v_start = rambase + text_offset) */
        QUAD(0)                   /* res1: Reserved */
        QUAD(0)                   /* res2: Reserved */
        QUAD(0)                   /* res3: Reserved */
        QUAD(0)                   /* res4: Reserved */
        QUAD(0)                   /* res5: Reserved */
        LONG(0x644d5241)          /* magic1: ZIMAGE64_MAGIC_V1 ("ARM\x64") */
        LONG(0)                   /* res6: Reserved */
        . = ALIGN(8);             /* 8-byte alignment for 64-bit fields */
    } > IMU_SRAM

    text_start = .;
    .start_bspinit :
    {
        __text_start = .;
        KEEP(*(.text.bspinit))
    } > IMU_SRAM

    .start_text :
    {
        KEEP(*(.text.startup))
    } > IMU_SRAM

    .text :
    {
        *(.text)
        *(.text.*)
        *(*.text)
        . = ALIGN(8);
        os_symtab_start = .;
        KEEP(*(.OsSymTab))
        os_symtab_end = .;
        __text_end = .;
    } > IMU_SRAM
    . = ALIGN(8);
    data_copy_start = .;

    .rodata :
    {
        . = ALIGN(8);
        __rodata_start = .;
        *(.rodata)
        *(.rodata.*)
        . = ALIGN(8);
        __rodata_end = .;
    } > IMU_SRAM

    .eh_frame :
    {
        . = ALIGN(8);
        __os_unwind_table_start = .;
        *(.eh_frame)
        __os_unwind_table_end = .;
    } > IMU_SRAM

    .heap (NOLOAD) :
    {
        . = ALIGN(8);
        PROVIDE (__HEAP_INIT = .);
        . = . + _heap_size;
        . = ALIGN(8);
        PROVIDE (__HEAP_END = .);
    } > IMU_SRAM

    .stack (NOLOAD) :
    {
        . = ALIGN(16);
        PROVIDE (__os_sys_sp_start = .);
        . = . + _stack_size;
        . = ALIGN(16);
        PROVIDE (__os_sys_sp_end = .);
    } > IMU_SRAM
    end = .;

    .percpu.data :
    {
        __os_per_cpu_start = .;
        *(.os.percpu.data)
        __os_per_cpu_end = .;
        LONG (ALIGNOF(.percpu.data))
    } > IMU_SRAM

    .data :
    {
        . = ALIGN(8);
        __data_start = .;
        *(.data)
        *(.data.*)
        KEEP(*( SORT (.uniproton.table.*)));
        . = ALIGN(8);
        __os_text_start = .;
        QUAD(__text_start)
        QUAD(__text_end)
        __os_text_end = .;
        __data_end = .;
    } > IMU_SRAM

    .llt.bss :
    {
        _llt_bss_start = .;
        *__code_measure_stub*.o(.bss)
        *__code_measure_stub*.o(.bss.*)
        _llt_bss_end = .;
    } > IMU_SRAM

    .bss (NOLOAD) :
    {
        . = ALIGN(8);
        __bss_start__ = .;
        *(.bss)
        *(.bss.*)
        *(COMMON)
        . = ALIGN(8);
        __bss_end__ = .;
    } > IMU_SRAM

    .resource_table : ALIGN(8)
    {
	KEEP(*(.resource_table*))
    } > IMU_SRAM
}

