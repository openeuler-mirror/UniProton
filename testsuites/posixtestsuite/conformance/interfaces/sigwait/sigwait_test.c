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
 * Description: 信号sigwait测试用例
 */

#include <stdio.h>
#include <stdint.h>
#include <signal.h>
#include <pthread.h>
#include <time.h>
#include "prt_signal.h"
#include "posixtest.h"
#include "prt_typedef.h"
#include "prt_mem.h"
#include "prt_task.h"
#include "prt_config.h"

#define PTHREAD_NUN 2
static unsigned int g_TestValue = 0;
static pthread_t g_pthread[PTHREAD_NUN];
static int g_signo, error_code, g_sigcode;

static void TEST_sleep_sec(unsigned int sec)
{
    struct timespec tssleepfor;
    tssleepfor.tv_sec = sec;
    tssleepfor.tv_nsec = 0;
    nanosleep(&tssleepfor, NULL);
}

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

/* 测试用例1：等待信号时已经有pending信号，可正常等到该pending信号 */
int TEST_sigwait_1(void)
{
    int signo = SIGINT;
    sigset_t set;
    int ret = sigemptyset(&set);
    if (ret != 0) {
        printf("TEST_sigwait_1 sigemptyset fail ret = %d.\n", ret);
        return PTS_FAIL;
    }
    ret = sigaddset(&set, signo);
    if (ret != 0) {
        printf("TEST_sigwait_1 sigaddset fail ret = %d.\n", ret);
        return PTS_FAIL;
    }

    ret = sigprocmask(SIG_SETMASK, &set, NULL);
    if (ret != 0) {
        printf("TEST_sigwait_1 sigprocmask fail ret = %d.\n", ret);
        return PTS_FAIL;
    }

    pthread_t taskId = pthread_self();
    ret = pthread_kill(taskId, signo);
    if (ret != 0) {
        printf("TEST_sigwait_1 send signal fail ret = %d.\n", ret);
        return PTS_FAIL;
    }
    if (!isPendingSignal(signo)) {
        printf("TEST_sigwait_1 signo(%d) is not PendingSignal.\n", signo);
        return PTS_FAIL;
    }

    int sig;
    ret = sigwait(&set, &sig);
    printf("TEST_sigwait_1 wait signal sig = %d, ret = %d.\n", sig, ret);
    if (sig != signo) {
        return PTS_FAIL;
    }

    if (isPendingSignal(signo)) {
        printf("TEST_sigwait_1 signo(%d) is still PendingSignal.\n", signo);
        return PTS_FAIL;
    }

    return PTS_PASS;
}

/* 测试用例2：等待信号中不存在pending信号，线程挂起等待，信号来后线程可恢复执行 */
void *TEST_sigwait_pthread_1()
{
    int sig;
    sigset_t set;
    int ret = sigemptyset(&set);
    if (ret != 0) {
        error_code = 1;
        return NULL;
    }
    ret = sigaddset(&set, g_signo);
    if (ret != 0) {
        error_code = 2;
        return NULL;
    }

    g_TestValue++;
    ret = sigwait(&set, &sig);
    if (ret != 0) {
        error_code = 3;
        return NULL;
    }
    if (sig != g_signo) {
        error_code = 4;
        return NULL;
    }
    g_TestValue++;

    return NULL;
}

int TEST_sigwait_2(void)
{
    error_code = 0;
    g_TestValue = 0;
    g_signo = SIGINT;

    if(pthread_create(&g_pthread[0], NULL, TEST_sigwait_pthread_1, NULL) != 0) {
        printf("Error creating thread.\n");
        return PTS_FAIL;
    }

    TEST_sleep_sec(1);
    if (g_TestValue != 1) {
        printf("TEST_sigwait_2 g_TestValue check fail g_TestValue = %u.\n", g_TestValue);
        return PTS_FAIL;
    }

    int ret = pthread_kill(g_pthread[0], g_signo);
    if (ret != 0) {
        printf("TEST_sigwait_2 send signal fail ret = %d.\n", ret);
        return PTS_FAIL;
    }
    pthread_join(g_pthread[0], NULL);

    printf("TEST_sigwait_2 sigwait end, error_code = %d, g_TestValue = %u.\n", error_code, g_TestValue);
    if (error_code == 0 && g_TestValue == 2) {
        return PTS_PASS;
    }

    return PTS_FAIL;
}

