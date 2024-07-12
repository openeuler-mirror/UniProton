#include "prt_config.h"
#include "prt_task.h"
#include "prt_mem.h"
#include "prt_sem.h"
#include "prt_log.h"
#include "prt_clk.h"
#include "prt_sys_external.h"
#include "prt_task_external.h"
#include "securec.h"
#include "time.h"
#include "kern_test_public.h"

#define TASK_NUM 10

static volatile U64 g_task_count[TASK_NUM] = {0};
static volatile U64 g_task_prev_count[TASK_NUM] = {0};

static void test_sched_rr_tsk(uintptr_t index, uintptr_t arg2, uintptr_t arg3, uintptr_t arg4)
{
    (void)arg2;
    (void)arg3;
    (void)arg4;
    TEST_LOG_FMT("[sched_test_tsk] task %d enter", (int)index);
    while(!g_testFinish) {
        g_task_count[index]++;
    }
    TEST_LOG_FMT("[sched_test_tsk] task %d finish", (int)index);
    return;
}

#if !defined(OS_OPTION_RR_SCHED)
static bool test_check_only_first_task_sched(void) {
    if (g_task_count[0] == 0) {
        TEST_LOG("[sched_rr] error: task 0 count unchange");
        return false;
    }
    for (int task = 1; task < TASK_NUM; task++) {
        if (g_task_count[task] != 0) {
            TEST_LOG_FMT("[sched_rr] error: task %d count changed", task);
            return false;
        }
    }
    return true;
}
#endif

/**
 * 10个同优先级任务，每个任务在循环中加计数
 * 每一秒计数都会有变化，测10秒出结果
 * （关闭RR功能后，只有第一个任务的计数在增长）
*/
static int test_sched_rr(void)
{
    int task, sec;
    g_testFinish = 0;
    for (task = 0; task < TASK_NUM; task++) {
        g_task_prev_count[task] = 0;
        g_task_count[task] = 0;
        test_start_task_param(test_sched_rr_tsk, 26, OS_TSK_SCHED_RR, task, 0, 0, 0);
    }
    // 10秒测试
    for (sec = 0; sec < 10; sec++) {
        PRT_TaskDelay(OS_TICK_PER_SECOND);
        TEST_LOG_FMT("[sched_rr] %d second pass", sec + 1);
#if defined(OS_OPTION_RR_SCHED)
        for (task = 0; task < TASK_NUM; task++) {
            if (g_task_count[task] <= g_task_prev_count[task]) {
                g_testResult = 1;
                TEST_LOG_FMT("[sched_rr] error: task %d count unchange for 1 sec", task);
                goto EXIT;
            }
            TEST_LOG_FMT("[sched_rr] task %d, count:%llu", task, g_task_count[task]);
            g_task_prev_count[task] = g_task_count[task];
        }
#endif
    }

// 关闭RR功能后，只有第一个任务的计数在增长
#if !defined(OS_OPTION_RR_SCHED)
    if (!test_check_only_first_task_sched()) {
        g_testResult = 1;
        goto EXIT;
    }
#endif

EXIT:
    g_testFinish = 1;
    PRT_TaskDelay(OS_TICK_PER_SECOND / 10);
    return g_testResult;
}

