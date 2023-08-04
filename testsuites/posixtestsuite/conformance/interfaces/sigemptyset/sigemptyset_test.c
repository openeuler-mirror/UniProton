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
 * Create: 2023-06-18
 * Description: 信号sigemptyset测试用例
 */

#include <stdio.h>
#include <signal.h>
#include <string.h>
#include "posixtest.h"

#define NUMSIGNALS 28

int TEST_sigemptyset_1(void)
{
    sigset_t signalset;
    int test_failed = 0;

    int siglist[] = { SIGABRT, SIGALRM, SIGBUS, SIGCHLD,
                        SIGCONT, SIGFPE, SIGHUP, SIGILL, SIGINT,
                        SIGKILL, SIGPIPE, SIGQUIT, SIGSEGV, SIGSTOP,
                        SIGTERM, SIGTSTP, SIGTTIN, SIGTTOU, SIGUSR1,
                        SIGUSR2, SIGPOLL, SIGPROF, SIGSYS, SIGTRAP,
                        SIGURG, SIGVTALRM, SIGXCPU, SIGXFSZ };

    if (sigfillset(&signalset) == -1) {
        return PTS_FAIL;
    }

    for (int i = NUMSIGNALS - 1; i >= 0; i--) {
        if (sigismember(&signalset, siglist[i]) == 0) {
            printf("TEST_sigemptyset_1 signo(%d) is not member.\n", siglist[i]);
            test_failed = 1;
        }
    }

    if (test_failed == 1) {
        return PTS_FAIL;
    }

    test_failed = 0;
    if (sigemptyset(&signalset) == -1) {
        printf("sigemptyset failed -- test aborted");
        return PTS_FAIL;
    }

    for (int i = NUMSIGNALS - 1; i >= 0; i--) {
        if (sigismember(&signalset, siglist[i]) == 1) {
            printf("TEST_sigemptyset_1 signo(%d) is still member.\n", siglist[i]);
            test_failed = 1;
        }
    }

    if (test_failed == 1) {
        return PTS_FAIL;
    }

    return PTS_PASS;
}
