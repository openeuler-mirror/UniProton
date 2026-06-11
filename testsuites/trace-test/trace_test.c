#include "prt_config.h"
#include "prt_task.h"
#include "prt_mem.h"
#include "prt_trace_external.h"
#include "securec.h"
#include "trace_test_public.h"

volatile U32 g_testFinish = 0;
volatile int g_traceTestResult = 0;

#ifdef OS_OPTION_TRACE

#define TRACE_TEST_TASK_STACK_SIZE 0x800
#define TRACE_TEST_TASK_PRIORITY   20

static U32 g_traceTestTaskId;

static bool TestHwiFilter(U32 hwiNum)
{
    return (hwiNum == 1);
}

static void TraceTestTaskEntry(uintptr_t arg1, uintptr_t arg2, uintptr_t arg3, uintptr_t arg4)
{
    (void)arg1; (void)arg2; (void)arg3; (void)arg4;
    U32 ret;
    TraceOfflineHead *head;

    TEST_LOG("[trace] === Trace Feature Test Start ===\r\n");

    TEST_LOG("[trace] Test 1: Trace Start\r\n");
    ret = PRT_TraceStart();
    if (ret != OS_OK) {
        TEST_LOG("[trace] Trace start returned error (may be auto-initialized)\r\n");
    } else {
        TEST_LOG("[trace] Trace is running\r\n");
    }

    TEST_LOG("[trace] Test 2: Set event mask to all modules\r\n");
    PRT_TraceEventMaskSet(0xFFFFFFFF);
    TEST_LOG("[trace] Event mask set\r\n");

    TEST_LOG("[trace] Test 3: Use PRT_TRACE_EASY macro\r\n");
    PRT_TRACE_EASY(0x01, 0x12345678, 0x01, 0x02);
    PRT_TRACE_EASY(0x02, 0xABCDEFFF, 0x10, 0x20, 0x30);
    TEST_LOG("[trace] PRT_TRACE_EASY events recorded\r\n");

    TEST_LOG("[trace] Test 4: Use PRT_TRACE macro\r\n");
    PRT_TRACE(HWI_RESPONSE_IN, 55);
    PRT_TRACE(HWI_RESPONSE_OUT, 55);
    PRT_TRACE(TASK_CREATE, 1, 0x04, 10);
    PRT_TRACE(TASK_DELETE, 2, 0x02, 0);
    PRT_TRACE(SEM_CREATE, 0, 0, 1);
    PRT_TRACE(QUEUE_CREATE, 0, 10, 8, 0, 0);
    PRT_TRACE(MEM_ALLOC, 0x1000, 0x2000, 128);
    PRT_TRACE(SYS_ERROR, 0x1234);
    TEST_LOG("[trace] PRT_TRACE events recorded\r\n");

    TEST_LOG("[trace] Test 5: Get trace record\r\n");
    head = PRT_TraceRecordGet();
    if (head != NULL) {
        TEST_LOG("[trace] Trace buffer acquired OK\r\n");
        TEST_LOG_FMT("[trace] clockFreq = %u, version = %u, totalLen = %u\r\n",
            head->baseInfo.clockFreq, head->baseInfo.version, head->totalLen);
        TEST_LOG_FMT("[trace] objSize = %u, frameSize = %u, objOffset = %u, frameOffset = %u\r\n",
            head->objSize, head->frameSize, head->objOffset, head->frameOffset);
    } else {
        TEST_LOG("[trace] Trace buffer is NULL - FAIL\r\n");
        g_traceTestResult |= 1;
    }

    TEST_LOG("[trace] Test 6: Stop trace\r\n");
    PRT_TraceStop();
    TEST_LOG("[trace] Trace stopped\r\n");

    TEST_LOG("[trace] Test 7: Dump trace records\r\n");
    PRT_TraceRecordDump(FALSE);
    TEST_LOG("[trace] Trace dump completed\r\n");

    TEST_LOG("[trace] Test 8: Reset trace buffer\r\n");
    PRT_TraceReset();
    TEST_LOG("[trace] Trace buffer reset\r\n");

    TEST_LOG("[trace] Test 9: Restart trace\r\n");
    ret = PRT_TraceStart();
    if (ret == OS_OK) {
        TEST_LOG("[trace] Trace restarted OK\r\n");
        PRT_TRACE_EASY(0x03, 0x9999, 0x11, 0x22, 0x33);
        PRT_TraceStop();
        PRT_TraceRecordDump(FALSE);
    } else {
        TEST_LOG_FMT("[trace] Trace restart failed: %u\r\n", ret);
        g_traceTestResult |= 2;
    }

    TEST_LOG("[trace] Test 10: HWI filter hook\r\n");
    PRT_TraceHwiFilterHookReg(TestHwiFilter);
    TEST_LOG("[trace] HWI filter hook registered\r\n");

    if (g_traceTestResult == 0) {
        TEST_LOG("[trace] === ALL TRACE TESTS PASSED ===\r\n");
    } else {
        TEST_LOG_FMT("[trace] === TRACE TESTS FAILED (result=%u) ===\r\n", g_traceTestResult);
    }

    g_testFinish = 1;
}

