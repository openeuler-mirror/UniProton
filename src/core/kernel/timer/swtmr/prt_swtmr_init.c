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

OS_SEC_BSS U32 g_swTmrMaxNum;
/* 定时器内存空间首地址 */
OS_SEC_BSS struct TagSwTmrCtrl *g_swtmrCbArray;
/* 软件定时器空闲链表 */
OS_SEC_BSS struct TagSwTmrCtrl *g_tmrFreeList;
/* 软件定时器Sortlink */
OS_SEC_BSS struct TagSwTmrSortLinkAttr g_tmrSortLink;

/* 基于tick的软件定时器组ID，为支持独立升级，该变量需要在2022年之后版本才能删除 */
OS_SEC_BSS TimerGroupId g_tickSwTmrGroupId;
/*
 * 描述：创建普通软件定时器组
 */
OS_SEC_L4_TEXT U32 OsSwTimerGroupCreate(struct TmrGrpUserCfg *config, TimerGroupId *groupId)
{
    U32 ret;
    uintptr_t intSave;

    if (g_tickModInfo.tickPerSecond == 0) {
        return OS_ERRNO_TICK_NOT_INIT;
    }

    if (config->maxTimerNum == 0) {  // 最大定时器个数为零
        return OS_ERRNO_TIMER_NUM_ZERO;
    }

    if (config->maxTimerNum > (U32)OS_MAX_U16) {
        // 软件定时器个数为UINT16型，用户传入参数不能超过UINT16的最大值
        return OS_ERRNO_TIMER_NUM_TOO_LARGE;
    }

    intSave = OsIntLock();

    if (g_timerApi[TIMER_TYPE_SWTMR].createTimer != NULL) {  // 定时器组已经创建
        OsIntRestore(intSave);
        return OS_ERRNO_TIMER_TICKGROUP_CREATED;
    }

    g_swTmrMaxNum = config->maxTimerNum;

    ret = OsSwTmrResInit();
    if (ret != OS_OK) {
        OsIntRestore(intSave);
        return ret;
    }

    g_timerApi[TIMER_TYPE_SWTMR].createTimer = (TimerCreateFunc)OsSwTmrCreateTimer;
    g_timerApi[TIMER_TYPE_SWTMR].startTimer = (TimerStartFunc)OsSwTmrStartTimer;
    g_timerApi[TIMER_TYPE_SWTMR].stopTimer = (TimerStopFunc)OsSwTmrStopTimer;
    g_timerApi[TIMER_TYPE_SWTMR].deleteTimer = (TimerDeleteFunc)OsSwTmrDeleteTimer;
    g_timerApi[TIMER_TYPE_SWTMR].restartTimer = (TimerRestartFunc)OsSwTmrRestartTimer;
    g_timerApi[TIMER_TYPE_SWTMR].timerQuery = (TimerQueryFunc)OsSwTmrQuery;

    *groupId = OS_TICK_SWTMR_GROUP_ID;

    OsIntRestore(intSave);
    return OS_OK;
}

OS_SEC_ALW_INLINE INLINE U32 OsSwTmrCreateTimerParaChk(struct TimerCreatePara *createPara)
{
    /* OS接管中断, 中断处理函数必须为非NULL */
    if (createPara->callBackFunc == NULL) {
        return OS_ERRNO_TIMER_PROC_FUNC_NULL;
    }

    if (createPara->timerGroupId != OS_TICK_SWTMR_GROUP_ID) {
        return OS_ERRNO_TIMERGROUP_ID_INVALID;
    }

    if ((DIV64_REMAIN((U64)g_tickModInfo.tickPerSecond * (createPara->interval), OS_SYS_MS_PER_SECOND)) !=
        0) {  // ms转tick时，tick不为整数
        return OS_ERRNO_SWTMR_INTERVAL_NOT_SUITED;
    }

    if (createPara->interval >
        DIV64(OS_SYS_MS_PER_SECOND * ((U64)OS_MAX_U32), g_tickModInfo.tickPerSecond)) {  // 溢出判断
        return OS_ERRNO_SWTMR_INTERVAL_OVERFLOW;
    }
    return OS_OK;
}

OS_SEC_ALW_INLINE INLINE void OsSwTmrCreateTimerCbInit(struct TimerCreatePara *createPara,
                                                       struct TagSwTmrCtrl *swtmr, U32 interval)
{
    /* 设置新定时器的参数 */
    swtmr->handler = createPara->callBackFunc;
    swtmr->mode = (U8)createPara->mode;
    swtmr->interval = interval;
    swtmr->next = NULL;
    swtmr->prev = NULL;
    swtmr->idxRollNum = swtmr->interval;
    swtmr->arg1 = createPara->arg1;
    swtmr->arg2 = createPara->arg2;
    swtmr->arg3 = createPara->arg3;
    swtmr->arg4 = createPara->arg4;
    swtmr->state = (U8)OS_TIMER_CREATED;
    return;
}
OS_SEC_L4_TEXT U32 OsSwTmrInit(U32 maxTimerNum)
{
    struct TmrGrpUserCfg config;

    config.tmrGrpSrcType = OS_TIMER_GRP_SRC_TICK;
    config.maxTimerNum = maxTimerNum;

    return OsSwTimerGroupCreate(&config, &g_tickSwTmrGroupId);
}
/*
 * 描述：软件定时器的创建接口
 */