/* 测试用例3：多个pending信号时，每次只能等到一个信号 */
int TEST_sigwait_3(void)
{
    int signo1 = SIGFPE;
    int signo2 = SIGTERM;
    sigset_t set;
    int ret = sigemptyset(&set);
    if (ret != 0) {
        printf("TEST_sigwait_3 sigemptyset fail ret = %d.\n", ret);
        return PTS_FAIL;
    }
    ret = sigaddset(&set, signo1);
    ret != sigaddset(&set, signo2);
    if (ret != 0) {
        printf("TEST_sigwait_3 sigaddset fail ret = %d.\n", ret);
        return PTS_FAIL;
    }

    ret = sigprocmask(SIG_SETMASK, &set, NULL);
    if (ret != 0) {
        printf("TEST_sigwait_3 sigprocmask fail ret = %d.\n", ret);
        return PTS_FAIL;
    }

    pthread_t taskId = pthread_self();
    ret = pthread_kill(taskId, signo1);
    ret |= pthread_kill(taskId, signo2);
    if (ret != 0) {
        printf("TEST_sigwait_3 send signal fail ret = %d.\n", ret);
        return PTS_FAIL;
    }
    if (!isPendingSignal(signo1) || !isPendingSignal(signo2)) {
        printf("TEST_sigwait_3 signo1(%d) or signo2(%d) is not PendingSignal.\n", signo1, signo2);
        return PTS_FAIL;
    }

    int sig;
    ret = sigwait(&set, &sig);
    printf("TEST_sigwait_3 wait signal sig = %d, ret = %d.\n", sig, ret);
    if (sig != signo1) {
        return PTS_FAIL;
    }

    ret = sigwait(&set, &sig);
    printf("TEST_sigwait_3 wait signal again sig = %d, ret = %d.\n", sig, ret);
    if (sig != signo2) {
        return PTS_FAIL;
    }

    if (isPendingSignal(signo1) || isPendingSignal(signo2)) {
        printf("TEST_sigwait_3 signo1(%d) or signo2(%d) is still PendingSignal.\n", signo1, signo2);
        return PTS_FAIL;
    }

    return PTS_PASS;
}

/* 测试用例4：等待信号中不存在pending信号，线程挂起等待，等待超时前获得信号，可正常处理 */
void *TEST_sigtimedwait_pthread_1()
{
    siginfo_t info;
    sigset_t set;
    struct timespec timeout;
    struct timespec curtime;
    int ret = sigemptyset(&set);
    if (ret != 0) {
        error_code = 1;
        return NULL;
    }
    ret = sigaddset(&set, g_signo);
    if (ret != 0) {
        error_code = 2;
        return NULL;
    }

    g_TestValue++;
    if (clock_gettime(CLOCK_REALTIME, &curtime) != 0) {
        error_code = 3;
        return NULL;
    }
    timeout.tv_sec = curtime.tv_sec + 2;
    timeout.tv_nsec = 0;
    ret = sigtimedwait(&set, &info, &timeout);
    if (ret == -1 || info.si_signo != g_signo) {
        error_code = 4;
        return NULL;
    }
    g_TestValue++;

    return NULL;
}

int TEST_sigtimedwait_1(void)
{
    error_code = 0;
    g_TestValue = 0;
    g_signo = SIGPIPE;

    if(pthread_create(&g_pthread[0], NULL, TEST_sigtimedwait_pthread_1, NULL) != 0) {
        printf("Error creating thread.\n");
        return PTS_FAIL;
    }

    TEST_sleep_sec(1);
    if (g_TestValue != 1) {
        printf("TEST_sigtimedwait_1 g_TestValue check fail g_TestValue = %u.\n", g_TestValue);
        return PTS_FAIL;
    }

    int ret = pthread_kill(g_pthread[0], g_signo);
    if (ret != 0) {
        printf("TEST_sigtimedwait_1 send signal fail ret = %d.\n", ret);
        return PTS_FAIL;
    }

    pthread_join(g_pthread[0], NULL);

    printf("TEST_sigtimedwait_1 sigwait end, error_code = %d, g_TestValue = %u.\n", error_code, g_TestValue);
    if (error_code == 0 && g_TestValue == 2) {
        return PTS_PASS;
    }

    return PTS_FAIL;
}

