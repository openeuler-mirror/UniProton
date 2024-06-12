
#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include "prt_config.h"
#include "prt_log.h"
#include "prt_task.h"
#include "prt_mem.h"
#include "prt_sys.h"
#include "prt_config.h"
#include "securec.h"

// 后续考虑接口耗时测试(需要cycle)，最高速率测试 (不同环境最高速率不同）

/* 输出日志耗时 (LOOP_COUNT / 10) 秒 */
/* 输出日志速率 每秒 SINGLE_OUTPUT * 10 个 */
/* 输出日志总量 LOOP_COUNT * SINGLE_OUTPUT */
#define LOOP_COUNT 100UL
#define SINGLE_OUTPUT 1UL
#define BUFFER_BLOCK_SIZE 0x400UL /* 1KB */

#define ASSERT_EQ(a, b)     \
    if (a != b) {           \
        return -1;          \
    }                       \

extern volatile U64 g_uniTicks;
volatile U32 g_testFinish = 0;

#if defined(OS_OPTION_OPENAMP)
unsigned int is_tty_ready(void);
#endif

static int logEmit(enum OsLogLevel level, enum OsLogFacility facility, const char *format, ...)
{
    int len;
    va_list vaList;
    char buff[BUFFER_BLOCK_SIZE];
    memset_s(buff, BUFFER_BLOCK_SIZE, 0, BUFFER_BLOCK_SIZE);
    va_start(vaList, format);
    // 字符串格式化由用户负责
    len = vsnprintf(buff, BUFFER_BLOCK_SIZE, format, vaList);
    va_end(vaList);
    if (len < 0) {
        printf("vsn len < 0, %d\n", len);
        return len;
    }

    if (PRT_Log(level, facility, buff, len)) {
        printf("log emit fail\n");
        return -1;
    }
    return 0;
}

static int test_log()
{
    U64 startTick, endTick;

    while (!PRT_IsLogInit()) {
        PRT_TaskDelay(OS_TICK_PER_SECOND / 100);
    };

    startTick = g_uniTicks;
    for (int j = 0; j < LOOP_COUNT; j++) {
        for (int i = 0; i < SINGLE_OUTPUT; i++) {
            logEmit(OS_LOG_INFO, OS_LOG_F1, "test %d", LOOP_COUNT * SINGLE_OUTPUT - (i + j * SINGLE_OUTPUT));
        }
        // 升腾最小 delay 10ms, 每 SINGLE_OUTPUT 个 日志隔 100ms
        PRT_TaskDelay(OS_TICK_PER_SECOND / 10);
    }
    endTick = g_uniTicks;
    printf("log test, %lu logs, take %llu ms, \n", LOOP_COUNT * SINGLE_OUTPUT,
        ((endTick - startTick) * 1000) / OS_TICK_PER_SECOND);
    return 0;
}

static void test_log_thread()
{
    U64 startTick, endTick;
    TskHandle taskPid = 0;
    U32 core_id = PRT_GetCoreID();
    PRT_TaskSelf(&taskPid);

    while (!PRT_IsLogInit()) {
        PRT_TaskDelay(OS_TICK_PER_SECOND / 100);
    };

    startTick = g_uniTicks;
    /* 每个线程每秒输出 SINGLE_OUTPUT 个日志, 总共输出 (LOOP_COUNT * SINGLE_OUTPUT / 10) 个日志 */ 
#if SINGLE_OUTPUT > 10UL
    for (int j = 0; j < LOOP_COUNT; j++) {

        for (int i = 0; i < SINGLE_OUTPUT / 10; i++) {
            logEmit(OS_LOG_INFO, OS_LOG_F1, "[core:%u] test %d\n", core_id,
                LOOP_COUNT * SINGLE_OUTPUT / 10 - (i + j * (SINGLE_OUTPUT / 10)));
        }
        PRT_TaskDelay(OS_TICK_PER_SECOND / 10);
    }
#else
    for (int j = 0; j < LOOP_COUNT / 10; j++) {
        for (int i = 0; i < SINGLE_OUTPUT; i++) {
            logEmit(OS_LOG_INFO, OS_LOG_F1, "[core:%u] test %d\n", core_id,
                LOOP_COUNT * SINGLE_OUTPUT / 10 - (i + j * (SINGLE_OUTPUT)));
        }
        PRT_TaskDelay(OS_TICK_PER_SECOND);
    }
#endif
    endTick = g_uniTicks;
    printf("log test core_id:%u, pid:%u, %lu logs, take %llu ms, \n", core_id, taskPid, LOOP_COUNT * SINGLE_OUTPUT / 10,
        ((endTick - startTick) * 1000) / OS_TICK_PER_SECOND);
    g_testFinish = 1;
}