/**
 * 10个同优先级任务，倒数第二个是FIFO任务，每个任务在循环中加计数
 * 前8个任务计数只在第一秒有增长，FIFO任务计数每秒都在增长，最后一个任务计数永远是0，测10秒出结果
 * （关闭RR功能后，只有第一个任务的计数在增长）
*/
static int test_sched_rr_fifo(void)
{
    int task, sec;
    g_testFinish = 0;
    for (task = 0; task < TASK_NUM; task++) {
        g_task_prev_count[task] = 0;
        g_task_count[task] = 0;
        if (task == 8) {
            test_start_task_param(test_sched_rr_tsk, 26, OS_TSK_SCHED_FIFO, task, 0, 0, 0);
        } else {
            test_start_task_param(test_sched_rr_tsk, 26, OS_TSK_SCHED_RR, task, 0, 0, 0);
        }
    }
    PRT_TaskDelay(OS_TICK_PER_SECOND);
    TEST_LOG("[sched_rr_fifo] 1 seconds pass");
#if defined(OS_OPTION_RR_SCHED)
    // 第一秒，除了最后一个任务所有任务计数都在增长
    for (task = 0; task <= 8; task++) {
        if (g_task_count[task] == 0) {
            TEST_LOG_FMT("[sched_rr_fifo] error: task %d count unchange for 1 sec", task);
            goto EXIT;
        }
        g_task_prev_count[task] = g_task_count[task];
    }
    if (g_task_count[9] != 0) {
        TEST_LOG("[sched_rr_fifo] error: task 9 count changed");
        goto EXIT;
    }
    g_task_prev_count[9] = g_task_count[9];
#endif

    // 剩余时间，只有FIFO任务在增长
    for (sec = 1; sec < 10; sec++) {
        PRT_TaskDelay(OS_TICK_PER_SECOND);
        TEST_LOG_FMT("[sched_rr_fifo] %d seconds pass", sec + 1);
#if defined(OS_OPTION_RR_SCHED)
        for (task = 0; task < TASK_NUM; task++) {
            if (task == 8 && g_task_count[task] <= g_task_prev_count[task]) {
                g_testResult = 1;
                TEST_LOG("[sched_rr_fifo] error: task 8 count unchange for 1 sec");
                goto EXIT;
            } else if (task != 8 && g_task_count[task] != g_task_prev_count[task]) {
                g_testResult = 1;
                TEST_LOG_FMT("[sched_rr_fifo] error: task %d count changed", task);
                goto EXIT;
            }
            TEST_LOG_FMT("[sched_rr_fifo] task %d, count:%llu", task, g_task_count[task]);
            g_task_prev_count[task] = g_task_count[task];
        }
#endif
    }

// 关闭RR功能后，只有第一个任务的计数在增长
#if !defined(OS_OPTION_RR_SCHED)
    if (!test_check_only_first_task_sched()) {
        g_testResult = 1;
        goto EXIT;
    }
#endif

EXIT:
    g_testFinish = 1;
    PRT_TaskDelay(OS_TICK_PER_SECOND / 10);
    return g_testResult;
}

#define TEST_REQUIRE 10000
volatile int g_test_count = 0;
volatile U64 g_cycle_before = 0;
volatile U64 g_cycle_after = 0;
volatile U32 g_test_elapse_start = 0;
U32 g_time_consume[TEST_REQUIRE] = {0};

static void test_sched_elapse_tsk(uintptr_t index, uintptr_t arg2, uintptr_t arg3, uintptr_t arg4)
{
    uintptr_t intSave;
    (void)arg2;
    (void)arg3;
    (void)arg4;
    int local_count = 0;
    TEST_LOG_FMT("[sched_elapse_tsk] task %d enter", (int)index);
    // 关中断，避免中断干扰
    intSave = PRT_HwiLock();
    while(!g_testFinish && g_test_count < TEST_REQUIRE) {
        // 必定触发任务切换
        OsTskReadyDel(RUNNING_TASK);
        OsTskReadyAdd(RUNNING_TASK);
        // 第一圈，所有任务得到调度后先关中断，并固定触发任务切换，不会记录耗时
        g_cycle_before = PRT_ClkGetCycleCount64();
        OsTskSchedule();
        // 从第二圈开始，任务代码走到此处，开始记录耗时
        g_cycle_after = PRT_ClkGetCycleCount64();
        if (g_cycle_after <= g_cycle_before) {
            PRT_HwiRestore(intSave);
            TEST_IF_ERR_RET_VOID_FMT(1, "[sched_elapse_tsk] task %d, cycle before:%llu, cycle after:%llu, local cnt:%d",
                (int)index, g_cycle_before, g_cycle_after, local_count);
        }
        // 采集数据可能已经足够
        if (g_test_count >= TEST_REQUIRE) {
            break;
        }
        g_time_consume[g_test_count] = (U32)(g_cycle_after - g_cycle_before);
        g_test_count++;
        local_count++;
    }
    PRT_HwiRestore(intSave);
    g_testFinish = 1;
    TEST_LOG_FMT("[sched_elapse_tsk] task %d exit, local cnt:%d", (int)index, local_count);
    return;
}

/* 测量任务切换耗时，任务调度前测量一次时间，任务得到调度后计算耗时，10个RR任务不停切换，直到测量数据充足 */
static int test_sched_elapse(void)
{
    U32 min = U32_MAX;
    U32 max = 0;
    U32 sum = 0;
    U32 curr;
    g_testFinish = 0;
    g_testResult = 0;
    g_test_elapse_start = 0;
    for (int task = 0; task < TASK_NUM; task++) {
        test_start_task_param(test_sched_elapse_tsk, 26, OS_TSK_SCHED_RR, task, 0, 0, 0);
    }

    // 主任务需要睡眠足够久，避免影响从任务的切换与计时
    PRT_TaskDelay(OS_TICK_PER_SECOND);
    if (!g_testFinish) {
        TEST_LOG("[sched_elapse_tsk] error: main tsk should delay longer");
        g_testResult = 1;
    }

    // 计算平均，最高，最低
    for (int i = 0; i < TEST_REQUIRE; i++) {
        curr = g_time_consume[i];
        if (curr < min) {
            min = curr;
        }
        if (curr > max) {
            max = curr;
        }
        sum += curr;
    }
    TEST_LOG_FMT("[sched_elapse] test finished, max:%u, min:%u, avg:%f", max, min, (sum * 1.0 / TEST_REQUIRE));
    return g_testResult;
}