OS_SEC_L4_TEXT U32 OsSwTmrCreateTimer(struct TimerCreatePara *createPara, TimerHandle *tmrHandle)
{
    struct TagSwTmrCtrl *swtmr = NULL;
    U32 interval;
    uintptr_t intSave;
    U32 ret;

    ret = OsSwTmrCreateTimerParaChk(createPara);
    if (ret != OS_OK) {
        return ret;
    }

    interval = (U32)DIV64(((U64)g_tickModInfo.tickPerSecond * (createPara->interval)), OS_SYS_MS_PER_SECOND);

    intSave = OsIntLock();

    if (g_tmrFreeList == NULL) {
        OsIntRestore(intSave);
        return OS_ERRNO_SWTMR_MAXSIZE;
    }

    /*
     * 从空闲链表中取出一个控制块
     * 保证空闲链表的完整性，首节点没有前驱，尾节点没有后继节点
     */
    swtmr = g_tmrFreeList;
    g_tmrFreeList = swtmr->next;
    OsSwTmrCreateTimerCbInit(createPara, swtmr, interval);

    SWTMR_CREATE_DEL_UNLOCK();
    OsIntRestore(intSave);
    /* 把TimerID写入返回参数 */
    *tmrHandle = OS_SWTMR_INDEX_2_ID(swtmr->swtmrIndex);

    return OS_OK;
}

/*
 * 描述：软件定时器的删除接口
 */
OS_SEC_L4_TEXT U32 OsSwTmrDeleteTimer(TimerHandle tmrHandle)
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
            swtmr->state = OS_SWTMR_PRE_FREE | (U8)OS_TIMER_EXPIRED;
            OsSwtmrIqrSplUnlock(swtmr, intSave);
            return OS_OK;
        case OS_TIMER_RUNNING:
            OsSwTmrStop(swtmr, TRUE);
            break;
        default:
            break;
    }

    OsSwTmrDelete(swtmr);

    OsSwtmrIqrSplUnlock(&swtmrTmp, intSave);

    return OS_OK;
}

OS_SEC_L4_TEXT void OsSwTmrCtrlInit(struct TagSwTmrCtrl *swtmrIn)
{
    U16 idx;
    struct TagSwTmrCtrl *temp = NULL;
    struct TagSwTmrCtrl *swtmr = swtmrIn;

    swtmr->swtmrIndex = 0;
    swtmr->state = (U8)OS_TIMER_FREE;
    temp = swtmr;
    swtmr++;
    for (idx = 1; idx < g_swTmrMaxNum; idx++, swtmr++) {
        swtmr->swtmrIndex = idx;
        swtmr->state = (U8)OS_TIMER_FREE;
        temp->next = swtmr;
        temp = swtmr;
    }

    g_swtmrScanHook = OsSwTmrScan;
}

/*
 * 描述：软件定时器模块的初始化接口
 */
OS_SEC_L4_TEXT U32 OsSwTmrResInit(void)
{
    U32 ret;
    U32 size;
    U32 idx;
    struct TagListObject *listObject = NULL;
    struct TagSwTmrCtrl *swtmr = NULL;

    /*
     * 为定时器控制块分配内存，内存首地址保存到g_swtmrCbArray
     * 控制块所需内存超过0xFFFFFFFF
     */
    if (g_swTmrMaxNum > (OS_MAX_U32 / sizeof(struct TagSwTmrCtrl))) {
        return OS_ERRNO_SWTMR_NO_MEMORY;
    }

    /* 为SortLink分配内存，初始化g_tmrSortLink */
    size = sizeof(struct TagListObject) * OS_SWTMR_SORTLINK_LEN;
    listObject =
        (struct TagListObject *)OsMemAllocAlign((U32)OS_MID_SWTMR, OS_MEM_DEFAULT_FSC_PT, size, MEM_ADDR_ALIGN_016);
    if (listObject == NULL) {
        return OS_ERRNO_SWTMR_NO_MEMORY;
    }

    if (memset_s(listObject, size, 0, size) != EOK) {
        OS_GOTO_SYS_ERROR1();
    }
    g_tmrSortLink.sortLink = listObject;
    g_tmrSortLink.cursor = 0;

    for (idx = 0; idx < OS_SWTMR_SORTLINK_LEN; idx++, listObject++) {
        listObject->next = listObject;
        listObject->prev = listObject;
    }

    size = sizeof(struct TagSwTmrCtrl) * g_swTmrMaxNum;
    swtmr = (struct TagSwTmrCtrl *)OsMemAllocAlign((U32)OS_MID_SWTMR, OS_MEM_DEFAULT_FSC_PT, size, MEM_ADDR_ALIGN_016);
    if (swtmr == NULL) {
        ret = PRT_MemFree((U32)OS_MID_SWTMR, (void *)g_tmrSortLink.sortLink);
        if (ret != OS_OK) {
            OS_REPORT_ERROR(ret);
        }

        return OS_ERRNO_SWTMR_NO_MEMORY;
    }

    if (memset_s(swtmr, size, 0, size) != EOK) {
        OS_GOTO_SYS_ERROR1();
    }
    g_swtmrCbArray = swtmr;

    /* 把所有定时器控制块组织成一条单向链表，存放到空闲链中，空闲了首地址g_tmrFreeList */
    g_tmrFreeList = swtmr;

    OsSwTmrCtrlInit(swtmr);

    return OS_OK;
}