// 多核 多线程写入测试
int multi_thread_test_log(TskEntryFunc func)
{
    U32 ret;
    struct TskInitParam param = {0};
    TskHandle testTskHandle;

    PRT_TaskDelay(OS_TICK_PER_SECOND / 10);
    g_testFinish = 0;

    /* 10个线程同时输出日志, 每个核5个线程 */
    for (int i = 0; i < 5; i++) {
        param.stackAddr = (uintptr_t)PRT_MemAllocAlign(0, OS_MEM_DEFAULT_FSC_PT, 0x3000, MEM_ADDR_ALIGN_016);
        param.taskEntry = func;
        // 支持的优先级(0~31)
        param.taskPrio = 26 + i;
        param.name = "testThread";
        param.stackSize = 0x3000;

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
        printf("Create task:%d, prio:%d\n", testTskHandle, param.taskPrio);
    }

    while (g_testFinish == 0) {
        PRT_TaskDelay(OS_TICK_PER_SECOND / 10);
    }

    return 0;
}

static void test_log_filter_thread()
{
    U64 startTick, endTick;
    U32 ret;
    TskHandle taskPid = 0;
    U32 level = 0;
    U32 core_id = PRT_GetCoreID();
    PRT_TaskSelf(&taskPid);

    while (!PRT_IsLogInit()) {
        PRT_TaskDelay(OS_TICK_PER_SECOND / 100);
    };

    startTick = g_uniTicks;
    // 未设置过滤
    for (level = (U32)OS_LOG_EMERG; level < (U32)OS_LOG_NONE; level++) {
        logEmit((enum OsLogLevel)level, OS_LOG_F1, "[core:%u] level:%d, first\n", core_id, level);
    }
    PRT_TaskDelay(OS_TICK_PER_SECOND);

    // 设置 alert 级别
    ret = PRT_LogSetFilter(OS_LOG_ALERT);
    if (ret != OS_OK) {
        goto ERR_EXIT;
    }
    for (level = (U32)OS_LOG_EMERG; level < (U32)OS_LOG_NONE; level++) {
        logEmit((enum OsLogLevel)level, OS_LOG_F1, "[core:%u] level:%d, second\n", core_id, level);
    }
    PRT_TaskDelay(OS_TICK_PER_SECOND);

    // 设置 info 级别
    ret = PRT_LogSetFilter(OS_LOG_INFO);
    if (ret != OS_OK) {
        goto ERR_EXIT;
    }
    for (level = (U32)OS_LOG_EMERG; level < (U32)OS_LOG_NONE; level++) {
        logEmit((enum OsLogLevel)level, OS_LOG_F1, "[core:%u] level:%d, thrid\n", core_id, level);
    }
    PRT_TaskDelay(OS_TICK_PER_SECOND);

    // 设置 F1 的 warn 级别
    ret = PRT_LogSetFilterByFacility(OS_LOG_F1, OS_LOG_WARN);
    if (ret != OS_OK) {
        goto ERR_EXIT;
    }
    for (level = (U32)OS_LOG_ERR; level < (U32)OS_LOG_NONE; level++) {
        logEmit((enum OsLogLevel)level, OS_LOG_F1, "[core:%u] level:%d, F1\n", core_id, level);
    }
    for (level = (U32)OS_LOG_ERR; level < (U32)OS_LOG_NONE; level++) {
        logEmit((enum OsLogLevel)level, OS_LOG_F2, "[core:%u] level:%d, F2\n", core_id, level);
    }
    PRT_TaskDelay(OS_TICK_PER_SECOND);

    (void)PRT_Log(OS_LOG_EMERG, OS_LOG_F1, "log pos5", 8);

    // 设置 NONE 级别, 取消过滤
    ret = PRT_LogSetFilter(OS_LOG_NONE);
    if (ret != OS_OK) {
        goto ERR_EXIT;
    }
    for (level = OS_LOG_EMERG; level < OS_LOG_NONE; level++) {
        logEmit(level, OS_LOG_F1, "[core:%u] level:%d, fourth\n", core_id, level);
    }
    endTick = g_uniTicks;
    printf("log filter test finish, core_id:%u, pid:%u take %llu ms, \n", core_id, taskPid,
            ((endTick - startTick) * 1000) / OS_TICK_PER_SECOND);
    g_testFinish = 1;
    return;

ERR_EXIT:
    printf("log filter test err, core_id:%u, pid:%u, ret:%d\n", core_id, taskPid, ret);
    return;
}

