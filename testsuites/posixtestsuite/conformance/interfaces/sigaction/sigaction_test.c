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
 * Description: 信号sigaction测试用例
 */

#include <stdio.h>
#include <stdint.h>
#include <signal.h>
#include <pthread.h>
#include "prt_signal.h"
#include "prt_task.h"
#include "posixtest.h"

#define PTHREAD_NUN 2
static unsigned int g_TestValue = 0;
static pthread_t g_pthread[PTHREAD_NUN];
static int g_signo, error_code;

static void TEST_sleep_sec(unsigned int sec)
{
    struct timespec tssleepfor;
    tssleepfor.tv_sec = sec;
    tssleepfor.tv_nsec = 0;
    nanosleep(&tssleepfor, NULL);
}

/* 测试用例1：信号可按照注册的处理函数正常处理 */
static void TEST_sigaction_Handler1(int signo)
{
    g_TestValue--;
    printf("TEST_sigaction_Hander1, g_TestValue = %u.\n", g_TestValue);
    return;
}

int TEST_sigaction_1(void)
{
    g_TestValue = 10;
    struct sigaction act;
    act.sa_handler = TEST_sigaction_Handler1;

    int signo = SIGINT;
    int ret = sigaction(signo, &act, NULL);
    if (ret != 0) {
        printf("TEST_sigaction_1 install signal handler fail ret = %d.\n", ret);
        return PTS_FAIL;
    }

    pthread_t taskId = pthread_self();
    ret = pthread_kill(taskId, signo);
    if (ret != 0) {
        printf("TEST_sigaction_1 send signal fail ret = %d.\n", ret);
    }

    /* 信号可按照注册的处理函数正常处理 */
    if (g_TestValue == 9) {
        return PTS_PASS;
    }

    return PTS_FAIL;
}

/* 测试用例2：线程先处理信号再返回上下文继续处理 */
void *TEST_sigaction_pthread_1()
{
    struct sigaction act;
    act.sa_handler = TEST_sigaction_Handler1;

    g_signo = SIGABRT;
    int ret = sigaction(g_signo, &act, NULL);
    if (ret != 0) {
        error_code = 1;
        printf("TEST_sigaction_pthread_1 install signal handler fail ret = %d.\n", ret);
        return NULL;
    }
    g_TestValue--;
    TEST_sleep_sec(1);
    if (g_TestValue != 8) {
        error_code = 2;
        return NULL;
    }
    g_TestValue--;
    return NULL;
}

void *TEST_sigaction_pthread_2()
{
    int ret = pthread_kill(g_pthread[0], g_signo);
    if (ret != 0) {
        printf("TEST_sigaction_pthread_2 send signal fail ret = %d.\n", ret);
        error_code = 3;
        return NULL;
    }
    return NULL;
}

int TEST_sigaction_2(void)
{
    error_code = 0;
    g_TestValue = 10;

    if(pthread_create(&g_pthread[0], NULL, TEST_sigaction_pthread_1, NULL) != 0) {
        printf("Error creating thread.\n");
        return PTS_FAIL;
    }

    if(pthread_create(&g_pthread[1], NULL, TEST_sigaction_pthread_2, NULL) != 0) {
        printf("Error creating thread.\n");
        return PTS_FAIL;
    }

    for (int i = 0; i < PTHREAD_NUN; i++) {
        pthread_join(g_pthread[i], NULL);
    }

    printf("TEST_sigaction_2 pthread exit, error_code: %d, g_TestValue: %u.\n", error_code, g_TestValue);
    if (error_code == 0 && g_TestValue == 7) {
        return PTS_PASS;
    }

    return PTS_FAIL;
}

/* 测试用例3：sigaction可返回正确的oldact */
int TEST_sigaction_3(void)
{
    struct sigaction act;
    act.sa_handler = TEST_sigaction_Handler1;
    struct sigaction oldact;

    int signo = SIGQUIT;
    (void)sigaction(signo, &act, &oldact);
    if (oldact.sa_handler == NULL) {
        printf("TEST_sigaction_3 oldact is NULL.\n");
        return PTS_FAIL;
    }

    act.sa_handler = SIG_IGN;
    (void)sigaction(signo, &act, &oldact);
    if (oldact.sa_handler != TEST_sigaction_Handler1) {
        printf("TEST_sigaction_3 get olact fail.\n");
        return PTS_FAIL;
    }

    (void)sigaction(signo, NULL, &oldact);
    if (oldact.sa_handler != NULL) {
        printf("TEST_sigaction_3 clear act fail.\n");
        return PTS_FAIL;
    }

    return PTS_PASS;
}