#include "prt_config.h"
#include "prt_task.h"
#include "prt_mem.h"
#include "prt_sem.h"
#include "prt_log.h"
#include <stdio.h>
#include "kern_test_public.h"

volatile U32 g_testFinish = 0;
volatile int g_testResult = 0;

TskHandle test_start_task(TskEntryFunc func, TskPrior taskPrio, U16 policy)
{
    U32 ret;
    TskHandle testTskHandle;
    struct TskInitParam param = {0};
    param.stackAddr = (uintptr_t)PRT_MemAllocAlign(0, OS_MEM_DEFAULT_FSC_PT, 0x3000, MEM_ADDR_ALIGN_016);
    param.taskEntry = func;
    // 支持的优先级(0~31)
    param.taskPrio = taskPrio;
    param.name = "testThread";
    param.stackSize = 0x3000;
    param.policy = policy;

    ret = PRT_TaskCreate(&testTskHandle, &param);
    if (ret) {
        printf("create task fail, %u\n", ret);
        return -1;
    }

    ret = PRT_TaskResume(testTskHandle);
    if (ret) {
        printf("resume task fail, %u\n", ret);
        return -1;
    }
    return testTskHandle;
}

TskHandle test_start_task_param(TskEntryFunc func, TskPrior taskPrio, U16 policy, uintptr_t arg1, uintptr_t arg2,
    uintptr_t arg3, uintptr_t arg4)
{
    U32 ret;
    TskHandle testTskHandle;
    struct TskInitParam param = {0};
    param.stackAddr = (uintptr_t)PRT_MemAllocAlign(0, OS_MEM_DEFAULT_FSC_PT, 0x3000, MEM_ADDR_ALIGN_016);
    param.taskEntry = func;
    // 支持的优先级(0~31)
    param.taskPrio = taskPrio;
    param.name = "testThread";
    param.stackSize = 0x3000;
    param.args[0] = arg1;
    param.args[1] = arg2;
    param.args[2] = arg3;
    param.args[3] = arg4;
    param.policy = policy;

    ret = PRT_TaskCreate(&testTskHandle, &param);
    if (ret) {
        printf("create task fail, %u\n", ret);
        return -1;
    }

    ret = PRT_TaskResume(testTskHandle);
    if (ret) {
        printf("resume task fail, %u\n", ret);
        return -1;
    }
    return testTskHandle;
}

static void prt_kern_test_entry()
{
    int i = 0;
    int cnt = 0;
    int fails = 0;
    int len = g_test_case_size / sizeof(test_case_t);
#if defined(OS_OPTION_OPENAMP)
    /* 如果使用openamp则等待tty打开后开始测试，不会错过打印 */
    while (!is_tty_ready()) {
        PRT_TaskDelay(OS_TICK_PER_SECOND / 10);
    }
#endif
    printf("\r\n===test start=== \r\n");
    for (i = 0, cnt = 0; i < len; i++) {
        test_case_t *tc = &g_cases[i];
        if (tc->skip) {
            printf("\r\n===%s skip === \r\n", tc->name);
            continue;
        }
        printf("\r\n===%s start === \r\n", tc->name);
        cnt++;
        int ret = tc->fn();
        if (ret) {
            fails++;
            printf("\r\n===%s failed ===\r\n", tc->name);
        } else {
            printf("\r\n===%s success ===\r\n", tc->name);
        }
    }
    printf("\r\n===test end, total: %d, fails:%d ===\r\n", cnt, fails);
    prt_kern_test_end();
}

void Init(uintptr_t param1, uintptr_t param2, uintptr_t param3, uintptr_t param4)
{
    U32 ret;
    struct TskInitParam param = {0};
    TskHandle testTskHandle;
    (void)(param1);
    (void)(param2);
    (void)(param3);
    (void)(param4);

    param.stackAddr = (uintptr_t)PRT_MemAllocAlign(0, OS_MEM_DEFAULT_FSC_PT, 0x3000, MEM_ADDR_ALIGN_016);
    param.taskEntry = (TskEntryFunc)prt_kern_test_entry;
    // 支持的优先级(0~31)
    param.taskPrio = 25;
    param.name = "testlog";
    param.stackSize = 0x3000;

    ret = PRT_TaskCreate(&testTskHandle, &param);
    if (ret) {
        return;
    }

    ret = PRT_TaskResume(testTskHandle);
    if (ret) {
        return;
    }
}
