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
 * Description: 信号sigprocmask测试用例
 */

#include <stdio.h>
#include <stdint.h>
#include <signal.h>
#include "prt_signal.h"
#include "posixtest.h"
#include <pthread.h>

static unsigned int g_TestValue = 0;

static int isPendingSignal(int signo)
{
    sigset_t pendingSet;
    int ret = sigemptyset(&pendingSet);
    if (ret != 0) {
        printf("isPendingSignal sigemptyset fail ret = %d.\n", ret);
        return 0;
    }
    ret = sigpending(&pendingSet);
    if (ret != 0) {
        printf("isPendingSignal get sigpending fail ret = %d.\n", ret);
        return 0;
    }

    ret = sigismember(&pendingSet, signo);

    printf("isPendingSignal signo = %d, ismember = %d.\n", signo, ret);

    return (ret == 1 ? ret : 0);
}

/* 测试用例1：sigpromask阻塞的信号无法处理，SIG_UNBLOCK信号解除阻塞后可正常处理 */
static void TEST_sigpromask_Handler1(int signo)
{
    g_TestValue--;
    printf("TEST_sigpromask_Handler1, g_TestValue = %u.\n", g_TestValue);
    return;
}

int TEST_sigpromask_1(void)
{
    g_TestValue = 10;

    /* 注册信号处理函数 */
    struct sigaction act;
    act.sa_handler = TEST_sigpromask_Handler1;
    int signo = SIGINT;
    int ret = sigaction(signo, &act, NULL);
    if (ret != 0) {
        printf("TEST_sigpromask_1 install signal handler fail ret = %d.\n", ret);
        return PTS_FAIL;
    }

    /* 阻塞信号 */
    sigset_t set;
    ret = sigemptyset(&set);
    if (ret != 0) {
        printf("TEST_sigpromask_1 sigemptyset fail ret = %d.\n", ret);
        return PTS_FAIL;
    }
    ret = sigaddset(&set, signo);
    if (ret != 0) {
        printf("TEST_sigpromask_1 sigaddset fail ret = %d.\n", ret);
        return PTS_FAIL;
    }
    ret = sigprocmask(SIG_BLOCK, &set, NULL);
    if (ret != 0) {
        printf("TEST_sigpromask_1 SIG_BLOCK fail ret = %d.\n", ret);
        return PTS_FAIL;
    }

    /* 向线程发送信号，信号无法处理 */
    pthread_t taskId = pthread_self();
    ret = pthread_kill(taskId, signo);
    if (ret != 0) {
        printf("TEST_sigpromask_1 send signal fail ret = %d.\n", ret);
        return PTS_FAIL;
    }

    /* 信号被阻塞，无法处理 */
    if(g_TestValue == 9 || !isPendingSignal(signo)) {
        printf("TEST_sigpromask_1 signo(%d) is not pending.\n", signo);
        return PTS_FAIL;
    }

    /* 去除阻塞信号，信号正常处理，信号pending消除 */
    ret = sigprocmask(SIG_UNBLOCK, &set, NULL);
    if (ret != 0) {
        printf("TEST_sigpromask_1 SIG_UNBLOCK fail ret = %d.\n", ret);
        return PTS_FAIL;
    }

    /* 信号可按照注册的处理函数正常处理 */
    if (g_TestValue == 9 && !isPendingSignal(signo)) {
        return PTS_PASS;
    }

    return PTS_FAIL;
}

/* 测试用例2：sigpromask可正确返回oldset */
int TEST_sigpromask_2(void)
{
    /* SIG_SETMASK设置阻塞信号 */
    sigset_t set;
    (void)sigemptyset(&set);
    (void)sigaddset(&set, SIGINT);
    (void)sigprocmask(SIG_SETMASK, &set, NULL);

    /* SIG_BLOCK设置阻塞信号 */
    sigemptyset(&set);
    (void)sigaddset(&set, SIGSEGV);
    (void)sigprocmask(SIG_BLOCK, &set, NULL);

    /* 获取oldset */
    sigset_t oldset;
    (void)sigprocmask(SIG_BLOCK, NULL, &oldset);

    int ret = sigismember(&oldset, SIGINT);
    if (ret != 1) {
        printf("TEST_sigpromask_2 SIGINT is not member.\n");
        return PTS_FAIL;
    }
    ret = sigismember(&oldset, SIGSEGV);
    if (ret != 1) {
        printf("TEST_sigpromask_2 SIGSEGV is not member.\n");
        return PTS_FAIL;
    }
    (void)sigemptyset(&set);
    (void)sigprocmask(SIG_SETMASK, &set, NULL);

    return PTS_PASS;
}

/* 测试用例3：sigpromask阻塞的信号无法处理，SIG_SETMASK信号解除阻塞后可正常处理 */
static void TEST_sigpromask_Handler2(int signo)
{
    g_TestValue -= 2;
    printf("TEST_sigpromask_Handler2, g_TestValue = %u.\n", g_TestValue);
    return;
}

int TEST_sigpromask_3(void)
{
    g_TestValue = 10;

    /* 注册信号处理函数 */
    struct sigaction act;
    act.sa_handler = TEST_sigpromask_Handler1;
    int signo1 = SIGINT;
    (void)sigaction(signo1, &act, NULL);
    act.sa_handler = TEST_sigpromask_Handler2;
    int signo2 = SIGSEGV;
    (void)sigaction(signo2, &act, NULL);

    /* 阻塞信号 */
    sigset_t set;
    (void)sigemptyset(&set);
    (void)sigaddset(&set, signo1);
    (void)sigaddset(&set, signo2);
    (void)sigprocmask(SIG_SETMASK, &set, NULL);

    /* 向线程发送信号，信号无法处理 */
    pthread_t taskId = pthread_self();
    int ret = pthread_kill(taskId, signo1);
    ret |= pthread_kill(taskId, signo2);
    if (ret != 0) {
        printf("TEST_sigpromask_3 send signal fail ret = %d.\n", ret);
        return PTS_FAIL;
    }

    /* 信号被阻塞，无法处理 */
    if(g_TestValue != 10 || !isPendingSignal(signo1) || !isPendingSignal(signo2)) {
        printf("TEST_sigpromask_3 signo1(%d) or signo2(%d) is not pending.\n", signo1, signo2);
        return PTS_FAIL;
    }

    /* 去除阻塞信号1，信号正常处理，信号1pending消除 */
    (void)sigemptyset(&set);
    (void)sigaddset(&set, signo2);
    (void)sigprocmask(SIG_SETMASK, &set, NULL);
    /* 信号1可按照注册的处理函数正常处理 */
    if (g_TestValue != 9 || isPendingSignal(signo1)) {
        return PTS_FAIL;
    }

    /* 去除阻塞信号2，信号正常处理，信号2pending消除 */
    (void)sigemptyset(&set);
    (void)sigprocmask(SIG_SETMASK, &set, NULL);
    /* 信号2可按照注册的处理函数正常处理 */
    if (g_TestValue != 7 || isPendingSignal(signo2)) {
        return PTS_FAIL;
    }

    return PTS_PASS;
}