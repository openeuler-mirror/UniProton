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
 * Description: 信号sigdelset测试用例
 */

#include <stdio.h>
#include <signal.h>
#include "posixtest.h"

int TEST_sigdelset_1(void)
{
    sigset_t signalset;

    if (sigemptyset(&signalset) == -1) {
        printf("sigemptyset failed -- test aborted");
        return PTS_FAIL;
    }

    if (sigaddset(&signalset, SIGALRM) == 0) {
        if (sigismember(&signalset, SIGALRM) == 0) {
            printf("sigaddset returned, sigismember failed\n");
            return PTS_FAIL;
        }
    } else {
        printf("sigaddset did not successfully add signal\n");
        return PTS_FAIL;
    }

    if (sigdelset(&signalset, SIGALRM) == 0) {
        if (sigismember(&signalset, SIGALRM) == 1) {
            printf("Signal is still in signal set.\n");
            return PTS_FAIL;
        }
    } else {
        printf("sigdelset() failed\n");
        return PTS_FAIL;
    }

    if (sigismember(&signalset, SIGALRM) == 0) {
        return PTS_PASS;
    }

    return PTS_FAIL;
}