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
 * Description: 硬件定时器模块的内部头文件
 */
#ifndef PRT_TIMER_EXTERNAL_H
#define PRT_TIMER_EXTERNAL_H

#include "prt_timer.h"

/*
 * 模块间typedef声明
 */
typedef U32(*TimerCreateFunc)(struct TimerCreatePara *createPara, TimerHandle *tmrHandle);

typedef U32(*TimerStartFunc)(TimerHandle timerHdl);

typedef U32(*TimerStopFunc)(TimerHandle timerHdl);

typedef U32(*TimerDeleteFunc)(TimerHandle timerHdl);

typedef U32(*TimerRestartFunc)(TimerHandle timerHdl);

typedef U32(*TimerSetIntervalFunc)(TimerHandle timerHdl, U32 interVal);

typedef U32(*TimerQueryFunc)(TimerHandle timerHdl, U32 *expireTime);

typedef U32(*TimerGetOverrunFunc)(TimerHandle timerHdl, U32 *overrun);

/* 定时器函数库 */
struct TagFuncsLibTimer {
    TimerCreateFunc createTimer;
    TimerStartFunc startTimer;
    TimerStopFunc stopTimer;
    TimerDeleteFunc deleteTimer;
    TimerRestartFunc restartTimer;
    TimerSetIntervalFunc setIntervalTimer;
    TimerQueryFunc timerQuery;
    TimerGetOverrunFunc getOverrun;
};

/* 定时器类型 */
enum {
    TIMER_TYPE_HWTMR, /* 核内私有硬件定时器 */
    TIMER_TYPE_SWTMR, /* 软件定时器 */
    TIMER_TYPE_INVALID
};

/*
 * 模块间全局变量声明
 */
extern struct TagFuncsLibTimer g_timerApi[TIMER_TYPE_INVALID];

#define OS_SWTMR_PRE_FREE 0x40 /* 在软件定时器超时状态下进行删除操作，定时器进入预free态 */
#define OS_SWTMR_PRE_CREATED 0x80 /* 在软件定时器超时状态下进行停止操作，定时器进入预created态 */
#define OS_SWTMR_PRE_RUNNING 0xc0 /* 在软件定时器超时状态下进行删除操作，定时器进入预running态 */

#define OS_TIMER_PRE_FREE (OS_SWTMR_PRE_FREE | (U8)OS_TIMER_EXPIRED)
#define OS_TIMER_PRE_CREATED (OS_SWTMR_PRE_CREATED | (U8)OS_TIMER_EXPIRED)
#define OS_TIMER_PRE_RUNNING (OS_SWTMR_PRE_RUNNING | (U8)OS_TIMER_EXPIRED)

/*
 * timerHandle : timeType | timerIndex
 *               [31...28] | [27...28]
 * timeType : 0, TIMER_TYPE_HWTMR
 * timeType : 1, TIMER_TYPE_SWTMR
 *
 */
#define OS_TIMER_GET_HANDLE(type, index) (((type) << 28) | (index))
#define OS_TIMER_GET_INDEX(handle) ((handle) & 0x0FFFFFFFU)
#define OS_TIMER_GET_TYPE(handle) ((handle) >> 28)

#endif /* PRT_TIMER_EXTERNAL_H */
