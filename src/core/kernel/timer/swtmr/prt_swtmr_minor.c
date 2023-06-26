/*
 * Copyright (c) 2009-2022 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Create: 2009-12-22
 * Description: 软件定时器模块的C文件
 */
#include "prt_swtmr_internal.h"

OS_SEC_L4_TEXT U32 OsSwTmrGetRemain(struct TagSwTmrCtrl *swtmr)
{
    U32 retValue = 0;
    U32 temp;
    struct TagSwTmrCtrl *timer = NULL;
    struct TagListObject *listObject = NULL;

    temp = UWSORTINDEX(swtmr->idxRollNum);
    if (temp <= g_tmrSortLink.cursor) {
        temp = temp + OS_SWTMR_SORTLINK_LEN;
    }
    temp -= g_tmrSortLink.cursor;

    listObject = g_tmrSortLink.sortLink + (uintptr_t)UWSORTINDEX(swtmr->idxRollNum);

    timer = swtmr;

    while (timer != (struct TagSwTmrCtrl *)listObject) {
        retValue += UWROLLNUM(timer->idxRollNum);
        timer = timer->prev;
    }
    retValue = retValue * OS_SWTMR_SORTLINK_LEN;
    retValue = retValue + temp;

    return retValue;
}

/*
 * 描述：获取软件定时器剩余Tick数
 */
OS_SEC_L4_TEXT U32 OsSwTmrGetRemainTick(struct TagSwTmrCtrl *swtmr)
{
    U32 temp;
    uintptr_t intSave;

    intSave = OsIntLock();
    switch (swtmr->state & OS_SWTMR_STATUS_MASK) {
        case OS_TIMER_CREATED:
            OsIntRestore(intSave);
            return (swtmr->idxRollNum);
        case OS_TIMER_EXPIRED:
            OsIntRestore(intSave);
            return 0;
        default:
            break;
    }

    temp = OsSwTmrGetRemain(swtmr);

    OsIntRestore(intSave);
    return temp;
}

/*
 * 描述：软件定时器的暂停内部接口,定时器由Ticking状态迁移到Created状态
 */
OS_SEC_L2_TEXT void OsSwTmrStop(struct TagSwTmrCtrl *swtmr, bool reckonOff)
{
    U32 idx;
    struct TagListObject *listObject = NULL;
    uintptr_t intSave;

    intSave = OsIntLock();
    /* if the stoping node have next node ,set its rollnum to the next node's */
    idx = UWSORTINDEX(swtmr->idxRollNum);
    listObject = (g_tmrSortLink.sortLink + idx);
    if (swtmr->next != (struct TagSwTmrCtrl *)listObject) {
        UWROLLNUMADD(swtmr->next->idxRollNum, swtmr->idxRollNum);
    }

    if (!reckonOff) {
        /* 调用获取定时器剩余Tick数内部接口 */
        swtmr->idxRollNum = OsSwTmrGetRemainTick(swtmr);
    }

    swtmr->next->prev = swtmr->prev;
    swtmr->prev->next = swtmr->next;
    /* 定时器被暂停，修改定时器状态 */
    swtmr->state = (U8)OS_TIMER_CREATED;
    swtmr->next = NULL;
    swtmr->prev = NULL;

    OsIntRestore(intSave);
}

/*
 * 描述：软件定时器的删除内部接口
 */
OS_SEC_L2_TEXT void OsSwTmrDelete(struct TagSwTmrCtrl *swtmr)
{
    /*
     * 插入到空闲链表中
     */
    swtmr->next = g_tmrFreeList;
    g_tmrFreeList = swtmr;
    swtmr->state = (U8)OS_TIMER_FREE;
}

/*
 * 描述：软件定时器的启动接口
 */
OS_SEC_L2_TEXT U32 OsSwTmrStartTimer(TimerHandle tmrHandle)
{
    struct TagSwTmrCtrl *swtmr = NULL;
    uintptr_t intSave;

    if (OS_TIMER_GET_INDEX(tmrHandle) >= g_swTmrMaxNum) {
        return OS_ERRNO_TIMER_HANDLE_INVALID;
    }

    // 转化为软件定时器内部ID号
    swtmr = g_swtmrCbArray + OS_SWTMR_ID_2_INDEX(tmrHandle);
    intSave = OsSwtmrIqrSplLock(swtmr);

    switch (swtmr->state & OS_SWTMR_STATUS_MASK) {
        case OS_TIMER_FREE:
            OsSwtmrIqrSplUnlock(swtmr, intSave);
            return OS_ERRNO_SWTMR_NOT_CREATED;
        case OS_TIMER_EXPIRED:
            swtmr->state = OS_SWTMR_PRE_RUNNING | (U8)OS_TIMER_EXPIRED;
            OsSwtmrIqrSplUnlock(swtmr, intSave);
            return OS_OK;
        case OS_TIMER_RUNNING:
            OsSwtmrIqrSplUnlock(swtmr, intSave);
            return OS_OK;
        default:
            break;
    }

    if (swtmr->idxRollNum == 0) {
        // ,定时器超时处理后或者在超时处理函数中停止定时器，剩余时间置为0，启动定时器时再赋值swtmr->interval
        swtmr->idxRollNum = swtmr->interval;
    }

    OsSwTmrStart(swtmr, swtmr->idxRollNum);

    OsSwtmrIqrSplUnlock(swtmr, intSave);

    return OS_OK;
}