static void test_log_switch_thread()
{
    U32 core_id = PRT_GetCoreID();
    TskHandle taskPid = 0;
    PRT_TaskSelf(&taskPid);

    // 正常应该已经初始化了
    while (!PRT_IsLogInit()) {
        PRT_TaskDelay(OS_TICK_PER_SECOND / 100);
    };

    logEmit(OS_LOG_INFO, OS_LOG_F1, "[core:%u] log switch enter\n", core_id);

    PRT_LogOn();
    logEmit(OS_LOG_INFO, OS_LOG_F1, "[core:%u] log on\n", core_id);
    PRT_TaskDelay(OS_TICK_PER_SECOND / 10);

    PRT_LogOff();
    logEmit(OS_LOG_INFO, OS_LOG_F1, "[core:%u] log off\n", core_id);
    PRT_TaskDelay(OS_TICK_PER_SECOND / 10);

    PRT_LogOn();
    logEmit(OS_LOG_INFO, OS_LOG_F1, "[core:%u] log on again\n", core_id);
    PRT_TaskDelay(OS_TICK_PER_SECOND / 10);

    g_testFinish = 1;
}

int log_simple_test()
{
    return multi_thread_test_log((TskEntryFunc)test_log_thread);
}

int log_filter_test()
{
    return multi_thread_test_log((TskEntryFunc)test_log_filter_thread);
}

int log_switch_test()
{
    return multi_thread_test_log((TskEntryFunc)test_log_switch_thread);
}

int log_interface_test()
{
    U32 ret;
    char buff[BUFFER_BLOCK_SIZE];
    // 只需要主核测试
    if (PRT_GetCoreID() != OS_SYS_CORE_PRIMARY) {
        return 0;
    }

    // 正常应该已经初始化了
    while (!PRT_IsLogInit()) {
        PRT_TaskDelay(OS_TICK_PER_SECOND / 100);
    };

    // PRT_LogInit 重复初始化不会报错
    ret = PRT_LogInit(12345);
    ASSERT_EQ(ret, 0);

    // PRT_Log 接口测试
    ret = PRT_Log(OS_LOG_NONE, OS_LOG_F1, "test", 4);
    ASSERT_EQ(ret, -1);
    ret = PRT_Log(OS_LOG_NONE, 0, "test", 4);
    ASSERT_EQ(ret, -1);
    ret = PRT_Log(OS_LOG_NONE, OS_LOG_F1, NULL, 4);
    ASSERT_EQ(ret, -1);
    ret = PRT_Log(OS_LOG_NONE, OS_LOG_F1, "test", 0);
    ASSERT_EQ(ret, -1);

    // PRT_LogSetFilter 接口测试
    ret = PRT_LogSetFilter(OS_LOG_NONE);
    ASSERT_EQ(ret, 0);
    ret = PRT_LogSetFilter(OS_LOG_NONE + 1);
    ASSERT_EQ(ret, -1);

    // PRT_LogSetFilterByFacility 接口测试
    ret = PRT_LogSetFilterByFacility(OS_LOG_F0, OS_LOG_NONE);
    ASSERT_EQ(ret, 0);
    ret = PRT_LogSetFilterByFacility(OS_LOG_F0, OS_LOG_NONE + 1);
    ASSERT_EQ(ret, -1);
    ret = PRT_LogSetFilterByFacility(0, OS_LOG_NONE);
    ASSERT_EQ(ret, -1)

    // PRT_Log 超长日志
    (void)PRT_Log(OS_LOG_INFO, OS_LOG_F1, "long log start", 14);
    memset_s(buff, BUFFER_BLOCK_SIZE / 2, 'a', BUFFER_BLOCK_SIZE / 2);
    memset_s(buff + BUFFER_BLOCK_SIZE / 2, BUFFER_BLOCK_SIZE / 2, 'b', BUFFER_BLOCK_SIZE / 2);
    ret = PRT_Log(OS_LOG_INFO, OS_LOG_F1, buff, BUFFER_BLOCK_SIZE);
    ASSERT_EQ(ret, 0);
    ret = PRT_Log(OS_LOG_INFO, OS_LOG_F1, buff, BUFFER_BLOCK_SIZE);
    ASSERT_EQ(ret, 0);
    ret = PRT_Log(OS_LOG_INFO, OS_LOG_F1, buff, BUFFER_BLOCK_SIZE);
    ASSERT_EQ(ret, 0);
    (void)PRT_Log(OS_LOG_INFO, OS_LOG_F1, "finish", 6);
    return 0;
}

