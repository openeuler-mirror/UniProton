/* ----------------------------------------------------------------------------
 * Copyright (c) Huawei Technologies Co., Ltd. 2013-2020. All rights reserved.
 * Description: ARMv7 Exc Implementation
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
 /* ----------------------------------------------------------------------------
 * Modifications Copyright (c) 2024, Greater Bay Area National Center of Technology Innovation
 * Change Logs:  更改文件名，适配UniProton并删除多余函数
 * --------------------------------------------------------------------------- */

#include "prt_exc_internal.h"

#define IS_ALIGNED(val, align)                   (((U32)(val) & ((U32)(align) - 1)) == 0)

#define OS_MAX_BACKTRACE    15U
#define INSTR_SET_MASK      0x01000020U
#define THUMB_INSTR_LEN     2U
#define ARM_INSTR_LEN       4U
#define POINTER_SIZE        4U
#define STATIC

#define GET_FS(fsr) (((fsr) & 0xFU) | (((fsr) & (1U << 10)) >> 6))
#define GET_WNR(dfsr) ((dfsr) & (1U << 11))

extern uintptr_t __exc_stack_top;
// 异常时获取当前任务的信息



STATIC INLINE U32 OsGetDFSR(void)
{
    U32 regDFSR;
    __asm__ __volatile__("mrc p15, 0, %0, c5, c0, 0"
                         : "=r"(regDFSR));
    return regDFSR;
}

STATIC INLINE U32 OsGetIFSR(void)
{
    U32 regIFSR;
    __asm__ __volatile__("mrc p15, 0, %0, c5, c0, 1"
                         : "=r"(regIFSR));
    return regIFSR;
}

STATIC INLINE U32 OsGetDFAR(void)
{
    U32 regDFAR;
    __asm__ __volatile__("mrc p15, 0, %0, c6, c0, 0"
                         : "=r"(regDFAR));
    return regDFAR;
}

STATIC INLINE U32 OsGetIFAR(void)
{
    U32 regIFAR;
    __asm__ __volatile__("mrc p15, 0, %0, c6, c0, 2"
                         : "=r"(regIFAR));
    return regIFAR;
}

void OsCallStackInfo(void)
{
    U32 count = 0;
    struct TagTskCb *runTask = RUNNING_TASK;//LosTaskCB *runTask = OsCurrTaskGet();
    U32 stackBottom = RUNNING_TASK->topOfStack + RUNNING_TASK->stackSize;
    U32 *stackPointer = (U32 *)stackBottom;

    printf("runTask->stackPointer = 0x%x\n", stackPointer);
    printf("runTask->topOfStack = 0x%x\n", runTask->topOfStack);
    printf("text_start:0x%x,text_end:0x%x\n", &__text_start, &__text_end);

    while ((stackPointer > (U32 *)runTask->topOfStack) && (count < OS_MAX_BACKTRACE)) {
        if ((*stackPointer > (U32)(&__text_start)) &&
            (*stackPointer < (U32)(&__text_end)) &&
            IS_ALIGNED((*stackPointer), POINTER_SIZE)) {
            if ((*(stackPointer - 1) > (U32)runTask->topOfStack) &&
                (*(stackPointer - 1) < stackBottom) &&
                IS_ALIGNED((*(stackPointer - 1)), POINTER_SIZE)) {
                count++;
                printf("traceback %u -- lr = 0x%x\n", count, *stackPointer);
            }
        }
        stackPointer--;
    }
    printf("\n");
}

STATIC S32 OsDecodeFS(U32 bitsFS)
{
    switch (bitsFS) {
        case 0x05:  /* 0b00101 */
        case 0x07:  /* 0b00111 */
            printf("Translation fault, %s\n", (bitsFS & 0x2) ? "page" : "section");
            break;
        case 0x09:  /* 0b01001 */
        case 0x0b:  /* 0b01011 */
            printf("Domain fault, %s\n", (bitsFS & 0x2) ? "page" : "section");
            break;
        case 0x0d:  /* 0b01101 */
        case 0x0f:  /* 0b01111 */
            printf("Permission fault, %s\n", (bitsFS & 0x2) ? "page" : "section");
            break;
        default:
            printf("Unknown fault! FS:0x%x. "
                         "Check IFSR and DFSR in ARM Architecture Reference Manual.\n",
                         bitsFS);
            break;
    }

    return OS_OK;
}

