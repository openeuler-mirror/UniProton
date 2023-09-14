/*
 * Copyright (c) 2022-2022 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Create: 2023-08-31
 * Description: abort 功能实现
 */

#include <stdlib.h>
#include <signal.h>
#include "pthread_impl.h"
#include "prt_task_external.h"
#include "prt_signal_external.h"
#include "atomic.h"
#include "lock.h"
#include "ksigaction.h"

_Noreturn void abort(void)
{
    uintptr_t intSave;
    intSave = OsIntLock();

    signalInfo info = {0};
    info.si_signo = SIGABRT;
    info.si_code = SI_KERNEL;

    sigset_t set;
    (void)sigemptyset(&set);
    (void)sigaddset(&set, SIGABRT);
    for (int i = 0; i < g_tskMaxNum; i++) {
        if (g_tskCbArray[i].taskPid == IDLE_TASK_ID) {
            continue;
        }
        if (g_tskCbArray[i].taskStatus == OS_TSK_UNUSED) {
            continue;
        }
        if (g_tskCbArray[i].taskStatus == OS_TSK_RUNNING) {
            continue;
        }
        
        (void)PRT_SignalDeliver(g_tskCbArray[i].taskPid, &info);
    }
    (void)PRT_SignalDeliver(RUNNING_TASK->taskPid, &info);

    /* If there was a SIGABRT handler installed and it returned, or if
     * SIGABRT was blocked or ignored, take an AS-safe lock to prevent
     * sigaction from installing a new SIGABRT handler, uninstall any
     * handler that may be present, and re-raise the signal to generate
     * the default action of abnormal termination. */
    for (int i = 0; i < g_tskMaxNum; i++) {
        if (g_tskCbArray[i].taskPid == IDLE_TASK_ID) {
            continue;
        }
        if (g_tskCbArray[i].taskStatus == OS_TSK_UNUSED) {
            continue;
        }
        
        g_tskCbArray[i].sigMask |= -1UL;
        g_tskCbArray[i].sigVectors[SIGABRT] = OsSigDefaultHandler;
    }
    LOCK(__abort_lock);
    for (int i = 0; i < g_tskMaxNum; i++) {
        if (g_tskCbArray[i].taskPid == IDLE_TASK_ID) {
            continue;
        }
        if (g_tskCbArray[i].taskStatus == OS_TSK_UNUSED) {
            continue;
        }

        (void)PRT_SignalDeliver(g_tskCbArray[i].taskPid, &info);
    }
    for (int i = 0; i < g_tskMaxNum; i++) {
        if (g_tskCbArray[i].taskPid == IDLE_TASK_ID) {
            continue;
        }
        if (g_tskCbArray[i].taskStatus == OS_TSK_UNUSED) {
            continue;
        }
        if (g_tskCbArray[i].taskStatus == OS_TSK_RUNNING) {
            continue;
        }
        
        g_tskCbArray[i].sigMask &= ~set.__bits[0];
        OsHandleUnBlockSignal(&g_tskCbArray[i]);
    }
    RUNNING_TASK->sigMask &= ~set.__bits[0];
    OsHandleUnBlockSignal(RUNNING_TASK);
	OsIntRestore(intSave);

    /* Beyond this point should be unreachable. */
    a_crash();
    info.si_signo = SIGKILL;
    info.si_code = SI_KERNEL;

    for (int i = 0; i < g_tskMaxNum; i++) {
        if (g_tskCbArray[i].taskPid == IDLE_TASK_ID) {
            continue;
        }
        if (g_tskCbArray[i].taskStatus == OS_TSK_UNUSED) {
            continue;
        }
        
        (void)PRT_SignalDeliver(g_tskCbArray[i].taskPid, &info);
    }
    _Exit(127);
}
