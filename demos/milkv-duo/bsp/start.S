
#include "prt_buildef.h"
#include "csr.h"
.type     start, function
.section .text.bspinit, "ax"
.align 4
.extern __os_sys_sp_end
.extern g_ped_ops
.extern cpup_shellcmd
.extern help_shellcmd
.extern memInfo_shellcmd
.extern systeminfo_shellcmd
.extern taskInfo_shellcmd
.extern uname_shellcmd
.extern main
start:
	# save boot firmware's parameter
	la a1, g_ped_ops
	sd a0, 0(a1)
	la a1, cpup_shellcmd
	la a1, help_shellcmd
 	la a1, memInfo_shellcmd
	la a1, systeminfo_shellcmd
	la a1, taskInfo_shellcmd
	la a1, uname_shellcmd
	# clear normal registers
        li x1, 0
        li x2, 0
        li x3, 0
        li x4, 0
        li x5, 0
        li x6, 0
        li x7, 0
        li x8, 0
        li x9, 0
        li x10, 0
        li x11, 0
        li x12, 0
        li x13, 0
        li x14, 0
        li x15, 0
        li x16, 0
        li x17, 0
        li x18, 0
        li x19, 0
        li x20, 0
        li x21, 0
        li x22, 0
        li x23, 0
        li x24, 0
        li x25, 0
        li x26, 0
        li x27, 0
        li x28, 0
        li x29, 0
        li x30, 0
        li x31, 0
/*
	// invalidate all memory for BTB,BHT,DCACHE,ICACHE
        li x3, 0x30013
        csrs mcor, x3
        // enable ICACHE,DCACHE,BHT,BTB,RAS,WA
        li x3, 0x7f
        csrs mhcr, x3
        // enable data_cache_prefetch, amr
        li x3, 0x610c
        csrs mhint, x3 #mhint	
*/
	# init csr registers
	csrw mtvec	, zero

	# disable mstatus-mie 
	csrc mstatus, 8
	
	# enable EIE,TIE,SIE interrupt
        li x3, 0x880
        csrw mie, x3
	
	# enable fp
        li x3, 0x1 << 13
        csrs mstatus, x3
	
	# init sysstack
	la sp, __os_sys_sp_end

	# call to main
	call main
_err:
	wfi
	j _err