STATIC S32 OsDecodeInstructionFSR(U32 regIFSR)
{
    S32 ret;
    U32 bitsFS = GET_FS(regIFSR); /* FS bits[4]+[3:0] */

    ret = OsDecodeFS(bitsFS);
    return ret;
}

STATIC S32 OsDecodeDataFSR(U32 regDFSR)
{
    S32 ret = 0;
    U32 bitWnR = GET_WNR(regDFSR); /* WnR bit[11] */
    U32 bitsFS = GET_FS(regDFSR);  /* FS bits[4]+[3:0] */

    if (bitWnR) {
        printf("Abort caused by a write instruction. ");
    } else {
        printf("Abort caused by a read instruction. ");
    }

    if (bitsFS == 0x01) { /* 0b00001 */
        printf("Alignment fault.\n");
        return ret;
    }
    ret = OsDecodeFS(bitsFS);
    return ret;
}

void OsExcType(U32 excType, struct ExcRegInfo *excRegs)
{
    /* undefinited exception handling or software interrupt */
    if ((excType == OS_EXCEPT_UNDEF_INSTR) || (excType == OS_EXCEPT_SWI)) {
        if ((excRegs->regCPSR & INSTR_SET_MASK) == 0) { /* work status: ARM */
            excRegs->PC = excRegs->PC - ARM_INSTR_LEN;
        } else if ((excRegs->regCPSR & INSTR_SET_MASK) == 0x20) { /* work status: Thumb */
            excRegs->PC = excRegs->PC - THUMB_INSTR_LEN;
        }
    }

    if (excType == OS_EXCEPT_PREFETCH_ABORT) {
        printf("prefetch_abort fault fsr:0x%x, far:0x%0+8x\n", OsGetIFSR(), OsGetIFAR());
        (void)OsDecodeInstructionFSR(OsGetIFSR());
    } else if (excType == OS_EXCEPT_DATA_ABORT) {
        printf("data_abort fsr:0x%x, far:0x%0+8x\n", OsGetDFSR(), OsGetDFAR());
        (void)OsDecodeDataFSR(OsGetDFSR());
    }
}

STATIC const char *g_excTypeString[] = {
    "reset",
    "undefined instruction",
    "software interrupt",
    "prefetch abort",
    "data abort",
    "fiq",
    "address abort",
    "irq"
};

void OsExcSysInfo(U32 excType, const struct ExcRegInfo *excRegs)
{
    struct TagTskCb *runTask = RUNNING_TASK;

    printf("excType:%s\n", g_excTypeString[excType]);
#if (defined(OS_OPTION_TASK_INFO))
    printf("taskName = %s\n", runTask->name);
#endif
    printf("taskId = %u\n", runTask->taskPid);
    printf("task topStack = 0x%x\n", runTask->topOfStack);
    printf("task stackSize = 0x%x\n", runTask->stackSize);
    printf("excRegs pc = 0x%x\n", excRegs->PC);
    printf("excRegs lr = 0x%x\n", excRegs->LR);
    printf("excRegs sp = 0x%x\n", excRegs->SP);
    printf("excRegs fp = 0x%x\n", excRegs->R11);
}

void OsExcRegsInfo(struct ExcRegInfo *excBufAddr)
{
    /*
     * Split register information into two parts:
     * Ensure printing does not rely on memory modules.
     */
    printf("R0         = 0x%x\n", excBufAddr->R0);
    printf("R1         = 0x%x\n", excBufAddr->R1);
    printf("R2         = 0x%x\n", excBufAddr->R2);
    printf("R3         = 0x%x\n", excBufAddr->R3);
    printf("R4         = 0x%x\n", excBufAddr->R4);
    printf("R5         = 0x%x\n", excBufAddr->R5);
    printf("R6         = 0x%x\n", excBufAddr->R6);
    printf("R7         = 0x%x\n", excBufAddr->R7);
    printf("R8         = 0x%x\n", excBufAddr->R8);
    printf("R9         = 0x%x\n", excBufAddr->R9);
    printf("R10        = 0x%x\n", excBufAddr->R10);
    printf("R11        = 0x%x\n", excBufAddr->R11);
    printf("R12        = 0x%x\n", excBufAddr->R12);
    printf("CPSR       = 0x%x\n", excBufAddr->regCPSR);
}