int log_format_test()
{
    U32 ret;
    char buff[BUFFER_BLOCK_SIZE];
    U32 core_id = PRT_GetCoreID();

    // PRT_LogFormat 接口测试
    ret = PRT_LogFormat(OS_LOG_NONE, OS_LOG_F1, "test %d", 1);
    ASSERT_EQ(ret, -1);
    ret = PRT_LogFormat(OS_LOG_NONE, 0, "test %d", 1);
    ASSERT_EQ(ret, -1);
    ret = PRT_LogFormat(OS_LOG_NONE, OS_LOG_F1, "test %d", 1);
    ASSERT_EQ(ret, -1);
    ret = PRT_LogFormat(OS_LOG_NONE, OS_LOG_F1, "test %d", 1);
    ASSERT_EQ(ret, -1);

    (void)PRT_LogFormat(OS_LOG_INFO, OS_LOG_F1, "[core:%u] format normal test level:%d facility:%d", 
        core_id, OS_LOG_INFO, OS_LOG_F1);

    // PRT_Log 超长日志
    (void)PRT_LogFormat(OS_LOG_INFO, OS_LOG_F1, "[core:%u] formate long log start", core_id);
    memset_s(buff, BUFFER_BLOCK_SIZE / 2, 'a', BUFFER_BLOCK_SIZE / 2);
    memset_s(buff + BUFFER_BLOCK_SIZE / 2, BUFFER_BLOCK_SIZE / 2, 'b', BUFFER_BLOCK_SIZE / 2);
    buff[BUFFER_BLOCK_SIZE - 1] = 0;
    ret = PRT_LogFormat(OS_LOG_INFO, OS_LOG_F1, buff);
    ASSERT_EQ(ret, 0);
    ret = PRT_LogFormat(OS_LOG_INFO, OS_LOG_F1, buff);
    ASSERT_EQ(ret, 0);
    ret = PRT_LogFormat(OS_LOG_INFO, OS_LOG_F1, buff);
    ASSERT_EQ(ret, 0);
    (void)PRT_LogFormat(OS_LOG_INFO, OS_LOG_F1, "formate test finish");
    return 0;
}

int log_error_trigger()
{
    U32 ret;
    U8 tmp;
    U8 *addr = 0x0;
    U32 core_id = PRT_GetCoreID();
    // 只挂从核，主核正常运行
    if (core_id == OS_SYS_CORE_PRIMARY) {
        return 0;
    }

    ret = PRT_LogFormat(OS_LOG_INFO, OS_LOG_F1, "[core:%u] error test start", core_id);
    ASSERT_EQ(ret, 0);

    tmp = *addr;

    ret = PRT_Log(OS_LOG_WARN, OS_LOG_F1, "should not be here", 18);
    ASSERT_EQ(ret, 0);

    printf("should not be here\n");
    return tmp;
}

typedef int (*test_fn)(int argc, char **argv);

typedef struct test_case {
    char *name;
    test_fn fn;
    int skip;
} test_case_t;

#define TEST_CASE(func, s) {         \
    .name = #func,                   \
    .fn = func,                      \
    .skip = s                        \
}

#define TEST_CASE_Y(func) TEST_CASE(func, 0)
#define TEST_CASE_N(func) TEST_CASE(func, 1)

static test_case_t g_cases[] = {
    TEST_CASE_N(test_log),
    TEST_CASE_Y(log_simple_test),
    TEST_CASE_Y(log_filter_test),
    TEST_CASE_Y(log_switch_test),
    TEST_CASE_Y(log_format_test),
    TEST_CASE_Y(log_interface_test),
    TEST_CASE_N(log_error_trigger),
};

static void log_test_entry()
{
    int i = 0;
    int cnt = 0;
    int fails = 0;
    int len = sizeof(g_cases) / sizeof(test_case_t);
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
        int ret = tc->fn(0, NULL);
        if (ret) {
            fails++;
            printf("\r\n===%s failed ===\r\n", tc->name);
        } else {
            printf("\r\n===%s success ===\r\n", tc->name);
        }
    }
    printf("\r\n===test end, total: %d, fails:%d ===\r\n", cnt, fails);
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
    param.taskEntry = (TskEntryFunc)log_test_entry;
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