#if defined(OS_OPTION_RR_SCHED)
SemHandle g_testSem;

static void test_sched_timeslice_update_tsk(void)
{
    uintptr_t intSave;
    U64 startTime, startTimePrev;
    U32 timeSlice, timeSlicePrev;

    startTimePrev = RUNNING_TASK->startTime;
    // 入队列后重新得到调度，timeslice重置，startTime更新
    PRT_TaskDelay(1);
    startTime = RUNNING_TASK->startTime;
    timeSlice = RUNNING_TASK->timeSlice;
    TEST_IF_ERR_RET_VOID((startTime <= startTimePrev), "[sched_timeslice_update] error: startTime not update error");
    TEST_IF_ERR_RET_VOID((timeSlice != g_timeSliceCycle), "[sched_timeslice_update] error: timeSlice not update error");

    intSave = PRT_HwiLock();
    startTimePrev = RUNNING_TASK->startTime;
    timeSlicePrev = RUNNING_TASK->timeSlice;
    // 自旋等待一个tick时间
    PRT_ClkDelayUs(OS_SYS_US_PER_SECOND / OS_TICK_PER_SECOND);
#if defined OS_OPTION_SMP
    OsTaskTrap((uintptr_t)RUNNING_TASK);
#else
    OsTaskTrap();
#endif
    startTime = RUNNING_TASK->startTime;
    timeSlice = RUNNING_TASK->timeSlice;
    PRT_HwiRestore(intSave);
    // 不修改队列，单纯调度，timeSlice，startTime更新
    TEST_IF_ERR_RET_VOID((startTime <= startTimePrev), "[sched_timeslice_update] error: startTime2 not update error");
    TEST_IF_ERR_RET_VOID((timeSlice >= timeSlicePrev), "[sched_timeslice_update] error: timeSlice2 not update error");

    intSave = PRT_HwiLock();
    startTimePrev = RUNNING_TASK->startTime;
    // 单纯入队列，timeSlice重置，startTime不变
    OsTskReadyDel(RUNNING_TASK);
    OsTskReadyAdd(RUNNING_TASK);
    startTime = RUNNING_TASK->startTime;
    timeSlice = RUNNING_TASK->timeSlice;
    PRT_HwiRestore(intSave);
    TEST_IF_ERR_RET_VOID((startTime != startTimePrev), "[sched_timeslice_update] error: startTime3 not update error");
    TEST_IF_ERR_RET_VOID((timeSlice != g_timeSliceCycle),
        "[sched_timeslice_update] error: timeSlice3 not update error");
    
    startTimePrev = startTime;
    RUNNING_TASK->timeSlice = 100;
    // 入队列后重新得到调度，timeslice重置，startTime更新
    PRT_SemPend(g_testSem, 1);
    startTime = RUNNING_TASK->startTime;
    timeSlice = RUNNING_TASK->timeSlice;
    TEST_IF_ERR_RET_VOID((startTime <= startTimePrev), "[sched_timeslice_update] error: startTime4 not update error");
    TEST_IF_ERR_RET_VOID((timeSlice != g_timeSliceCycle),
        "[sched_timeslice_update] error: timeSlice4 not update error");
    g_testFinish = 1;
    return;
}

/* 白盒测试，任务入队列后timeSlice重置，得到调度后startTime更新，timeSlice更新 */
static int test_sched_timeslice_update(void)
{
    g_testFinish = 0;
    g_testResult = 0;
    PRT_SemCreate(0, &g_testSem);
    test_start_task((TskEntryFunc)test_sched_timeslice_update_tsk, 24, OS_TSK_SCHED_RR);

    while(!g_testFinish) {
        PRT_TaskDelay(OS_TICK_PER_SECOND);
    }
    PRT_SemDelete(g_testSem);
    TEST_LOG("[sched_timeslice_update] test finished");
    return g_testResult;
}

// 耗时中断等待10分之一个tick的时间
#define TEST_IRQ_TIME_CONSUME   ((OS_SYS_US_PER_SECOND / OS_TICK_PER_SECOND) / 10)
volatile U32 g_irq_cycle_spend = 0;

#if !defined(OS_OPTION_SMP)
extern void OsHwiMcTrigger(U32 coreMask, U32 hwiNum);
#else
extern void OsHwiMcTrigger(enum OsHwiIpiType type, U32 coreMask, U32 hwiNum);
#endif

