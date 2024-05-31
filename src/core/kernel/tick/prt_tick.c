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
 * Description: Tick interrupt implementation
 */
#include "prt_tick_internal.h"
#if defined(OS_OPTION_LINUX)
#include <linux/jiffies.h>
#endif

/* 任务检测Tick中断调用钩子 */
OS_SEC_BSS TskmonTickHook g_tskMonHook;
/* 软件定时器扫描钩子 */
OS_SEC_BSS SwitchScanFunc g_swtmrScanHook;
OS_SEC_BSS TickHandleFunc g_tickUsrHook;

#if defined(OS_OPTION_LINUX)
unsigned long volatile jiffies;
#endif
#if defined(OS_OPTION_SMP)
extern U32 g_slaveTickEnable;
#endif
#define TICK_SIGNED_MAX_VALUE 0x7fffffff

#if defined(OS_OPTION_TICKLESS)
#define OS_CORE_INV_ID 0xFFFFFFFF
#define NEAREST_TICK_NUM 4
/* 获取所有任务延时最近的tick刻度 */
OS_SEC_BSS GetNearestTickFunc g_getTskDlyNearestTick;

/* 获取软件定时器最近的tick刻度 */
OS_SEC_BSS GetNearestTickFunc g_getSwtmrNearestTick;
OS_SEC_BSS GetNearestTickFunc g_getCpupNearestTick;

/* 检查是否有需要处理的任务延时 */
OS_SEC_BSS CheckTickProcessFunc g_checkTskDlyTickProcess;

/* 检查是否有需要处理的软件定时 */
OS_SEC_BSS CheckTickProcessFunc g_checkSwtmrTickProcess;
OS_SEC_BSS CheckTickProcessFunc g_checkCpupTickProcess;
#endif
/*
 * 描述: 调整Tick计数值。
 */
OS_SEC_L4_TEXT U32 PRT_TickCountAdjust(S32 tick)
{
    S64 offsetTmp;
    uintptr_t intSave;
    U32 tickCountHi;
    U32 tickCountLo;
    U32 tickCountHiTmp;
    U32 tickAdjustHi;
    U32 tickAdjustLo;
    U64 adjustTmp;

    intSave = OsIntLock();
    offsetTmp = g_ticksOffset;
    offsetTmp += tick;

    adjustTmp = (U64)offsetTmp;

    tickCountHi = OS_GET_64BIT_HIGH_32BIT(g_uniTicks);
    tickCountLo = OS_GET_64BIT_LOW_32BIT(g_uniTicks);
    tickAdjustHi = OS_GET_64BIT_HIGH_32BIT(adjustTmp);
    tickAdjustLo = OS_GET_64BIT_LOW_32BIT(adjustTmp);

    tickCountHiTmp = tickCountHi;
    OsAdd64(&tickCountLo, &tickCountHi, tickAdjustLo, tickAdjustHi);

    /* Tick补偿总大小为负数，且它的绝对值大于Tick计数值 */
    if ((tickAdjustHi > TICK_SIGNED_MAX_VALUE) && (tickCountHiTmp <= TICK_SIGNED_MAX_VALUE) &&
        (tickCountHi > TICK_SIGNED_MAX_VALUE)) {
        OsIntRestore(intSave);
        return OS_ERRNO_TICK_ADJUST_VALUE_INVALID;
    }

    g_ticksOffset = offsetTmp;
    OsIntRestore(intSave);

    return OS_OK;
}

/*
 * 描述：Tick中断的处理函数。扫描任务超时链表、扫描超时软件定时器、扫描TSKMON等。
 */
OS_SEC_TEXT void OsTickDispatcher(void)
{
    uintptr_t intSave;

    intSave = OsIntLock();

#if !defined(OS_OPTION_SMP)
    g_uniTicks++;
#endif

#if defined(OS_OPTION_SMP) && !defined(OS_OPTION_TICKLESS)
    if (THIS_CORE() == g_cfgPrimaryCore) {
        g_uniTicks++;
    }
#endif

#if defined(OS_OPTION_LINUX)
    jiffies = g_uniTicks;
#endif

    OS_TICK_COUNT_UPDATE();

    // 任务超时扫描
    OS_TASK_SCAN();

    // tick模式下的cpu占用率采样
    OS_TICK_TASK_ENTRY();

    OsIntRestore(intSave);

    TICK_USER_HOOK_RUN();

    OS_SWTMR_SCAN();
    TSKMON_TICK_RUN();
}

#if defined(OS_OPTION_TICKLESS)
/*
* 描述：获取指定核的下次tick触发点
*/
OS_SEC_L2_TEXT U64 OsCoreNearestTickGet(U32 coreID)
{
    U64 aullNearestTicks[NEAREST_TICK_NUM] = {
        OS_TICKLESS_FOREVER,
    };
    U64 minTicks = OS_TICKLESS_FOREVER;
    U32 num = 0;

    if(g_getTskDlyNearestTick != NULL) {
        // 任务延时
        aullNearestTicks[num++] = g_getTskDlyNearestTick(coreID);
    }

    if(g_getSwtmrNearestTick != NULL) {
        // 软件定时器
        aullNearestTicks[num++] = g_getSwtmrNearestTick(coreID);
    }

    if (coreID == 0) {
        if (g_getCpupNearestTick != NULL) {
            // CPUP
            aullNearestTicks[num++] = g_getSwtmrNearestTick(0);
        }
    }

    for (U32 idx = 0; idx < num; idx++) {
        if (aullNearestTicks[idx] != OS_TICKLESS_FOREVER) {
            if ((minTicks == OS_TICKLESS_FOREVER) || (aullNearestTicks[idx] < minTicks)) {
                minTicks = aullNearestTicks[idx];
            }
        }
    }
    return minTicks;
}

