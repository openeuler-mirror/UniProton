#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "posixtest.h"

int SIGUSR1_called = 0;
int SIGUSR2_called = 0;

static void handler(int signo)
{
    if (signo == SIGUSR1) {
        printf("SIGUSR1 called. Inside handler\n");
        SIGUSR1_called = 1;
    } 
    else if (signo == SIGUSR2) {
        printf("SIGUSR2 called. Inside handler\n");
        SIGUSR2_called = 1;
    }
}

void *sigsuspend_7_1_sig1_func(void *arg)
{
    sigset_t tempmask, originalmask;

    struct sigaction act;

    (void)arg;

    act.sa_handler = handler;
    act.sa_flags=0;
    sigemptyset(&act.sa_mask);
    sigemptyset(&tempmask);
    sigaddset(&tempmask, SIGUSR2);
    if (sigaction(SIGUSR1,  &act, 0) == -1) {
        printf("Unexpected error while attempting to pre-conditions");
        return PTS_UNRESOLVED;
    }

    if (sigaction(SIGUSR2,  &act, 0) == -1) {
        printf("Unexpected error while attempting to pre-conditions");
        return PTS_UNRESOLVED;
    }

    sigemptyset(&originalmask);
    sigaddset(&originalmask, SIGUSR1);
    sigprocmask(SIG_SETMASK, &originalmask, NULL);

    printf("suspending child\n");
    if (sigsuspend(&tempmask) != -1) {
        printf("sigsuspend error");
    }
            
    printf("returned from suspend\n");
    sleep(1);
    return 0;
}

void *sigsuspend_7_1_sig2_func(void *arg)
{
    pthread_kill(*(pthread_t *)arg, SIGUSR2);
    sleep(1);
    pthread_kill(*(pthread_t *)arg, SIGUSR1);
    sleep(1);
}

int sigsuspend_7_1()
{
    pthread_t sig1_th, sig2_th;
    
    if(pthread_create(&sig1_th, NULL, sigsuspend_7_1_sig1_func, NULL) != 0)
    {    
        printf("Error creating thread\n");
        return PTS_UNRESOLVED;
    }

    if(pthread_create(&sig2_th, NULL, sigsuspend_7_1_sig2_func, &sig1_th) != 0)
    {    
        printf("Error creating thread\n");
        return PTS_UNRESOLVED;
    }

    sleep(5);

    if (SIGUSR2_called == 1) {
        printf("Test FAILED: SIGUSR1_called had handle\n");
        return PTS_FAIL;    
    }

    if (SIGUSR1_called != 1) {
        printf("Test FAILED: SIGUSR2_called not handle\n");
        return PTS_FAIL;    
    }

    return PTS_PASS;
}