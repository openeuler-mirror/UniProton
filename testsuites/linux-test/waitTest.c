#include <stdio.h>

#include <linux/wait.h>
#include <linux/kthread.h>
#include <linux/err.h>
#include <linux/current.h>

#include "prt_buildef.h"
#include "prt_task.h"

// 创建n个task，wait_event之后多次唤醒，删除部分task，以及改变condtion为的值

extern volatile U64 g_uniTicks;

static int magical_return = 123321;
static struct wait_queue_head g_waitQ;
static int g_condition;

void waitTest(void);

static int subThread(void *data)
{
    TskHandle pid = current->pid;
    printf("[sub%lu] enter subThread\n", pid);
    int ret = wait_event_interruptible(g_waitQ, g_condition == 1);
    printf("[sub%lu] wait event 1, g_condition: %d\n", pid, g_condition);
    if (ret != 0) goto WAKE_FAIL;

    ret = wait_event_interruptible(g_waitQ, g_condition == 0);
    printf("[sub%lu] wait event 2, g_condition: %d\n", pid, g_condition);
    if (ret != 0) goto WAKE_FAIL;

    ret = wait_event_interruptible(g_waitQ, g_condition == 1);
    printf("[sub%lu] wait event 3, g_condition: %d\n", pid, g_condition);
    if (ret != 0) goto WAKE_FAIL;

    ret = wait_event_interruptible(g_waitQ, g_condition == 1);
    printf("[sub%lu] wait event 4, g_condition: %d\n", pid, g_condition);
    if (ret != 0) goto WAKE_FAIL;

    ret = wait_event_interruptible(g_waitQ, g_condition == 0);
    printf("[sub%lu] wait event 5, g_condition: %d\n", pid, g_condition);
    if (ret != 0) goto WAKE_FAIL;
    return (int)pid;

WAKE_FAIL:
    printf("[sub%lu] wait fail, ret:%d\n", pid, ret);
    return OS_FAIL;
}

#define wake_up_and_printf                                                                                             \
    do {                                                                                                               \
        printf("[main] wake up, condition %d\n", g_condition);                                                         \
        wake_up_all(&g_waitQ);                                                                                         \
        PRT_TaskDelay(100);                                                                                            \
    } while (0)

static int mainThread(void *data)
{
    struct task_struct *sub1, *sub2, *sub3;
    printf("[main] enter mainThread\n");
    init_waitqueue_head(&g_waitQ);
    g_condition = 0;
    int threadRet = 0;
    sub1 = kthread_run(subThread, NULL, "sub%u", 1);
    if (IS_ERR(sub1)) {
        printf("[main] create sub1 fail, err:%lu\n", PTR_ERR(sub1));
        return OS_FAIL;
    }
    sub2 = kthread_run(subThread, NULL, "sub%u", 2);
    if (IS_ERR(sub2)) {
        printf("[main] create sub2 fail, err:%lu\n", PTR_ERR(sub2));
        return OS_FAIL;
    }
    sub3 = kthread_run(subThread, NULL, "sub%u", 3);
    if (IS_ERR(sub3)) {
        printf("[main] create sub3 fail, err:%lu\n", PTR_ERR(sub3));
        return OS_FAIL;
    }
    PRT_TaskDelay(1000);

    wake_up_and_printf;
    wake_up_and_printf;
    wake_up_and_printf;

    g_condition = 1;

    wake_up_and_printf;
    PRT_TaskDelay(20000);
    wake_up_and_printf;

    g_condition = 0;

    wake_up_and_printf;
    wake_up_and_printf;

    PRT_TaskDelete(sub3->pid);

    wake_up_and_printf;

    g_condition = 1;

    wake_up_and_printf;
    PRT_TaskDelete(sub2->pid);

    g_condition = 0;

    wake_up_and_printf;
    wake_up_and_printf;

    printf("[main] finish wake-up\n");

    threadRet = kthread_stop(sub1);
    printf("[main] pid:%d, ret:%d", (int)sub1->pid, threadRet);
    if (threadRet != (int)sub1->pid) return OS_FAIL;
    
    printf("[main] success\n");
    return magical_return;
}

void waitTest()
{
    int threadRet = 0;

    printf("enter wait test\n");
    struct task_struct *mainTsk = kthread_run(mainThread, NULL, "main%u", 1);
    if (IS_ERR(mainTsk)) {
        printf("create thread fail, err:%lu\n", PTR_ERR(mainTsk));
        return;
    }

    printf("main should stop tick: %llu\n", g_uniTicks);
    threadRet = kthread_stop(mainTsk);
    printf("main stop tick: %llu\n", g_uniTicks);
    if (threadRet == magical_return) {
        printf("wait test success!\n");
    } else {
        printf("wait test fail, ret = %lu!\n", threadRet);
    }
}