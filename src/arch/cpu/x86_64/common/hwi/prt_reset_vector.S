#include "prt_buildef.h"
.globl OsResetVector
.globl OsReadCr0
.globl OsReadCr4
.global loop
.global InitBssOs
.global __os_sys_sp_end

OsResetVector:
    movabs  $__os_sys_sp_end, %rsp
    pushq $0
    call InitBssOs
    call OsIdtInit

    movq %cr4, %rax
    bts  $9, %rax
    bts  $10, %rax
    movq %rax, %cr4
    finit
#ifdef OS_GDB_STUB
    call OsGdbStubInit
#endif
    call main

loop :
    hlt
    jmp loop
