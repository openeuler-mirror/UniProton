
OUTPUT_ARCH("arm")

ENTRY(main)

SECTIONS
{
  /*--------------------------------------------------------------------*/
  /* Code and read-only segment                                         */
  /*--------------------------------------------------------------------*/

  . = 0x80000000;

  _stext = .;

  /* text: Startup */
  .text :
  {
    KEEP(*(.text.boot))
  }

  /* text: Program code section */
  .text : 
  {
    *(.os*.text)
    *(.text)
    *(.text.*)
  }

  /* rodata: Read-only data */
  .rodata : 
  {
    *(.os*.rodata)
    *(.rodata)
    *(.rodata.*)
	. = ALIGN(4);
    __os_text_start = .;
    LONG(_stext)
    LONG(_etext)
    __os_text_end = .;
	__os_bss_table_start = .;
    LONG(_sbss)
    LONG(_ebss)
    __os_bss_table_end = .;
  }

  _etext = .;

  /*--------------------------------------------------------------------*/
  /* Initialized data segment                                           */
  /*--------------------------------------------------------------------*/

  

  /* data: Writable data */
  .data : 
  {
    _sdata = .;
    *(.os*.data)
    *(.data)
    *(.data.*)
    _edata = .;
  }  
	
  /*--------------------------------------------------------------------*/
  /* Uninitialized data segment                                         */
  /*--------------------------------------------------------------------*/

  .bss : 
  {
    g_bssStart = .;
    __bss_start__ = .;
    _sbss = .;
    *(.os*.bss)
    *(.bss)
    *(.bss.*)
    *(COMMON)
     _ebss = .;
    g_bssEnd = .;
    __bss_end__ = .;
  }

  /* data: System stack */
  .stack :
  {
    . = ALIGN(16);
    g_stackStart  = .;
    . += 0x10000;
    g_stackEnd = .;
  }
  
  .arm.exidx :
  {
    . = ALIGN (4);             
    __os_exidx_start = . ;
	__exidx_start = .;
    *(.ARM.exidx*)
	__exidx_end = .;
    __os_exidx_end = . ;
  }
  
  .arm.extab :
  {
    . = ALIGN (4);             
    *(.ARM.extab*)   
  }
  
  .init           :
  {
    _init = .;
            KEEP (*(.init$00));
            KEEP (*(.init$0[1-9]));
            KEEP (*(.init$[1-8][0-9]));
            KEEP (*(.init$9[0-8]));
    KEEP (*(.init))
    KEEP (*(.init$99));
  }
  
  .fini           :
  {
    _fini = .;
            KEEP (*(.fini$00));
            KEEP (*(.fini$0[1-9]));
            KEEP (*(.fini$[1-8][0-9]));
            KEEP (*(.fini$9[0-8]));
    KEEP (*(.fini))
    KEEP (*(.fini$99));
          PROVIDE (_etext = .);
  }
  
  preinit_array     :
  {
    PROVIDE_HIDDEN (__preinit_array_start = .);
    KEEP (*(.preinit_array))
    PROVIDE_HIDDEN (__preinit_array_end = .);
  }
  
  .init_array     :
  {
    PROVIDE_HIDDEN (__init_array_start = .);
    KEEP (*(SORT_BY_INIT_PRIORITY(.init_array.*)))
    KEEP (*(.init_array))
    PROVIDE_HIDDEN (__init_array_end = .);
  }
  
  .fini_array    :
  {
    PROVIDE_HIDDEN (__fini_array_start = .);
    KEEP (*(SORT_BY_INIT_PRIORITY(.fini_array.*) SORT_BY_INIT_PRIORITY(.dtors.*)))
    KEEP (*(.fini_array EXCLUDE_FILE (*crtbegin.o *crtbegin?.o *crtend.o *crtend?.o ) .dtors))
    PROVIDE_HIDDEN (__fini_array_end = .);
  }

  .ctors          :
  {
    /* gcc uses crtbegin.o to find the start of
       the constructors, so we make sure it is
       first.  Because this is a wildcard, it
       doesn't matter if the user does not
       actually link against crtbegin.o; the
       linker won't look for a file to match a
       wildcard.  The wildcard also means that it
       doesn't matter which directory crtbegin.o
       is in.  */
    KEEP (*crtbegin.o(.ctors))
    KEEP (*crtbegin?.o(.ctors))
    /* We don't want to include the .ctor section from
       the crtend.o file until after the sorted ctors.
       The .ctor section from the crtend file contains the
       end of ctors marker and it must be last */
    KEEP (*(EXCLUDE_FILE (*crtend.o *crtend?.o ) .ctors))
    KEEP (*(SORT(.ctors.*)))
    KEEP (*(.ctors))
  }
  
  .dtors          :
  {
    KEEP (*crtbegin.o(.dtors))
    KEEP (*crtbegin?.o(.dtors))
    KEEP (*(EXCLUDE_FILE (*crtend.o *crtend?.o ) .dtors))
    KEEP (*(SORT(.dtors.*)))
    KEEP (*(.dtors))
  }
  
  end = .;
}
