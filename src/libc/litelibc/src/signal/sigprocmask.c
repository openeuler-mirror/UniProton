/*
 * Copyright (c) 2023-2023 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Create: 2023-06-08
 * Description: posix sigprocmask功能实现
 */
#include "signal.h"
#include "prt_signal.h"
#include "prt_signal_external.h"

int sigprocmask(int how, const sigset_t *__restrict set, sigset_t *__restrict oldset)
{
    uintptr_t intSave = OsIntLock();
    struct TagTskCb *runTsk = RUNNING_TASK;
    if (oldset != NULL) {
        oldset->__bits[0] = runTsk->sigMask;
    }

    if (set != NULL) {
        signalSet prtSet = set->__bits[0];
        switch(how) {
            case SIG_BLOCK:
                runTsk->sigMask |= prtSet;
                break;
            case SIG_UNBLOCK:
                runTsk->sigMask &= ~prtSet;
                OsHandleUnBlockSignal(runTsk);
                break;
            case SIG_SETMASK:
                runTsk->sigMask = prtSet;
                OsHandleUnBlockSignal(runTsk);
                break;
            default:
                break;
        }
    }

    OsIntRestore(intSave);
    return 0;
}