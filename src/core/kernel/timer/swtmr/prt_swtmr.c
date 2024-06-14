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

#if defined(OS_OPTION_TICKLESS)
#if defined(OS_OPTION_SMP)
/*
 * 描述：刷新最近超时tick数
 */
OS_SEC_TEXT void OsSwtmrNearestTicksRefresh(struct TagSwTmrSortLinkAttr *tmrSort)
{
    U64 ticks = OS_TICKLESS_FOREVER;
    struct TagListObject *listObject = &tmrSort->sortLink;

    if (!ListEmpty(listObject)) {
        struct TagSwTmrCtrl *swtmr = (struct TagSwTmrCtrl *)(uintptr_t)listObject->next;
        ticks = swtmr->expectedTick;
    }
    tmrSort->nearestTicks = ticks;
}
#else
OS_SEC_TEXT void OsSwtmrNearestTicksRefresh(struct TagSwTmrSortLinkAttr *tmrSort)
{
    (void)tmrSort;
}
#endif
#endif

OS_SEC_ALW_INLINE INLINE void OsSwtmrProc(struct TagSwTmrCtrl *swtmr)
{
    switch (swtmr->state & OS_SWTMR_PRE_STATUS_MASK) {
        /* 为了超时状态未改变的情况下执行效率，先对定时器的此种状态进行处理 */
        case OS_SWTMR_STATUS_DEFAULT:
            swtmr->overrun = 0;
            if (swtmr->mode == (U8)OS_TIMER_LOOP) {
                OsSwTmrStart(swtmr, swtmr->interval);
            } else {
                swtmr->idxRollNum = 0;
                swtmr->state = (U8)OS_TIMER_CREATED;
            }
            break;
        case OS_SWTMR_PRE_RUNNING:
            OsSwTmrStart(swtmr, swtmr->interval);
            break;
        case OS_SWTMR_PRE_CREATED:
            // ,定时器超时处理后，剩余时间置为0，启动定时器时再赋值swtmr->interval
            swtmr->idxRollNum = 0;
            swtmr->overrun = 0;
            swtmr->state = (U8)OS_TIMER_CREATED;
            break;
        case OS_SWTMR_PRE_FREE:
            OsSwTmrDelete(swtmr);
            break;
        default:
            break;
    }
    return;
}

/*
 * 描述：调用本函数需要先锁上timer, 调用osIntLock
 */
OS_SEC_ALW_INLINE INLINE void OsSwTmrScanProcess(struct TagListObject *object, struct TagListObject *listObject,
                                                 struct TagSwTmrSortLinkAttr *tmrSort, struct TagSwTmrCtrl *outLink)
{
    /*
     * 处理3种情况
     * [1] 如果第一个节点的rollNum就大于0，则outLink=NULL。
     * [2] 如果是从中间跳出，则第一个rollNum非0节点为swtmr，前面的定时器为超时定时器。
     * 最后一个超时定时器的next置为NULL，listObject->next指向swtmr。
     * swtmr->prev指向listObject。
     * [3] 如果全部节点都超时，则是从while条件语句跳出。swtmr为listObject。
     * 最后一个定时器的next置为NULL，listObject->next指向swtmr即本身。
     * swtmr->prev指向listObject即本身。
     */
    struct TagSwTmrCtrl *temp = NULL;
    struct TagSwTmrCtrl *swtmr = (struct TagSwTmrCtrl *)object->next;

#if !defined(OS_OPTION_SMP)
    (void)tmrSort;
#endif
    object->next = NULL;
    /* 更新SortLink成员next指针 */
    listObject->next = (struct TagListObject *)swtmr;
    /* 第一个rollNum非0节点的prev指针指向SortLink成员 */
    swtmr->prev = (struct TagSwTmrCtrl *)listObject;
#if defined(OS_OPTION_TICKLESS)
    OsSwtmrNearestTicksRefresh(tmrSort);
#endif
#if defined(OS_OPTION_SMP)
    OsSplUnlock(&tmrSort->spinLock);
#endif
    (void)OsIntUnLock();

