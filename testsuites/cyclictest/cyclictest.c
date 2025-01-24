#include <float.h>
#include "prt_task.h"
#include "prt_sys.h"
#include "prt_hwi.h"
#include "prt_config.h"
#include "cpu_config.h"

#define US_PER_TICK  (OS_SYS_US_PER_SECOND / OS_TICK_PER_SECOND)
U64 g_timeIntervalUs;
U64 g_loopNums;
extern U64 g_timerFrequency;

static inline void ResetTimer()
{
    U32 cfgMask = 0x0;
    U64 cycle = g_timerFrequency / OS_TICK_PER_SECOND;

    OS_EMBED_ASM("MSR CNTP_CTL_EL0, %0" : : "r"(cfgMask) : "memory");
    PRT_ISB();
    OS_EMBED_ASM("MSR CNTP_TVAL_EL0, %0" : : "r"(cycle) : "memory", "cc");

    cfgMask = 0x1;
    OS_EMBED_ASM("MSR CNTP_CTL_EL0, %0" : : "r"(cfgMask) : "memory");
    PRT_ISB();
}

void CyclicThread()
{
    printf("\n===cyclictest thread start===\n");
    int ret;
    uintptr_t intSave;
    U64 now, next, diff;
    float time_diff_us;
    U32 loop_cnt = 0;
    U64 cycle = g_timerFrequency / OS_TICK_PER_SECOND;
    U64 tick = g_timeIntervalUs / US_PER_TICK + (g_timeIntervalUs % US_PER_TICK > 0);
    float max = 0;
    float min = FLT_MAX;
    float avg = 0;

    printf("\ncyclictest time interval is %d us, loop num is %d\n", g_timeIntervalUs, g_loopNums);
    while (loop_cnt < g_loopNums) {
        intSave = PRT_HwiLock();
        ResetTimer();
        PRT_HwiRestore(intSave);
        now = OsCurCycleGet64();
        ret = PRT_TaskDelay((U32)tick);
        next = OsCurCycleGet64();
        if (ret != OS_OK) {
            printf("\nPRT_TaskDelay failed, ret = %d\n", ret);
            return;
        }

        diff = next - (now + tick * cycle);
        time_diff_us = (float)diff / (float)(g_timerFrequency / OS_SYS_US_PER_SECOND);
        max = time_diff_us > max ? time_diff_us : max;
        min = time_diff_us < min ? time_diff_us : min;
        avg += time_diff_us;
        loop_cnt++;
    }

    avg /= (float)loop_cnt;
    printf("\nMax time diff is %.2f us\n", max);
    printf("\nMin time diff is %.2f us\n", min);
    printf("\nAverage time diff is %.2f us\n", avg);
    printf("\n===cyclictest thread finish===\n");
}

void cyclictest_entry(U64 interval, U64 loopNums)
{
    U32 ret;
    g_timeIntervalUs = interval;
    g_loopNums = loopNums;
    struct TskInitParam param = {0};
    TskHandle taskId;
    param.taskEntry = (TskEntryFunc)CyclicThread;
    param.stackSize = 0x800;
    param.name = "TASK_CYC";
    param.taskPrio = OS_TSK_PRIORITY_05;
    param.stackAddr = 0;

    ret = PRT_TaskCreate(&taskId, &param);
    if (ret) {
        printf("PRT_TaskCreate failed, ret = %d\n", ret);
        return;
    }

    ret = PRT_TaskResume(taskId);
    if (ret) {
        printf("PRT_TaskResume failed, ret = %d\n", ret);
        return;
    }
}