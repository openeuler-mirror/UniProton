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
 * Description: posix sigaction功能实现
 */
#include <errno.h>
#include "signal.h"
#include "prt_signal.h"
#include "prt_signal_external.h"

int __sigaction(int signum, const struct sigaction *__restrict act, struct sigaction *__restrict oldact)
{
    if (signum == SIGKILL || signum == SIGSTOP) {
        errno = EINVAL;
        return -1;
    }

    if (!sigValid(signum)) {
        errno = EINVAL;
        return -1;
    }

    if (act->sa_flags == SA_SIGINFO) {
        errno = ENOTSUP;
        return -1;
    }

    uintptr_t intSave = OsIntLock();
    struct TagTskCb *runTsk = RUNNING_TASK;
    if (oldact != NULL) {
        oldact->sa_handler = runTsk->sigVectors[signum];
    }

    if (act == NULL) {
        OsIntRestore(intSave);
        return 0;
    }

    runTsk->sigMask |= act->sa_mask.__bits[0];

    _sa_handler handler = act->sa_handler;
    if (handler == SIG_IGN) {
        runTsk->sigVectors[signum] = NULL;
    } else if (handler == SIG_DFL) {
        runTsk->sigVectors[signum] = OsSigDefaultHandler;
    } else {
        runTsk->sigVectors[signum] = handler;
    }

    OsIntRestore(intSave);

    return 0;
}

weak_alias(__sigaction, sigaction);