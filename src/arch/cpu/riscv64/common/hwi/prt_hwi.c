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
 * Create: 2024-01-13
 * Description: 硬中断。
 */

#include "prt_hwi_external.h"
#include "prt_task_external.h"
#include "prt_irq_external.h"
#include "prt_hwi.h"
#include "prt_hw_plic.h"
#include "prt_cpu_external.h"
#include "prt_typedef.h"
#include "prt_hook_external.h"
#include "prt_hwi_internal.h"


OS_SEC_ALW_INLINE INLINE U64 r_mhartid()
{
  U64 x;
  OS_EMBED_ASM("csrr %0, mhartid":"=r" (x)::);
  return x;
}

OS_SEC_L4_TEXT HwiHandle OsHwiGetIrqNumber()
{
    U64 hwiNum = (U64)PLIC_CLAIM(CPU_M_MODE, r_mhartid());
    return hwiNum;
}

OS_SEC_L4_TEXT void OsHwiGICInit(void)
{
    return;
}
    

OS_SEC_L4_TEXT U32 OsHwiPriorityGet(HwiHandle hwiNum)
{
    if(OS_HWI_NUM_CHECK(hwiNum)) {
        return OS_ERRNO_HWI_NUM_INVALID;
    }

    return (U32)PLIC_GET_PRIO(hwiNum);
}

OS_SEC_L4_TEXT void OsHwiPrioritySet(HwiHandle hwiNum, HwiPrior hwiPrio)
{
    if(OS_HWI_NUM_CHECK(hwiNum)) {
        return;
    }

    if(OS_HWI_PRIO_CHECK(hwiPrio)) {
        return;
    }

    PLIC_SET_PRIO(hwiNum,hwiPrio);
}

OS_SEC_L4_TEXT void OsHwiMcTrigger(U32 coreMask, U32 hwiNum)
{
    (void)coreMask;
    (void)hwiNum;
    return;
}

OS_SEC_L4_TEXT void PRT_HwiRestore(uintptr_t intSave)
{
    if(intSave & MIE ) {
        PRT_HwiUnLock();
    } else {
        PRT_HwiLock();
    }
    return;
}

OS_SEC_L4_TEXT uintptr_t PRT_HwiLock(void)
{
    uintptr_t mstatus_r;
    uintptr_t mstatus_w;
    OS_EMBED_ASM("csrr %0,mstatus":"=r"(mstatus_r)::);
    mstatus_w = mstatus_r & (~MIE_S);
    OS_EMBED_ASM("csrw mstatus,%0"::"r"(mstatus_w):);
    return (mstatus_r & MIE);
}

OS_SEC_L4_TEXT uintptr_t PRT_HwiUnLock(void)
{
    uintptr_t mstatus_r;
    uintptr_t mstatus_w;
    OS_EMBED_ASM("csrr %0,mstatus":"=r"(mstatus_r)::);
    mstatus_w = mstatus_r | MIE_S;
    OS_EMBED_ASM("csrw mstatus,%0"::"r"(mstatus_w):);
    return (mstatus_r & MIE);
}

OS_SEC_L4_TEXT U32 PRT_HwiEnable(HwiHandle hwiNum)
{
    if(OS_HWI_NUM_CHECK(hwiNum)) {
        return OS_ERRNO_HWI_NUM_INVALID;
    }
    
    PLIC_ENI(CPU_M_MODE, r_mhartid(), hwiNum);
    return OS_OK;
}

OS_SEC_L4_TEXT U32 PRT_HwiDisable(HwiHandle hwiNum)
{
    if(OS_HWI_NUM_CHECK(hwiNum)) {
        return OS_ERRNO_HWI_NUM_INVALID;
    }

    PLIC_DSI(CPU_M_MODE, r_mhartid(), hwiNum);
    return OS_OK;
}

OS_SEC_L4_TEXT U32 PRT_HwiClearPendingBit(HwiHandle hwiNum)
{
    if(OS_HWI_NUM_CHECK(hwiNum)) {
        return OS_ERRNO_HWI_NUM_INVALID;
    }

    PLIC_COMPLETE(CPU_M_MODE, r_mhartid(), hwiNum);
    return OS_OK;
}

OS_SEC_L4_TEXT void PRT_HwiClearAllPending(void)
{
    for(U32 i =1; i <= OS_HWI_MAX_NUM ;i++) {
        PRT_HwiClearPendingBit(i);
    }
    return;
}

OS_SEC_L4_TEXT U32 PRT_HwiTrigger(U32 dstCore, HwiHandle hwiNum)
{
    if(OS_HWI_NUM_CHECK(hwiNum)) {
        return OS_ERRNO_HWI_NUM_INVALID;
    }

    if (r_mhartid() != dstCore) {
        return OS_ERRNO_HWI_CORE_ID_INVALID;
    }
    
    return OS_ERRNO_HWI_HW_REPORT_HWINO_INVALID;
}

OS_SEC_L4_TEXT U32 PRT_HwiDelExitHook(HwiExitHook hook)
{
    return OsHookDel(OS_HOOK_HWI_EXIT, (OsVoidFunc)hook);
}
OS_SEC_L4_TEXT U32 PRT_HwiAddExitHook(HwiExitHook hook)
{
    return OsHookAdd(OS_HOOK_HWI_EXIT, (OsVoidFunc)hook);
}
OS_SEC_L4_TEXT U32 PRT_HwiDelEntryHook(HwiEntryHook hook)
{
    return OsHookDel(OS_HOOK_HWI_ENTRY, (OsVoidFunc)hook);
}
OS_SEC_L4_TEXT U32 PRT_HwiAddEntryHook(HwiEntryHook hook)
{
    return OsHookAdd(OS_HOOK_HWI_ENTRY, (OsVoidFunc)hook);
}

OS_SEC_L4_TEXT void OsHwiDispatchHandle(U64 archNum)
{
    U64 intSave;
    U32 hwiNum = (U32)archNum;
    if (OS_INT_COUNT > INT_NEST_DEPTH) {
        return;
    }
    
    if (OS_HWI_NUM_CHECK(hwiNum)) {
        OS_GOTO_SYS_ERROR();
    }
    intSave = PRT_HwiLock();
    OsHwiHookDispatcher(hwiNum);
    PRT_HwiClearPendingBit(hwiNum);

    PRT_HwiRestore(intSave);
}