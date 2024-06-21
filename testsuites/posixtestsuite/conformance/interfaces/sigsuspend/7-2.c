#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "posixtest.h"
#include "prt_typedef.h"
#include "prt_mem.h"
#include "prt_task.h"
#include "prt_config.h"

int SIGUSR_called_1 = 0;
int SIGUSR_called_2 = 0;
static int g_globleNum = 0;
static TskHandle g_testHandle = 0;

pthread_t g_sig1_th, g_sig2_th;
extern U8 g_numOfCores;

#if (defined(OS_OPTION_SMP) && (OS_SYS_CORE_RUN_NUM > 1))
static void handler(int signo)
{
    if (signo == SIGUSR1) {
        printf("SIGUSR1 called. Inside handler\n");
        SIGUSR_called_1 = 1;
    } 
    else if (signo == SIGUSR2) {
        printf("SIGUSR2 called. Inside handler\n");
        SIGUSR_called_2 = 1;
    }
}

void *sigsuspend_7_2_sig1_func(void *arg)
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
    g_testHandle = getpid();
    g_globleNum++;
    if (sigsuspend(&tempmask) != -1) {
        printf("sigsuspend error");
    }
    
    sleep(1);
    return 0;
}

void *sigsuspend_7_2_sig2_func(void *arg)
{
    while(g_globleNum != 1);
    //  从核sigsuspend_7_2_sig1_func 中sigsuspend 的同时，主核设置sigsuspend_7_2_sig1_func优先级，验证自旋锁是否正确
    U32 ret = PRT_TaskSetPriority(g_testHandle, 9);
    if (ret != 0) {
        printf("PRT_TaskSetPriority error\n");
    }
    while(g_globleNum != 2);
    pthread_kill(*(pthread_t *)arg, SIGUSR2);
    sleep(1);
    pthread_kill(*(pthread_t *)arg, SIGUSR1);
    sleep(1);
    g_globleNum++;
}

void slave_sigsuspend_7_2(void) {
    if(pthread_create(&g_sig1_th, NULL, sigsuspend_7_2_sig1_func, NULL) != 0)
    {    
        printf("Error creating thread\n");
        return PTS_UNRESOLVED;
    }
    printf("pthread_create success\n");

    g_globleNum++;
}

int sigsuspend_7_2()
{
    g_globleNum = 0;
    U32 ret;
    U8 ptNo = OS_MEM_DEFAULT_FSC_PT;
    struct TskInitParam param = {0};
    TskHandle testTskHandle;

    // task 1
    param.stackAddr = PRT_MemAllocAlign(0, ptNo, 0x2000, MEM_ADDR_ALIGN_016);
    param.taskEntry = (TskEntryFunc)slave_sigsuspend_7_2;
    param.taskPrio = 25;
    param.name = "SlaveTask";
    param.stackSize = 0x2000;

    ret = PRT_TaskCreate(&testTskHandle, &param);
    if (ret) {
        return ret;
    }

    if(g_numOfCores > 1) {
        ret = PRT_TaskCoreBind(testTskHandle, 1 << (PRT_GetPrimaryCore() + 1));
        if (ret) {
            return ret;
        }
    }

    ret = PRT_TaskResume(testTskHandle);
    if (ret) {
        return ret;
    }

    if(pthread_create(&g_sig2_th, NULL, sigsuspend_7_2_sig2_func, &g_sig1_th) != 0)
    {    
        printf("Error creating thread\n");
        return PTS_UNRESOLVED;
    }

    sleep(5);

    if (SIGUSR_called_2 == 1) {
        printf("Test FAILED: SIGUSR_called_1 had handle\n");
        return PTS_FAIL;    
    }

    if (SIGUSR_called_1 != 1) {
        printf("Test FAILED: SIGUSR_called_2 not handle\n");
        return PTS_FAIL;    
    }

    return PTS_PASS;
}
#endif