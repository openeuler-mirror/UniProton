#include <stdio.h>
#include <stdint.h>
#include <signal.h>
#include <pthread.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include "posixtest.h"

static int handler_tag = 0;

static void handler(int signo)
{
    handler_tag = 1;
}

int setitimer_1_2()
{
    struct sigaction act;
    act.sa_handler = handler;
    struct itimerval setitimer1;

    setitimer1.it_interval.tv_sec = 1;
    setitimer1.it_interval.tv_usec = 0;
    setitimer1.it_value.tv_sec = 0;
    setitimer1.it_value.tv_usec = 0;

    int signo = SIGALRM;
    int ret = sigaction(signo, &act, NULL);
    if (ret != 0) {
        printf("setitimer_1_2 install signal handler fail ret = %d.\n", ret);
        return PTS_FAIL;
    }

    ret = setitimer(ITIMER_REAL, NULL, NULL);
    if (ret == 0) {
        printf("setitimer need fail ret = %d.\n", ret);
        return PTS_FAIL;
    }

    ret = setitimer(ITIMER_VIRTUAL, &setitimer1, NULL);
    if (ret == 0 || errno != EINVAL) {
        printf("setitimer need fail ret = %d.\n", ret);
        return PTS_FAIL;
    }

    ret = setitimer(ITIMER_PROF, &setitimer1, NULL);
    if (ret == 0 || errno != EINVAL) {
        printf("setitimer need fail ret = %d.\n", ret);
        return PTS_FAIL;
    }

    ret = setitimer(100, &setitimer1, NULL);
    if (ret == 0 || errno != EINVAL) {
        printf("setitimer need fail ret = %d.\n", ret);
        return PTS_FAIL;
    }

    return PTS_PASS;
}