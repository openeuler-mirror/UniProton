#include <stdio.h>
#include <stdint.h>
#include <signal.h>
#include <pthread.h>
#include <time.h>
#include <errno.h>
#include <sys/time.h>
#include "posixtest.h"

static int handler_tag = 0;

static void handler(int signo)
{
    handler_tag = 1;
}

int getitimer_1_2()
{
    struct sigaction act;
    act.sa_handler = handler;
    struct itimerval setitimer1;
    struct itimerval getitimer1;

    setitimer1.it_interval.tv_sec = 10;
    setitimer1.it_interval.tv_usec = 0;
    setitimer1.it_value.tv_sec = 0;
    setitimer1.it_value.tv_usec = 0;

    getitimer1.it_interval.tv_sec = 0;
    getitimer1.it_interval.tv_usec = 0;
    getitimer1.it_value.tv_sec = 0;
    getitimer1.it_value.tv_usec = 0;

    int signo = SIGALRM;
    int ret = sigaction(signo, &act, NULL);
    if (ret != 0) {
        printf("getitimer_1_2 install signal handler fail ret = %d.\n", ret);
        return PTS_FAIL;
    }

    ret = setitimer(ITIMER_REAL, &setitimer1, NULL);
    if (ret != 0) {
        printf("setitimer fail ret = %d.\n", ret);
        return PTS_FAIL;
    }

     ret = getitimer(ITIMER_REAL, NULL);
    if (ret == 0) {
        printf("getitimer need fail ret = %d.\n", ret);
        return PTS_FAIL;
    }

    ret = getitimer(ITIMER_VIRTUAL, &getitimer1);
    if (ret == 0 || errno != EINVAL) {
        printf("getitimer need fail ret = %d.\n", ret);
        return PTS_FAIL;
    }

    ret = getitimer(ITIMER_PROF, &getitimer1);
    if (ret == 0 || errno != EINVAL) {
        printf("getitimer need fail ret = %d.\n", ret);
        return PTS_FAIL;
    }

    ret = getitimer(100, &getitimer1);
    if (ret == 0 || errno != EINVAL) {
        printf("getitimer need fail ret = %d.\n", ret);
        return PTS_FAIL;
    }

    return PTS_PASS;
}