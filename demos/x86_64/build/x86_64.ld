OUTPUT_FORMAT(elf64-x86-64)
OUTPUT_ARCH(i386:x86-64)
ENTRY(start)

SECTIONS
{
    . = 0xf02400000;
    __openamp_phys_addr = .;
    __os_print_start = .;
    . = . + 0x100000;
    __os_print_end = .;

    . = 0xf02600000;
    .text : {
        *(.startup.text)
        *(.text)
        *(.text.*)
        __os_stub_text_start = .;
        *(.stub.text)
        __os_stub_text_end = .;
        *(.rodata)
        *(.rodata.*)
    }

    .init_section : {
        PROVIDE_HIDDEN (__init_array_start = .);
        _sinit = ABSOLUTE(.);
        KEEP(*(.init_array .init_array.*))
        _einit = ABSOLUTE(.);
        PROVIDE_HIDDEN (__init_array_end = .);
    }

    .fini_section : {
        PROVIDE_HIDDEN (__fini_array_start = .);
        _sinit = ABSOLUTE(.);
        KEEP(*(.fini_array .fini_array.*))
        _einit = ABSOLUTE(.);
        PROVIDE_HIDDEN (__fini_array_end = .);
    }

    . = 0xf02a00000;
    .data : {
        *(.data)
        *(.data.*)
        *(.tdata)
        __os_stub_data_start = .;
        *(.stub.data)
        __os_stub_data_end = .;
    }

    .bss : {
        __bss_start = .;
        *(*.bss)
        *(.bss.*)
        *(.tbss)
        __bss_end = .;
    }

    __os_sys_sp_start = .;
    . = . + 0x200000;
    __os_sys_sp_end = .;
}
