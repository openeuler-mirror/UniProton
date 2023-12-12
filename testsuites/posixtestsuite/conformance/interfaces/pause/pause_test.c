#include <stdio.h>
#include <stdint.h>
#include <signal.h>
#include <pthread.h>
#include <unistd.h>
#include "prt_signal.h"
#include "prt_task.h"
#include "posixtest.h"

#define PTHREAD_NUN 2
static unsigned int g_TestValue = 0;
static pthread_t g_pthread[PTHREAD_NUN];
static int g_signo, error_code;

static void TEST_pause_Handler1(int signo)
{
    g_TestValue--;
    printf("TEST_pause_Hander1, g_TestValue = %u.\n", g_TestValue);
    return;
}

/* 测试用例：pause之后收到信号可以唤醒 */
void *TEST_pause_pthread()
{
    PRT_TaskDelay(1000);
    if (g_TestValue != 9) {
        printf("TEST_pause_pthread too early\n");
        error_code = 1;
        return NULL;
    }
    int ret = pthread_kill(g_pthread[0], g_signo);
    if (ret != 0) {
        printf("TEST_pause_pthread send signal fail ret = %d.\n", ret);
        error_code = 3;
        return NULL;
    }
    return NULL;
}

int TEST_pause(void)
{
    error_code = 0;
    g_TestValue = 10;

    g_pthread[0] = pthread_self();

    if(pthread_create(&g_pthread[1], NULL, TEST_pause_pthread, NULL) != 0) {
        printf("Error creating thread.\n");
        return PTS_FAIL;
    }
    void* ret = signal(SIGABRT, TEST_pause_Handler1);
    if (ret == SIG_ERR) {
        printf("TEST_pause set signal handler fiale\n");
        return PTS_FAIL;
    }
    g_signo = SIGABRT;
    g_TestValue--;
    pause();

    if (g_TestValue != 8) {
        printf("TEST_pause signal handler doesn't trigger\n");
        return PTS_FAIL;
    }

    pthread_join(g_pthread[1], NULL);

    printf("TEST_pause_2 pthread exit, error_code: %d, g_TestValue: %u.\n", error_code, g_TestValue);
    if (error_code == 0 && g_TestValue == 8) {
        return PTS_PASS;
    }

    return PTS_FAIL;
}