    /* 处理超时链表中的定时器 */
    swtmr = outLink;
    while ((swtmr != NULL) && (swtmr != (struct TagSwTmrCtrl *)listObject)) {
        temp = swtmr->next;
        swtmr->handler(OS_SWTMR_INDEX_2_ID(swtmr->swtmrIndex), swtmr->arg1, swtmr->arg2, swtmr->arg3, swtmr->arg4);

        (void)OsIntLock();
#if defined(OS_OPTION_SMP)
        OsSplLock(&tmrSort->spinLock);
#endif
        swtmr->prev = NULL;
        swtmr->next = NULL;
        OsSwtmrProc(swtmr);
#if defined(OS_OPTION_TICKLESS)
        OsSwtmrNearestTicksRefresh(tmrSort);
#endif
#if defined(OS_OPTION_SMP)
        OsSplUnlock(&tmrSort->spinLock);
#endif
        (void)OsIntUnLock();

        swtmr = temp;
    }
}
#if defined(OS_OPTION_SMP)
OS_SEC_TEXT void OsSwTmrAttrScan(struct TagSwTmrSortLinkAttr *tmrSort)
{
    struct TagSwTmrCtrl *swtmr = NULL;
    struct TagSwTmrCtrl *outLink = NULL;
    struct TagListObject *listObject = NULL;
    struct TagListObject *object = NULL;
    uintptr_t intSave;
    /* 对本核的超时链进行扫描 */
    intSave = OsIntLock();
    OsSplLock(&tmrSort->spinLock);

    /* 从计时链表中获取超时链表 */
    listObject = &tmrSort->sortLink;
    if (listObject->next == listObject) {  /* 没有计时定时器 */
        OsSplUnlock(&tmrSort->spinLock);
        OsIntRestore(intSave);
        return;
    }

    outLink = (struct TagSwTmrCtrl*)(uintptr_t)listObject->next;
    object = listObject;
    while (object->next !=listObject) {
        swtmr = (struct TagSwTmrCtrl*)(uintptr_t)object->next;
        if(swtmr->expectedTick > g_uniTicks) {
            if(swtmr->prev == (struct TagSwTmrCtrl*)(uintptr_t)listObject){
                /* 如果第一个定时器rollNum > 0, 则表示没有超时链表为空*/
                outLink = NULL;
            }
            break;
        }
        swtmr->state = (U8)OS_TIMER_EXPIRED;
        object = object->next;
    }

    OsSwTmrScanProcess(object, listObject, tmrSort, outLink);
    
}
/*
 * 描述：软件定时器模块的Tick中断运行接口
 */
OS_SEC_TEXT void OsSwTmrScan(void)
{
    U32 thisCoreID = THIS_CORE();
    struct TagSwTmrSortLinkAttr *tmrSort = NULL;

    tmrSort = CPU_SWTMR_SORT_LINK(thisCoreID);

    // 处理本核的软定时链
    OsSwTmrAttrScan(tmrSort);

    return;
}
#else
/*
 * 描述：软件定时器模块的Tick中断运行接口
 */