static void test_sched_irqTime_remove_tsk(void)
{
    U64 startTime, startTimePrev;
    U32 timeSlice, timeSlicePrev, timeSliceDiff, startTimeDiff;

    // task delay 保证timeSlice为最大值，并且一个tick中断刚好结束
    PRT_TaskDelay(1);
    startTimePrev = RUNNING_TASK->startTime;
    timeSlicePrev = RUNNING_TASK->timeSlice;
    // PRT_HwiTrigger(PRT_GetCoreID(), OS_HWI_IPI_NO_015);
#if !defined(OS_OPTION_SMP)
    OsHwiMcTrigger(0xf, OS_HWI_IPI_NO_015);
#else
    OsHwiMcTrigger(OS_TYPE_TRIGGER_TO_SELF, 0, OS_HWI_IPI_NO_015);
#endif
    // 中断可能会延迟数条指令后触发，此处需要略微等待
    PRT_ClkDelayUs(1);
    TEST_IF_ERR_RET_VOID((g_irq_cycle_spend == 0), "[sched_irqTime_remove] error: irq not trigger yet!");
    startTime = RUNNING_TASK->startTime;
    timeSlice = RUNNING_TASK->timeSlice;

    TEST_LOG_FMT("[sched_irqTime_remove] startTime:%llu->%llu, timeSlice:%u->%u",
        startTimePrev, startTime, timeSlicePrev, timeSlice);
    TEST_IF_ERR_RET_VOID((startTime <= startTimePrev), "[sched_irqTime_remove] error: startTime not update error");
    TEST_IF_ERR_RET_VOID((timeSlice >= timeSlicePrev), "[sched_irqTime_remove] error: timeSlice not update error");

    startTimeDiff = (U32)(startTime - startTimePrev);
    timeSliceDiff = (U32)(timeSlicePrev - timeSlice);

    TEST_IF_ERR_RET_VOID((startTimeDiff < timeSliceDiff), "[sched_irqTime_remove] error: timeSlice update invalid");
#if defined(OS_OPTION_RR_SCHED_IRQ_TIME_DISCOUNT)
    TEST_IF_ERR_RET_VOID((startTimeDiff - timeSliceDiff) < g_irq_cycle_spend,
        "[sched_irqTime_remove] error: irqTime not removed?")
#endif
    TEST_LOG_FMT("[sched_irqTime_remove] task finished, irq cycle spend:%u, non-task time:%u",
        g_irq_cycle_spend, startTimeDiff - timeSliceDiff);
    g_testFinish = 1;
    return;
}

/* 耗时中断 */
static void test_sched_irq_handler(void)
{
    U64 cycle = PRT_ClkGetCycleCount64();
    TEST_LOG("[sched_irqTime_remove] irq enter");
    PRT_ClkDelayUs(TEST_IRQ_TIME_CONSUME);
    g_irq_cycle_spend = (U32)(PRT_ClkGetCycleCount64() - cycle);
}

/* 白盒测试，中断耗时测试，任务运行时间不统计中断耗时 */
static int test_sched_irqTime_remove(void)
{
    g_testFinish = 0;
    g_testResult = 0;

    U32 ret = PRT_HwiSetAttr(OS_HWI_IPI_NO_015, 10, OS_HWI_MODE_ENGROSS);
    if (ret != OS_OK) {
        return ret;
    }
    ret = PRT_HwiCreate(OS_HWI_IPI_NO_015, (HwiProcFunc)test_sched_irq_handler, 0);
    if (ret != OS_OK) {
        return ret;
    }
    ret = PRT_HwiEnable(OS_HWI_IPI_NO_015);
    if (ret != OS_OK) {
        return ret;
    }

    test_start_task((TskEntryFunc)test_sched_irqTime_remove_tsk, 26, OS_TSK_SCHED_RR);

    // 1秒应该完全充足
    PRT_TaskDelay(OS_TICK_PER_SECOND);
    if (!g_testFinish) {
        // 等待时间不充足
        g_testResult = 1;
    }
    TEST_LOG("[sched_irqTime_remove] test finished");

    return g_testResult;
}
#endif

test_case_t g_cases[] = {
    TEST_CASE_Y(test_sched_rr),
    TEST_CASE_Y(test_sched_rr_fifo),
    TEST_CASE_Y(test_sched_elapse),
#if defined(OS_OPTION_RR_SCHED)
    TEST_CASE_Y(test_sched_timeslice_update),
    TEST_CASE_Y(test_sched_irqTime_remove),
#endif
};

int g_test_case_size = sizeof(g_cases);

void prt_kern_test_end()
{
    TEST_LOG("Round Robin Shcedule test finished\n");
}
