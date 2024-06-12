#include <stdio.h>

#include <linux/kthread.h>
#include <linux/err.h>
#include <linux/current.h>

#include "prt_buildef.h"
#include "prt_task.h"

// 创建n个task，唤醒后再停止，检查task返回值是否正确
extern volatile U64 g_uniTicks;
static int magical_return = 123321;
void kthreadTest(void);

static int subThread(void *data)
{
    TskHandle pid = current->pid;
    char *name = NULL;
    printf("[sub%lu] enter subThread\n", pid);
    // OsTaskNameGetHelper(pid, &name);
    if (name != NULL) {
        printf("[sub%lu] subThread name: %s\n", pid, name);
    } else {
        printf("[sub%lu] get subThread name fail\n", pid);
    }

    while (!kthread_should_stop()) {
        printf("[sub%lu] tick: %llu\n", pid, g_uniTicks);
        PRT_TaskDelay(200);
    }
    printf("[sub%lu] normal exit\n", pid);
    return (int)pid;
}

static int mainThread(void *data)
{
    struct task_struct *sub1, *sub2, *sub3;
    printf("[main] enter mainThread\n");
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
    threadRet = kthread_stop(sub1);
    printf("[main] sub1 ret %d\n", threadRet);
    if (threadRet != (int)sub1->pid) return OS_FAIL;

    PRT_TaskDelay(1000);
    threadRet = kthread_stop(sub2);
    printf("[main] sub2 ret %d\n", threadRet);
    if (threadRet != (int)sub2->pid) return OS_FAIL;

    PRT_TaskDelay(1000);
    threadRet = kthread_stop(sub3);
    printf("[main] sub3 ret %d\n", threadRet);
    if (threadRet != (int)sub3->pid) return OS_FAIL;

    printf("[main] success, tick: %llu\n", g_uniTicks);
    return magical_return;
}

void kthreadTest()
{
    char *name = NULL;
    int threadRet = 0;

    printf("enter kthread test\n");
    struct task_struct *mainTsk = kthread_run(mainThread, NULL, "main%u", 1);
    if (IS_ERR(mainTsk)) {
        printf("create thread fail, err:%lu\n", PTR_ERR(mainTsk));
        return;
    }

    // OsTaskNameGetHelper(mainTsk->pid, &name);
    if (name != NULL) {
        printf("get name: %s\n", name);
    } else {
        printf("get name fail\n");
    }

    printf("main should stop tick: %llu\n", g_uniTicks);
    threadRet = kthread_stop(mainTsk);
    printf("main stop tick: %llu\n", g_uniTicks);
    if (threadRet == magical_return) {
        printf("kthread test success!\n");
    } else {
        printf("kthread test fail, ret = %lu!\n", threadRet);
    }
}