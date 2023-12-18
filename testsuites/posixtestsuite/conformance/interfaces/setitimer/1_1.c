#include <stdio.h>
#include <stdint.h>
#include <signal.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include "posixtest.h"

static int setitimer_1_1_handler_tag = 0;

static void setitimer_1_1_handler(int signo)
{
    setitimer_1_1_handler_tag = 1;
}

int setitimer_1_1()
{
    sigset_t blockset;
    sigemptyset(&blockset);
    sigprocmask(SIG_SETMASK, &blockset, NULL);

    struct sigaction act;
    struct itimerval setitimer1;
    struct itimerval olditimer1;

    setitimer1.it_value.tv_sec = 1;
    setitimer1.it_value.tv_usec = 2;
    setitimer1.it_interval.tv_sec = 10;
    setitimer1.it_interval.tv_usec = 3;

    olditimer1.it_value.tv_sec = 0;
    olditimer1.it_value.tv_usec = 0;
    olditimer1.it_interval.tv_sec = 0;
    olditimer1.it_interval.tv_usec = 0;

    act.sa_flags=0;
    act.sa_handler = setitimer_1_1_handler;
    if (sigemptyset(&act.sa_mask) == -1) {
		perror("Error calling sigemptyset\n");
		return PTS_UNRESOLVED;
	}

    int signo = SIGALRM;
    int ret = sigaction(signo, &act, NULL);
    if (ret != 0) {
        printf("setitimer_1_1 install signal setitimer_1_1_handler fail ret = %d.\n", ret);
        return PTS_FAIL;
    }

    ret = setitimer(ITIMER_REAL, &setitimer1, NULL);
    if (ret != 0) {
        printf("setitimer fail ret = %d.\n", ret);
        return PTS_FAIL;
    }

    sleep(2);

    if (setitimer_1_1_handler_tag != 1) {
        printf("setitimer run fail setitimer_1_1_handler_tag = %d.\n", setitimer_1_1_handler_tag);
        return PTS_FAIL;
    }

    ret = setitimer(ITIMER_REAL, &setitimer1, &olditimer1);
    if (ret != 0) {
        printf("setitimer fail ret = %d.\n", ret);
        return PTS_FAIL;
    }

    if (setitimer1.it_value.tv_sec - olditimer1.it_value.tv_sec < 0) {
        printf("setitimer get oldtimer fail.\n");
        return PTS_FAIL;
    }

    setitimer1.it_value.tv_sec = 0;
    setitimer1.it_value.tv_usec = 0;
    setitimer1.it_interval.tv_sec = 0;
    setitimer1.it_interval.tv_usec = 0;
    ret = setitimer(ITIMER_REAL, &setitimer1, NULL);
    if (ret != 0) {
        printf("setitimer fail ret = %d.\n", ret);
        return PTS_FAIL;
    }

    return PTS_PASS;
}