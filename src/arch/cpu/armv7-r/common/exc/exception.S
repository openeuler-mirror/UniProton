/* ----------------------------------------------------------------------------
 * Copyright (c) Huawei Technologies Co., Ltd. 2013-2020. All rights reserved.
 * Description: ARMv7 Hw Exc Implementation
 * Author: Huawei LiteOS Team
 * Create: 2013-01-01
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 * conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 * of conditions and the following disclaimer in the documentation and/or other materials
 * provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific prior written
 * permission.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * --------------------------------------------------------------------------- */

#include "prt_exc_internal.h"

    .extern   g_intCount
    .extern   g_curNestCount
    .extern   OsExcHandleEntry

    .global   _osExceptFiqHdl
    .global   _osExceptAddrAbortHdl
    .global   _osExceptDataAbortHdl
    .global   _osExceptPrefetchAbortHdl
    .global   _osExceptSwiHdl
    .global   _osExceptUndefInstrHdl
    .global   OsGdbHandleException

    .fpu vfpv4


@ Description: Undefined instruction exception handler
_osExceptUndefInstrHdl:
                                                              @ LR offset to return from this exception:  0.
    STMFD   SP, {R0-R7}                                       @ Push working registers, but don`t change SP.

    MOV     R0, #OS_EXCEPT_UNDEF_INSTR                        @ Set exception ID to OS_EXCEPT_UNDEF_INSTR.

    B       _osExceptDispatch                                 @ Branch to global exception handler.

@ Description: Software interrupt exception handler
_osExceptSwiHdl:
    STMFD   SP!, {LR}                                         @ Store PC
    STMFD   SP!, {LR}
    STMFD   SP!, {SP}
    STMFD   SP!, {R0-R12}                                     @ Store SP,LR,R0-R12

    MRS     R1, SPSR                                          @ Save exception`s CPSR.
    STMFD   SP!, {R1}                                         @ Push task`s CPSR (i.e. exception SPSR).

    MOV     R0, #OS_EXCEPT_SWI                                @ Set exception ID to OS_EXCEPT_SWI.
    MOV     R5, SP

    B       _osExceptionSwi                                   @ Branch to global exception handler.

@ Description: Prefectch abort exception handler
_osExceptPrefetchAbortHdl:
    SUB     LR, LR, #4                                        @ LR offset to return from this exception: -4.
    STMFD   SP, {R0-R7}                                       @ Push working registers, but don`t change SP.

    MOV     R0, #OS_EXCEPT_PREFETCH_ABORT                     @ Set exception ID to OS_EXCEPT_PREFETCH_ABORT.

    B       _osExceptDispatch                                 @ Branch to global exception handler.


@ Description: Data abort exception handler
_osExceptDataAbortHdl:
    SUB     LR, LR, #8                                        @ LR offset to return from this exception: -8.
    STMFD   SP, {R0-R7}                                       @ Push working registers, but don`t change SP.

    MOV     R0, #OS_EXCEPT_DATA_ABORT                         @ Set exception ID to OS_EXCEPT_DATA_ABORT.

    B       _osExceptDispatch                                 @ Branch to global exception handler.

@ Description: Address abort exception handler
_osExceptAddrAbortHdl:
    SUB     LR, LR, #8                                        @ LR offset to return from this exception: -8.
    STMFD   SP, {R0-R7}                                       @ Push working registers, but don`t change SP.

    MOV     R0, #OS_EXCEPT_ADDR_ABORT                         @ Set exception ID to OS_EXCEPT_ADDR_ABORT.

    B       _osExceptDispatch                                 @ Branch to global exception handler.

@ Description: Fast interrupt request exception handler
_osExceptFiqHdl:
    SUB     LR, LR, #4                                        @ LR offset to return from this exception: -4.
    STMFD   SP, {R0-R7}                                       @ Push working registers.

    MOV     R0, #OS_EXCEPT_FIQ                                @ Set exception ID to OS_EXCEPT_FIQ.

    B       _osExceptDispatch                                 @ Branch to global exception handler.

@ Description: Exception handler
@ Parameter  : R0     Exception Type
@ Regs Hold  : R3     Exception`s CPSR
_osExceptDispatch:
    MRS     R1, SPSR                                          @ Save CPSR before exception.
    MOV     R2, LR                                            @ Save PC before exception.
    SUB     R3, SP, #(8 * 4)                                  @ Save the start address of working registers.

    MSR     CPSR_c, #(PSR_INT_DISABLE | PSR_SVC_MODE)       @ Switch to SVC mode, and disable all interrupts
    MOV     R5, SP

    EXC_SP_SET __exc_stack_top, OS_ARCH_SYS_STACK_SIZE, R6, R7

    STMFD   SP!, {R2}                                         @ Push Exception PC
    STMFD   SP!, {LR}
    STMFD   SP!, {R5}                                         @ Push original SP,
    STMFD   SP!, {R8-R12}                                     @ Push original R12-R8,
    LDMFD   R3!, {R4-R11}                                     @ Move original R7-R0 from exception stack to original stack.
    STMFD   SP!, {R4-R11}
    STMFD   SP!, {R1}                                         @ Push task`s CPSR (i.e. exception SPSR).

_osExceptionSwi:
    MOV     R1, SP

    LDR     R2, =g_curNestCount                               @ if(g_curNestCount > 0) dump to _osExceptionGetSP
    LDR     R4, [R2]

    CMP     R4, #0
    BNE     _osExceptionGetSP


    LDR     R3, =g_intCount                                   @ Judge the exception is occur in task stack or system stack
    LDR     R2, [R3]

    CMP     R2, #0                                            @ if (g_intCount > 0)
    BNE     _osExceptionGetSP                                 @ can not switch svc stack

    EXC_SP_SET __svc_stack_top, OS_ARCH_SVC_STACK_SIZE, R6, R7 @ Switch to unified exception stack.

    ADD     R2, R2, #1
    STR     R2, [R3]

_osExceptionGetSP:
    LDR     R2, =OsExcHandleEntry                             @ OsExcHandleEntry(UINT32 excType, ExcRegInfo * excBufAddr)

    MOV     LR, PC
    BX      R2

    .end