/* 测试用例5：等待信号中不存在pending信号，线程挂起等待，等待超时后线程可恢复执行 */
void *TEST_sigtimedwait_pthread_2()
{
    siginfo_t info;
    sigset_t set;
    struct timespec timeout;
    struct timespec curtime;
    int ret = sigemptyset(&set);
    if (ret != 0) {
        error_code = 1;
        return NULL;
    }
    ret = sigaddset(&set, g_signo);
    if (ret != 0) {
        error_code = 2;
        return NULL;
    }

    g_TestValue++;
    if (clock_gettime(CLOCK_REALTIME, &curtime) != 0) {
        error_code = 3;
        return NULL;
    }
    timeout.tv_sec = curtime.tv_sec + 2;
    timeout.tv_nsec = 0;
    ret = sigtimedwait(&set, &info, &timeout);
    if (ret != -1) {
        error_code = 4;
        return NULL;
    }
    g_TestValue++;

    return NULL;
}

int TEST_sigtimedwait_2(void)
{
    error_code = 0;
    g_TestValue = 0;
    g_signo = SIGTERM;

    if(pthread_create(&g_pthread[0], NULL, TEST_sigtimedwait_pthread_2, NULL) != 0) {
        printf("Error creating thread.\n");
        return PTS_FAIL;
    }

    TEST_sleep_sec(1);
    if (g_TestValue != 1) {
        printf("TEST_sigtimedwait_2 g_TestValue check fail g_TestValue = %u.\n", g_TestValue);
        return PTS_FAIL;
    }

    pthread_join(g_pthread[0], NULL);

    printf("TEST_sigtimedwait_2 sigwait end, error_code = %d, g_TestValue = %u.\n", error_code, g_TestValue);
    if (error_code == 0 && g_TestValue == 2) {
        return PTS_PASS;
    }

    return PTS_FAIL;
}

/* 测试用例6：等待到的信号，可获取正确的信号信息 */
void *TEST_sigwaitinfo_pthread_1()
{
    siginfo_t info;
    sigset_t set;
    int ret = sigemptyset(&set);
    if (ret != 0) {
        error_code = 1;
        return NULL;
    }
    ret = sigaddset(&set, g_signo);
    if (ret != 0) {
        error_code = 2;
        return NULL;
    }

    g_TestValue++;
    ret = sigwaitinfo(&set, &info);
    if (ret == -1 || info.si_signo != g_signo || info.si_code != g_sigcode) {
        error_code = 4;
        printf("TEST_sigwaitinfo_pthread_1 waitinfo fail, ret:%d, signo:%d, code:%d.\n",
            ret, info.si_signo, info.si_code);
        return NULL;
    }
    g_TestValue++;

    return NULL;
}

int TEST_sigwaitinfo_1(void)
{
    error_code = 0;
    g_TestValue = 0;
    g_signo = SIGPIPE;
    g_sigcode = SI_TKILL;
    if(pthread_create(&g_pthread[0], NULL, TEST_sigwaitinfo_pthread_1, NULL) != 0) {
        printf("Error creating thread.\n");
        return PTS_FAIL;
    }

    TEST_sleep_sec(1);
    if (g_TestValue != 1) {
        printf("TEST_sigwaitinfo_1 g_TestValue check fail g_TestValue = %u.\n", g_TestValue);
        return PTS_FAIL;
    }

    signalInfo info = {0};
    info.si_signo = g_signo;
    info.si_code = g_sigcode;
    unsigned int ret = PRT_SignalDeliver(g_pthread[0], &info);
    if (ret != 0) {
        printf("TEST_sigwaitinfo_1 send signal fail ret = %u.\n", ret);
        return PTS_FAIL;
    }

    pthread_join(g_pthread[0], NULL);

    printf("TEST_sigwaitinfo_1 sigwait end, error_code = %d, g_TestValue = %u.\n", error_code, g_TestValue);
    if (error_code == 0 && g_TestValue == 2 && !isPendingSignal(g_signo)) {
        return PTS_PASS;
    }

    return PTS_FAIL;
}

