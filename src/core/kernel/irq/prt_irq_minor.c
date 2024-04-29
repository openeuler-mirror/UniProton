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
 * Create: 2024-01-25
 * Description: 中断模块
 */
#include "prt_irq_internal.h"
#include "prt_buildef.h"

#if defined(OS_OPTION_SMP)
OS_SEC_BSS volatile uintptr_t g_hwiLock;
#endif

#if defined(OS_OPTION_HWI_AFFINITY)
OS_SEC_L4_TEXT U32 OsHwiAffinityGet(HwiHandle hwiNum)
{
    struct TagHwiModeForm *form = OS_HWI_MODE_ATTR(OS_HWI2IRQ(hwiNum));

    return form->affinityMask;
}

/*
 * 描述：中断绑定接口
 */
 OS_SEC_L2_TEXT U32 PRT_HwiSetAffinity(U32 hwiNum, U32 coreMask)
 {
    uintptr_t intSave;
    U32 validMask;
    U32 ret;
    U32 irqNum = OS_HWI2IRQ(hwiNum);

    if(OS_HWI_NUM_CHECK(hwiNum)) {
        return OS_ERRNO_HWI_NUM_INVALID;
    }
    
    validMask = coreMask & OS_ALLCORES_MASK;
    if(validMask == 0) {
        return OS_ERRNO_HWI_AFFINITY_MASK_INVALID;
    }

    /* SGI不支持绑定，其他绑定依赖硬件实现，OS不做逐一判断 */
    if (!OS_HWI_AFFINITY_CHECK(hwiNum)) {
        return OS_ERRNO_HWI_AFFINITY_HWINUM_SGI;
    }

    OS_HWI_IRQ_LOCK(intSave);
    ret = OsHwiAffinitySet(hwiNum, validMask);
    if(ret != OS_OK){
        OS_HWI_IRQ_UNLOCK(intSave);
        return ret;
    }
    /* 控制块记录 */
    OsHwiAffinityMaskSet(irqNum, validMask);

    OS_HWI_IRQ_UNLOCK(intSave);
    return OS_OK;
 }
#endif
 #if defined(OS_OPTION_SMP)
 /*
  * 描述：触发核间SGI中断
  */
 OS_SEC_TEXT U32 PRT_HwiMcTrigger(enum OsHwiIpiType type, U32 coreMask, U32 hwiNum)
 {
    U32 validMask = coreMask & OS_ALLCORES_MASK;
    if (type >= OS_TYPE_TRIGGER_BUTT) {
        return OS_ERRNO_HWI_TRIGGER_TYPE_INVALID;
    }

    if (!OS_HWI_IS_SGI(hwiNum)) {
        return OS_ERRNO_HWI_TRIGGER_HWINUM_NOT_SGI;
    }

    if((type == OS_TYPE_TRIGGER_BY_MASK) && (validMask == 0)) {
        return OS_ERRNO_HWI_TRIGGER_MASK_INVALID;
    }

    OsHwiMcTrigger(type, validMask, hwiNum);

    return OS_OK;
 }
/*
 * 描述: 中断处理流程尾部处理，TICK 、任务调用
 * 备注: NA
 */
OS_SEC_TEXT void OsHwiDispatchTail(void)
{
    struct TagOsRunQue *rq = THIS_RUNQ();

    if (UNLIKELY(rq->tickNoRespondCnt > 0)) {
        if ((rq->uniFlag & OS_FLG_TICK_ACTIVE) != 0) {
            return;
        }
        rq->uniFlag |= OS_FLG_TICK_ACTIVE;

        do {
            OsIntEnable();
            g_tickDispatcher();
            OsIntDisable();
            rq->tickNoRespondCnt--;
        } while (rq->tickNoRespondCnt > 0);

        rq->uniFlag &= ~OS_FLG_TICK_ACTIVE;
    }

    if (rq->uniTaskLock == 0) {
        rq->shakeCount++;
        PRT_DMB();
    }
    OsMainSchedule();
}

OS_SEC_L4_TEXT void OsHwiSplLock(void)
{
    OS_HWI_SPL_LOCK();
}

OS_SEC_L4_TEXT void OsHwiSplUnLock(void)
{
    OS_HWI_SPL_UNLOCK();
}

#endif