/*
 * 描述：软件定时器的暂停接口
 */
OS_SEC_L2_TEXT U32 OsSwTmrStopTimer(TimerHandle tmrHandle)
{
    struct TagSwTmrCtrl *swtmr = NULL;
    uintptr_t intSave;

    if (OS_TIMER_GET_INDEX(tmrHandle) >= g_swTmrMaxNum) {
        return OS_ERRNO_TIMER_HANDLE_INVALID;
    }

    swtmr = g_swtmrCbArray + OS_SWTMR_ID_2_INDEX(tmrHandle);
    intSave = OsSwtmrIqrSplLock(swtmr);

    switch (swtmr->state & OS_SWTMR_STATUS_MASK) {
        case OS_TIMER_FREE:
            OsSwtmrIqrSplUnlock(swtmr, intSave);
            return OS_ERRNO_SWTMR_NOT_CREATED;
        case OS_TIMER_EXPIRED:
            swtmr->state = OS_SWTMR_PRE_CREATED | (U8)OS_TIMER_EXPIRED;
            OsSwtmrIqrSplUnlock(swtmr, intSave);
            return OS_OK;
        case OS_TIMER_CREATED:
            OsSwtmrIqrSplUnlock(swtmr, intSave);
            return OS_ERRNO_SWTMR_UNSTART;
        default:
            break;
    }
    /*
     * 调用暂停定时器内部接口，在暂停时计算剩余时间
     */
    OsSwTmrStop(swtmr, FALSE);

    OsSwtmrIqrSplUnlock(swtmr, intSave);
    return OS_OK;
}

OS_SEC_L2_TEXT U32 OsSwTmrRestartTimer(TimerHandle tmrHandle)
{
    struct TagSwTmrCtrl *swtmr = NULL;
    uintptr_t intSave;

    if (OS_TIMER_GET_INDEX(tmrHandle) >= g_swTmrMaxNum) {
        return OS_ERRNO_TIMER_HANDLE_INVALID;
    }

    // 转化为软件定时器内部ID?
    swtmr = g_swtmrCbArray + OS_SWTMR_ID_2_INDEX(tmrHandle);
    intSave = OsSwtmrIqrSplLock(swtmr);

    switch (swtmr->state & OS_SWTMR_STATUS_MASK) {
        case OS_TIMER_FREE:
            OsSwtmrIqrSplUnlock(swtmr, intSave);
            return OS_ERRNO_SWTMR_NOT_CREATED;
        case OS_TIMER_EXPIRED:
            swtmr->state = OS_SWTMR_PRE_RUNNING | (U8)OS_TIMER_EXPIRED;
            OsSwtmrIqrSplUnlock(swtmr, intSave);
            return OS_OK;
        case OS_TIMER_RUNNING:
            OsSwTmrStop(swtmr, TRUE);
            break;
        default:
            break;
    }

    OsSwTmrStart(swtmr, swtmr->interval);

    OsSwtmrIqrSplUnlock(swtmr, intSave);

    return OS_OK;
}

/*
 * 描述：查询软件定时器剩余超时时间
 */
OS_SEC_L2_TEXT U32 OsSwTmrQuery(TimerHandle tmrHandle, U32 *expireTime)
{
    struct TagSwTmrCtrl *swtmr = NULL;
    uintptr_t intSave;
    U32 remainTick;
    U32 remainMs;

    if (OS_TIMER_GET_INDEX(tmrHandle) >= g_swTmrMaxNum) {
        return OS_ERRNO_TIMER_HANDLE_INVALID;
    }

    // 转化为软件定时器内部ID
    swtmr = g_swtmrCbArray + OS_SWTMR_ID_2_INDEX(tmrHandle);
    intSave = OsSwtmrIqrSplLock(swtmr);

    if (swtmr->state == (U8)OS_TIMER_FREE) {
        OsSwtmrIqrSplUnlock(swtmr, intSave);
        return OS_ERRNO_SWTMR_NOT_CREATED;
    }

    /* 调用获取定时器剩余Tick数内部接口 */
    remainTick = OsSwTmrGetRemainTick(swtmr);
    remainMs = (U32)DIV64(((U64)remainTick * OS_SYS_MS_PER_SECOND), g_tickModInfo.tickPerSecond);
    if (DIV64_REMAIN((U64)remainTick * OS_SYS_MS_PER_SECOND, g_tickModInfo.tickPerSecond) != 0) {  // 若不整除，则+1
        remainMs++;
    }

    *expireTime = remainMs;

    OsSwtmrIqrSplUnlock(swtmr, intSave);
    return OS_OK;
}

/*
 * 描述：软件定时器的获取超时次数接口
 */
OS_SEC_L2_TEXT U32 OsSwTmrGetOverrun(TimerHandle tmrHandle, U32 *overrun)
{
    struct TagSwTmrCtrl *swtmr = NULL;
    uintptr_t intSave;

    if (OS_TIMER_GET_INDEX(tmrHandle) >= g_swTmrMaxNum) {
        return OS_ERRNO_TIMER_HANDLE_INVALID;
    }

    // 转化为软件定时器内部ID号
    swtmr = g_swtmrCbArray + OS_SWTMR_ID_2_INDEX(tmrHandle);
    intSave = OsSwtmrIqrSplLock(swtmr);

    *overrun = (U32)swtmr->overrun;

    OsSwtmrIqrSplUnlock(swtmr, intSave);

    return OS_OK;
}