#if (defined(OS_OPTION_SMP) && (OS_SYS_CORE_RUN_NUM > 1))
/* 测试用例7：等待信号中不存在pending信号，线程挂起等待，同时改变线程优先级，信号来同时改变线程优先级，
    后线程可恢复执行 */
static TskHandle g_testHandle;
extern U8 g_numOfCores;
void *TEST_sigwait_pthread_7()
{
    int sig;
    sigset_t set;
    int ret = sigemptyset(&set);
    if (ret != 0) {
        error_code = 1;
        return NULL;
    }
    ret = sigaddset(&set, g_signo);
    if (ret != 0) {
        error_code = 2;
        return NULL;
    }
    g_testHandle = getpid();

    g_TestValue++;
    ret = sigwait(&set, &sig);
    if (ret != 0) {
        error_code = 3;
        return NULL;
    }
    if (sig != g_signo) {
        error_code = 4;
        return NULL;
    }
    g_TestValue++;

    return NULL;
}

int TEST_slave_sigwait(void)
{
    error_code = 0;
    g_TestValue = 0;
    g_signo = SIGINT;

    if(pthread_create(&g_pthread[0], NULL, TEST_sigwait_pthread_7, NULL) != 0) {
        printf("Error creating thread.\n");
        return PTS_FAIL;
    }

    TEST_sleep_sec(1);
    if (g_TestValue != 1) {
        printf("TEST_sigwait_2 g_TestValue check fail g_TestValue = %u.\n", g_TestValue);
        return PTS_FAIL;
    }

    g_TestValue++;
    /* 主核pthread_kill 同时，从核改变对应任务优先级，验证PRT_SignalDeliver是否生效 */
    U32 ret = PRT_TaskSetPriority(g_testHandle, 10);
    if (ret != 0) {
        printf("PRT_TaskSetPriority error\n");
    }

    return PTS_PASS;
}

int TEST_sigwait_7(void)
{
    U32 ret;
    U8 ptNo = OS_MEM_DEFAULT_FSC_PT;
    struct TskInitParam param = {0};
    TskHandle testTskHandle;

    // task 1
    param.stackAddr = PRT_MemAllocAlign(0, ptNo, 0x2000, MEM_ADDR_ALIGN_016);
    param.taskEntry = (TskEntryFunc)TEST_slave_sigwait;
    param.taskPrio = 25;
    param.name = "SlaveTask";
    param.stackSize = 0x2000;

    ret = PRT_TaskCreate(&testTskHandle, &param);
    if (ret) {
        return ret;
    }

    
    ret = PRT_TaskCoreBind(testTskHandle, 1 << (PRT_GetPrimaryCore() + 1));
    if (ret) {
        return ret;
    }

    ret = PRT_TaskResume(testTskHandle);
    if (ret) {
        return ret;
    }

    while(g_TestValue != 1);
    /* 从核sigwait 同时，主核改变对应任务优先级，验证PRT_SignalWait是否生效 */
    ret = PRT_TaskSetPriority(g_testHandle, 15);
    if (ret != 0) {
        printf("PRT_TaskSetPriority error\n");
    }
    while(g_TestValue != 2);

    ret = pthread_kill(g_pthread[0], g_signo);
    if (ret != 0) {
        printf("TEST_sigwait_2 send signal fail ret = %d.\n", ret);
        return PTS_FAIL;
    }
    pthread_join(g_pthread[0], NULL);

    printf("TEST_sigwait_2 sigwait end, error_code = %d, g_TestValue = %u.\n", error_code, g_TestValue);
    if (error_code == 0 && g_TestValue == 3) {
        return PTS_PASS;
    }
}
#endif