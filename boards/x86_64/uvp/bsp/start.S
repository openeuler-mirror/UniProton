.code64
.section .startup.text
.global start
.extern OsResetVector
.global __os_sys_sp_end

start:
    movabs $__os_sys_sp_end, %rsp
    jmp    OsResetVector
loop:
    hlt
    jmp loop