/*
* 描述：获取系统最短tickless计数及对应的coreId
* 备注：调用者确保不会换核（调用前关中断等方式）
*/
OS_SEC_L2_TEXT U32 PRT_TickLessCountGet(U32 *minTicks, U32 *coreId)
{
    U32 tmpCoreID;
    U64 ticks = OS_TICKLESS_FOREVER;
    U64 tmpTicks;
    U32 thisCoreID = THIS_CORE();
    U32 bindCoreID = OS_CORE_INV_ID;

    if ((minTicks == NULL) || (coreId == NULL)) {
        return OS_FAIL;
    }

    for (tmpCoreID = 0; tmpCoreID < g_maxNumOfCores; tmpCoreID++) {
        tmpTicks = OsCoreNearestTickGet(tmpCoreID);
        if(tmpTicks == OS_TICKLESS_FOREVER) {
            // FOREVER表示这个核没有需要相应的tick
            continue;
        }
        if (bindCoreID == OS_CORE_INV_ID) {
            ticks = tmpTicks;
            bindCoreID = tmpCoreID;
        } else if (ticks > tmpTicks) {
            ticks = tmpTicks;
            bindCoreID = tmpCoreID;
        } else if ((ticks == tmpTicks) && (tmpCoreID == thisCoreID)) {
            // 若本核也是最短浅睡的核，本核原就是tick中断唯一的目标核，无需绑定
            bindCoreID = thisCoreID;
        }
    }

    *coreId = bindCoreID;

    if (ticks == OS_TICKLESS_FOREVER) {
        *minTicks = OS_MAX_U32;
        return OS_FAIL;
    }
    if (ticks <= g_uniTicks) {
        *minTicks = 0;
        return OS_FAIL;
    }

    *minTicks = (U32)(ticks - g_uniTicks);
    return OS_OK;
}
/*
 * 描述: 更新tick计数值
 */
OS_SEC_L2_TEXT U32 PRT_TickCountUpdate(enum SleepMode sleepMode, S32 ticks)
{
    if (sleepMode == OS_TYPE_LIGHT_SLEEP) {
        uintptr_t intSave;
        U64 currTicks;
        U64 deltaTicks;

        intSave = OsIntLock();
        currTicks = g_uniTicks;

        if (ticks < 0) {
            deltaTicks = (U64)((S64)(-ticks));
            if (currTicks < deltaTicks) {
                OsIntRestore(intSave);
                return OS_ERRNO_TICK_ADJUST_VALUE_INVALID;
            } else {
                currTicks -= deltaTicks;
            }
        } else {
            deltaTicks = (U64)((S64)ticks);
            currTicks += deltaTicks;
        }

        g_uniTicks = currTicks;
        OsIntRestore(intSave);

        return OS_OK;
    } else if (sleepMode == OS_TYPE_DEEP_SLEEP) {
        return PRT_TickCountAdjust(ticks);
    }

    return OS_ERRNO_TICK_ADJUST_MODE_INVALID;
}

/*
* 描述： 查看目标核是否需要有tick事务处理
*/
OS_SEC_TEXT bool OsTickCoreCheck(U32 coreID)
{
    bool ret = FALSE;
    if (g_checkTskDlyTickProcess != NULL) {
        // 任务延时
        ret = (ret || g_checkTskDlyTickProcess(coreID));
    }

    if(g_checkSwtmrTickProcess != NULL) {
        // 软件定时器
        ret = (ret || g_checkSwtmrTickProcess(coreID));
    }

    if(coreID == 0) {
        if(g_checkCpupTickProcess != NULL) {
            // CPUP
            ret = (ret || g_checkSwtmrTickProcess(CPUP_TSKMON_CORE_ID));
        }
    }

    return ret;
}
#endif

#if defined (OS_OPTION_SMP) && defined(OS_OPTION_TICKLESS)


/*
* 描述：获取需响应此次tick的核掩码
*/
OS_SEC_TEXT U32 OsTickTargetCoresGet(void)
{
    U32 coreID;
    U32 coreMask = 0;

    for(coreID = g_cfgPrimaryCore; coreID < g_maxNumOfCores; coreID++) {
        if (OsTickCoreCheck(coreID)) {
            coreMask |= (1U << coreID);
        }
    }

    return coreMask;
}

/*
* 描述：smp下，Tick定时器中断处理函数
*/
OS_SEC_TEXT void PRT_TickISR(void)
{
    U32 coreMask;
    U32 thisCoreMask = (U32)(1UL << THIS_CORE());

    g_uniTicks++;

    coreMask = OsTickTargetCoresGet();
    
    if ((coreMask & thisCoreMask) != 0) {
        coreMask &= ~thisCoreMask;

        OsTickTailProcActivate();
    }
    
    if (coreMask != 0 && g_slaveTickEnable){
        OsTickForward(coreMask);
    }
}
#else
void PRT_TickISR(void)
{
#if !defined(OS_OPTION_TICK_USE_HWTMR)
    if (OsSysGetTickPerSecond() != 0) {
        TICK_NO_RESPOND_CNT++;
    }
#endif
}
#endif