OS_SEC_TEXT void OsSwTmrScan(void)
{
    struct TagSwTmrCtrl *swtmr = NULL;
    struct TagSwTmrCtrl *outLink = NULL;
    struct TagListObject *listObject = NULL;
    struct TagListObject *object = NULL;
    uintptr_t intSave;

    /*
     * 扫描计时链表，把超时定时器从计时链表中删除，状态置为OUT状态
     * 超时的定时器形成超时链表(从首节点到第一个rollNum非0节点)。
     */
    intSave = OsIntLock();

    g_tmrSortLink.cursor = (g_tmrSortLink.cursor + 1) % OS_SWTMR_SORTLINK_LEN;
    /*
     * 从计时链表中获取超时链表
     */
    listObject = g_tmrSortLink.sortLink + g_tmrSortLink.cursor;
    if (listObject->next == listObject) { /* 没有计时定时器 */
        OsIntRestore(intSave);
        return;
    }

    outLink = (struct TagSwTmrCtrl *)listObject->next;
    object = listObject;
    while (object->next != listObject) {
        swtmr = (struct TagSwTmrCtrl *)object->next;
        if (UWROLLNUM(swtmr->idxRollNum) > 0) {
            UWROLLNUMDEC(swtmr->idxRollNum);
            if (swtmr->prev == (struct TagSwTmrCtrl *)listObject) {
                /*
                 * 如果第一个定时器rollNum>0，则表示没有超时链表为空
                 */
                outLink = NULL;
            }
            break;
        } else {
            if (swtmr->overrun < (U8)0xFF) {
                swtmr->overrun++;
            }
        }
        swtmr->state = (U8)OS_TIMER_EXPIRED;
        object = object->next;
    }

    OsSwTmrScanProcess(object, listObject, NULL, outLink);

    return;
}
#endif
#if defined(OS_OPTION_SMP)
OS_SEC_ALW_INLINE INLINE struct TagListObject *OsSwTmrStartInner(struct TagSwTmrCtrl *swtmr, U32 interval)
{
    struct TagSwTmrSortLinkAttr *tmrSort = NULL;
    (void)interval;

    tmrSort = CPU_SWTMR_SORT_LINK(swtmr->coreID);
    swtmr->state = (U8)OS_TIMER_RUNNING;

    return &tmrSort->sortLink;
}
#else
OS_SEC_ALW_INLINE INLINE struct TagListObject *OsSwTmrStartInner(struct TagSwTmrCtrl *swtmr, U32 interval)
{
    U32 sortIndex;
    U32 rollNum;

    sortIndex = interval & OS_SWTMR_SORTLINK_MASK;
    rollNum = (interval / OS_SWTMR_SORTLINK_LEN);
    (sortIndex > 0) ? 0 : (rollNum--);
    EVALUATE_L(swtmr->idxRollNum, rollNum);
    sortIndex = (sortIndex + g_tmrSortLink.cursor);
    /* sortIndex % 64; */
    sortIndex = sortIndex & OS_SWTMR_SORTLINK_MASK;
    EVALUATE_H(swtmr->idxRollNum, sortIndex);

    swtmr->overrun = 0;
    swtmr->state = (U8)OS_TIMER_RUNNING;

    return g_tmrSortLink.sortLink + sortIndex;
}
#endif
/*
 * 描述：软件定时器的启动接口
 */
OS_SEC_TEXT void OsSwTmrStart(struct TagSwTmrCtrl *swtmr, U32 interval)
{
    struct TagSwTmrCtrl *temp = NULL;
    struct TagListObject *listObject = NULL;

#if (defined(OS_OPTION_TICKLESS) || defined(OS_OPTION_SMP))
    /* 确认是否能直接进行64位操作 */
    swtmr->expectedTick = (U64) interval + g_uniTicks;
#endif

    listObject = OsSwTmrStartInner(swtmr, interval);
    if (listObject->next == listObject) { // 该SortLink成员上是空链
        swtmr->next = (struct TagSwTmrCtrl *)listObject;
        swtmr->prev = (struct TagSwTmrCtrl *)listObject;
        listObject->next = (struct TagListObject *)swtmr;
        listObject->prev = (struct TagListObject *)swtmr;
    } else {
        /*
         * 从链表头开始查找，从小往大找
         * 每个链表节点的成员idxRollNum 都是相对于前一个
         * 节点的到期时间增量，这样就每次到期拿掉一个节点后，
         * idxRollNum还都是相对时间，不用更新
         */
        /* The First Node */
        temp = (struct TagSwTmrCtrl *)listObject->next;
        while (temp != (struct TagSwTmrCtrl *)listObject) {
#if defined(OS_OPTION_SMP)
            if(temp->expectedTick > swtmr->expectedTick) {
                break;
            }
#else
            if (UWROLLNUM(temp->idxRollNum) > UWROLLNUM(swtmr->idxRollNum)) {
                UWROLLNUMSUB(temp->idxRollNum, swtmr->idxRollNum);
                break;
            }
            UWROLLNUMSUB(swtmr->idxRollNum, temp->idxRollNum);
#endif
            temp = temp->next;
        }
        swtmr->next = temp;
        swtmr->prev = temp->prev;
        temp->prev->next = swtmr;
        temp->prev = swtmr;
    }
}
