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
static void TEST_signal_Handler1(int signo)
{
    g_TestValue--;
    printf("TEST_signal_Hander1, g_TestValue = %u.\n", g_TestValue);
    return;
}

int TEST_signal_1(void)
{
    g_TestValue = 10;

    int signo = SIGINT;
    void* sigret = signal(signo, TEST_signal_Handler1);
    if (sigret == SIG_ERR) {
        printf("TEST_signal_1 install signal handler fail ret = %d.\n", SIG_ERR);
        return PTS_FAIL;
    }

    pthread_t taskId = pthread_self();
    int ret = pthread_kill(taskId, signo);
    if (ret != 0) {
        printf("TEST_signal_1 send signal fail ret = %d.\n", ret);
    }

    /* 信号可按照注册的处理函数正常处理 */
    if (g_TestValue == 9) {
        return PTS_PASS;
    }

    return PTS_FAIL;
}

/* 测试用例2：线程先处理信号再返回上下文继续处理 */
void *TEST_signal_pthread_1()
{
    g_signo = SIGABRT;
    void* sigret = signal(g_signo, TEST_signal_Handler1);
    if (sigret == SIG_ERR) {
        error_code = 1;
        printf("TEST_signal_pthread_1 install signal handler fail ret = %d.\n", SIG_ERR);
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

void *TEST_signal_pthread_2()
{
    int ret = pthread_kill(g_pthread[0], g_signo);
    if (ret != 0) {
        printf("TEST_signal_pthread_2 send signal fail ret = %d.\n", ret);
        error_code = 3;
        return NULL;
    }
    return NULL;
}

int TEST_signal_2(void)
{
    error_code = 0;
    g_TestValue = 10;

    if(pthread_create(&g_pthread[0], NULL, TEST_signal_pthread_1, NULL) != 0) {
        printf("Error creating thread.\n");
        return PTS_FAIL;
    }

    if(pthread_create(&g_pthread[1], NULL, TEST_signal_pthread_2, NULL) != 0) {
        printf("Error creating thread.\n");
        return PTS_FAIL;
    }

    for (int i = 0; i < PTHREAD_NUN; i++) {
        pthread_join(g_pthread[i], NULL);
    }

    printf("TEST_signal_2 pthread exit, error_code: %d, g_TestValue: %u.\n", error_code, g_TestValue);
    if (error_code == 0 && g_TestValue == 7) {
        return PTS_PASS;
    }

    return PTS_FAIL;
}

/* 测试用例3：signal可返回正确的老回调函数 */
int TEST_signal_3(void)
{
    int signo = SIGQUIT;
    void *ret = signal(signo, TEST_signal_Handler1);
    if (ret == SIG_ERR) {
        printf("TEST_signal_3 set handle fail.\n");
        return PTS_FAIL;
    }

    ret = signal(signo, SIG_IGN);
    if (ret != TEST_signal_Handler1) {
        printf("TEST_signal_3 get old handle fail, %p.\n", ret);
        return PTS_FAIL;
    }

    ret = signal(signo, SIG_DFL);
    if (ret != NULL) {
        printf("TEST_signal_3 clear act fail, %p.\n", ret);
        return PTS_FAIL;
    }

    return PTS_PASS;
}