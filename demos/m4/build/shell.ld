/***********************************************************/
/* Configure memory regions */
MEMORY
{
    IROM (rwx)          : ORIGIN = 0x08000000,   LENGTH = 0x00100000
    SRAM (rwx)          : ORIGIN = 0x20000000,   LENGTH = 0x0001A000
    SYSSTACK (rwx)      : ORIGIN = 0x2001A000,   LENGTH = 0x2000
}

ENTRY(OsResetVector)

SECTIONS
{
    .resetzone :
    {
        __text_section_start = .;
	KEEP(*(.reset))
    } > IROM

    .text :
    {
        . = ALIGN(4);
	__text_start__ = .;
	*(.text*)
	*(.rodata*)
	*(.kernel*)
	*(.rel*)
    } > IROM

    .ARM.exidx :
    {
        *(.ARM.exidx*  .gnu.linkonce.armexidx.*)
	. = ALIGN(4);
    } > IROM

    .data : AT(LOADADDR(.ARM.exidx) + SIZEOF(.ARM.exidx))
    {
        __text_end__ = LOADADDR(.data);
	__data_start__ = .;
	KEEP(*(VECTOR))
	*(vtable)
	*(.data*)
    KEEP(*( SORT (.uniproton.table.*)));
	. = ALIGN(4);
	__data_end__ = .;
    } > SRAM

    .bss :
    {
        . = ALIGN(4);
	__bss_start__ = .;
	g_bssStart = .;
	*(.bss*)
	*(COMMON)
	. = ALIGN(4);
	__bss_end__ = .;
	g_bssEnd = .;
    } > SRAM

    .osStack :
    {
        g_stackStart = .;
	. = . + 0x2000;
	g_stackEnd = .;
    } > SYSSTACK

    .heap (COPY):
    {
        __end__ = .;
	end = __end__;
	*(.heap*)
	__HeapLimit = .;
    } > SYSSTACK
}

/***********************************************************/
