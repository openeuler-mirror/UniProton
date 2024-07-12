/*
 * Copyright (c) 2024-2024 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Create: 2024-01-15
 * Description: Hardware Initialization
 */

#include "prt_cpu_external.h"
#include "prt_sys_external.h"
#include "prt_irq_external.h"
#include "prt_hwi.h"
#include "prt_sys.h"
#include "prt_buildef.h"
#include "../hwi/prt_hw_clint.h"

#include "prt_riscv.h"

OS_SEC_ALW_INLINE INLINE U64 get_mstatus()
{
    U64 x;
    OS_EMBED_ASM("csrr %0, mstatus":"=r" (x)::);
    return x;
}
/*
* 描述: 手动触发异常（EL1）
*/
OS_SEC_L4_TEXT void OsAsmIll(void)
{
    OS_EMBED_ASM("ecall");
    return;
}

OS_SEC_L4_TEXT void OsTskContextGet(uintptr_t saveAddr, struct TskContext *context)
{
    *context = *((struct TskContext *)saveAddr);
    return;
}

#define OS_IS_INTERUPT(mcause)     (mcause & 0x8000000000000000ull)
#define OS_IS_EXCEPTION(mcause)    (~(OS_IS_INTERUPT))
#define OS_IS_TICK_INT(mcause)     (mcause == 0x8000000000000007ull)
#define OS_IS_SOFT_INT(mcause)     (mcause == 0x8000000000000003ull)
#define OS_IS_EXT_INT(mcause)      (mcause == 0x800000000000000bull)

OS_SEC_L4_TEXT void intterupt_in(U64 mcause)
{
    if(OS_IS_SOFT_INT(mcause)) {
        return;
    }
    UNI_FLAG |= OS_FLG_HWI_ACTIVE;
    OS_INT_COUNT++;
    return;
}

OS_SEC_L4_TEXT void intterupt_out(U64 mcause)
{
    if(OS_IS_SOFT_INT(mcause)) {
        return;
    }
    OS_INT_COUNT--;
    if (g_intCount == 0) {
        UNI_FLAG &= ~OS_FLG_HWI_ACTIVE;
    }
}

OS_SEC_L4_TEXT void *OsTskContextInit(U32 taskId, U32 stackSize, uintptr_t *topStack, uintptr_t funcTskEntry)
{
    (void)taskId;
    //获得栈基址
    uintptr_t stk_sp = (uintptr_t)topStack + (uintptr_t)stackSize -1;
    //对齐16字节, 对齐8字节也可，防止出现 ld 的硬件未对齐的异常
    stk_sp =  stk_sp & 0xfffffffffffffff0;
    stk_sp -= sizeof(struct TskContext);
    struct TskContext* context = (struct TskContext*)stk_sp;
    context->ra = 0x1;
    context->gp = 0x2;
    context->tp = OsGetCoreID();
    context->t0 = 0x3;
    context->t1 = 0x4;
    context->t2 = 0x5;
    context->s0 = 0x6;
    context->s1 = 0x7;
    context->a0 = 0x8;
    context->a1 = 0x9;
    context->a2 = 0xa;
    context->a3 = 0xb;
    context->a4 = 0xc;
    context->a5 = 0xd;
    context->a6 = 0xe;
    context->a7 = 0xf;
    context->s2 = 0x10;
    context->s3 = 0x11;
    context->s4 = 0x12;
    context->s5 = 0x13;
    context->s6 = 0x14;
    context->s7 = 0x15;
    context->s8 = 0x16;
    context->s9 = 0x17;
    context->s10 = 0x18;
    context->s11 = 0x19;
    context->t3 = 0x20;
    context->t4 = 0x21;
    context->t5 = 0x22;
    context->t6 = 0x23;
    context->mstatus = (get_mstatus() | MPIE_S | MPP);
    context->mepc = (U64)funcTskEntry;
#if defined(OS_ARCH_SURPORT_F)
    context->fs0 = 0x0;
    context->fs1 = 0x0;
    context->fs2 = 0x0;
    context->fs3 = 0x0;
    context->fs4 = 0x0;
    context->fs5 = 0x0;
    context->fs6 = 0x0;
    context->fs7 = 0x0;
    context->fs8 = 0x0;
    context->fs9 = 0x0;
    context->fs10 = 0x0;
    context->fs11 = 0x0;
    context->fcsr = 0x0;
#endif
    return (void*)stk_sp;
}

OS_SEC_L0_TEXT void OsHwiDispatchTail_rv64(void)
{
    U64 irqStartTime = 0;
    if (TICK_NO_RESPOND_CNT > 0) {
        if ((UNI_FLAG & OS_FLG_TICK_ACTIVE) != 0) {
            // OsTskContextLoad， 回到被打断的tick处理现场
            return;
        }
#if defined(OS_OPTION_RR_SCHED) && defined(OS_OPTION_RR_SCHED_IRQ_TIME_DISCOUNT)
        irqStartTime = OsCurCycleGet64();
#endif
        UNI_FLAG |= OS_FLG_TICK_ACTIVE;

        do {
            OsIntEnable();
            // tickISRִ，这里开中断
            g_tickDispatcher();
            OsIntDisable();
            TICK_NO_RESPOND_CNT--;
        } while (TICK_NO_RESPOND_CNT > 0);

        UNI_FLAG &= ~OS_FLG_TICK_ACTIVE;
        OS_IRQ_TIME_RECORD(irqStartTime);
    }
#if defined(OS_OPTION_RR_SCHED)
    OsHwiEndCheckTimeSlice(OsCurCycleGet64());
#endif
}

OS_SEC_L2_TEXT U64 PRT_ClkGetCycleCount64(void)
{
    U64 cycle;
    uintptr_t intSave;
    intSave = PRT_HwiLock();
    cycle = *(U64 *)CLINT_TIME;
    PRT_HwiRestore(intSave);
    return cycle;
}