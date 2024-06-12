#include <stdio.h>

#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/err.h>
#include <linux/current.h>
#include <pthread.h>
#include <signal.h>

#include "prt_buildef.h"
#include "prt_task.h"

// 创建一个task, 测试schedule_timeout之后能不能被提前唤醒
extern volatile U64 g_uniTicks;
static int magical_return = 123321;
void schedTest(void);

static int subThread(void *data)
{
    TskHandle pid = current->pid;
    printf("[sub%lu] enter subThread\n", pid);
    set_current_state(TASK_UNINTERRUPTIBLE);
    long ret = schedule_timeout(2000);
    printf("[sub%lu] ticks remains:%ld\n", pid, ret);
    if (ret > 1100 || ret < 900) {
        printf("[sub%lu] wake up fail, not in time\n", pid);
        return OS_FAIL;
    }

    set_current_state(TASK_INTERRUPTIBLE);
    ret = schedule_timeout(2000);
    printf("[sub%lu] ticks remains:%ld\n", pid, ret);
    if (ret > 1100 || ret < 900) {
        printf("[sub%lu] wake up fail, not in time\n", pid);
        return OS_FAIL;
    }
    return (int)pid;
}

static int mainThread(void *data)
{
    struct task_struct *sub1;
    printf("[main] enter mainThread\n");
    int threadRet = 0;
    sub1 = kthread_run(subThread, NULL, "sub%u", 1);
    if (IS_ERR(sub1)) {
        printf("[main] create sub1 fail, err:%lu\n", PTR_ERR(sub1));
        return OS_FAIL;
    }

    // task not yet sleep
    printf("[main] wake up first time\n");
    int ret = wake_up_process(sub1);
    if (ret != 0) {
        printf("[main] ret != 0, %d\n", ret);
        return OS_FAIL;
    }

    // wait task to sleep
    PRT_TaskDelay(500);
    printf("[main] wake up second time\n");
    // TASK_UNINTERRUPTIBLE 发送信号不应该有效果
    ret = pthread_kill((pthread_t)sub1->pid, SIGABRT);
    PRT_TaskDelay(500);
    ret = wake_up_process(sub1);
    if (ret != 1) {
        printf("[main] ret != 1, %d\n", ret);
        return OS_FAIL;
    }

    // wait task to sleep
    PRT_TaskDelay(1000);
    printf("[main] wake up thrid time\n");
    // 通过发送信号唤醒
    ret = pthread_kill((pthread_t)sub1->pid, SIGABRT);
    if (ret != 0) {
        printf("[main] send signal fail, ret %d\n", ret);
        return OS_FAIL;
    }
    threadRet = kthread_stop(sub1);
    if (threadRet != (int)sub1->pid) return OS_FAIL;
    return magical_return;
}

void schedTest()
{
    int threadRet = 0;

    printf("enter sched test\n");
    struct task_struct *mainTsk = kthread_run(mainThread, NULL, "main%u", 1);
    if (IS_ERR(mainTsk)) {
        printf("create thread fail, err:%lu\n", PTR_ERR(mainTsk));
        return;
    }

    printf("main should stop tick: %llu\n", g_uniTicks);
    threadRet = kthread_stop(mainTsk);
    printf("main stop tick: %llu\n", g_uniTicks);
    if (threadRet == magical_return) {
        printf("sched test success!\n");
    } else {
        printf("sched test fail, ret = %lu!\n", threadRet);
    }
}