static int trace_test_start(void)
{
    U32 ret;
    struct TskInitParam param = {0};

    g_testFinish = 0;
    g_traceTestResult = 0;

    param.taskEntry = (TskEntryFunc)TraceTestTaskEntry;
    param.taskPrio = TRACE_TEST_TASK_PRIORITY;
    param.stackSize = TRACE_TEST_TASK_STACK_SIZE;
    param.name = "TraceTest";

    ret = PRT_TaskCreate(&g_traceTestTaskId, &param);
    TEST_IF_ERR_RET(ret, "[ERROR] TraceTest task create failed");

    ret = PRT_TaskResume(g_traceTestTaskId);
    TEST_IF_ERR_RET(ret, "[ERROR] TraceTest task resume failed");

    while (g_testFinish == 0) {
        PRT_TaskDelay(OS_TICK_PER_SECOND / 10);
    }

    PRT_TaskDelete(g_traceTestTaskId);
    g_traceTestTaskId = -1;

    return g_traceTestResult;
}

test_case_t g_cases[] = {
    TEST_CASE_Y(trace_test_start),
};

int g_test_case_size = sizeof(g_cases);

void prt_kern_test_end(void)
{
    TEST_LOG("[trace] Trace test finished\r\n");
}

#else /* !OS_OPTION_TRACE */

test_case_t g_cases[] = {};
int g_test_case_size = 0;

void prt_kern_test_end(void)
{
    PRT_Printf("[trace] Trace feature not enabled\r\n");
}

#endif /* OS_OPTION_TRACE */

static void trace_test_entry(void)
{
    int i = 0;
    int cnt = 0;
    int fails = 0;
    int len = g_test_case_size / sizeof(test_case_t);

    TEST_LOG("[trace] === test start ===\r\n");
    for (i = 0, cnt = 0; i < len; i++) {
        test_case_t *tc = &g_cases[i];
        if (tc->skip) {
            TEST_LOG_FMT("[trace] === %s skip ===\r\n", tc->name);
            continue;
        }
        TEST_LOG_FMT("[trace] === %s start ===\r\n", tc->name);
        cnt++;
        int ret = tc->fn();
        if (ret) {
            fails++;
            TEST_LOG_FMT("[trace] === %s failed ===\r\n", tc->name);
        } else {
            TEST_LOG_FMT("[trace] === %s success ===\r\n", tc->name);
        }
    }
    TEST_LOG_FMT("[trace] === test end, total: %d, fails: %d ===\r\n", cnt, fails);
    prt_kern_test_end();
}

void Init(uintptr_t param1, uintptr_t param2, uintptr_t param3, uintptr_t param4)
{
    U32 ret;
    struct TskInitParam param = {0};
    TskHandle testTskHandle;
    (void)param1;
    (void)param2;
    (void)param3;
    (void)param4;

    param.stackAddr = (uintptr_t)PRT_MemAllocAlign(0, OS_MEM_DEFAULT_FSC_PT, 0x3000, MEM_ADDR_ALIGN_016);
    param.taskEntry = (TskEntryFunc)trace_test_entry;
    param.taskPrio = 25;
    param.name = "traceTestInit";
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
