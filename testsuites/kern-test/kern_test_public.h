#ifndef KERN_TEST_PUBLIC_H
#define KERN_TEST_PUBLIC_H
#include "prt_buildef.h"
#include "prt_task.h"
#include "prt_log.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

extern volatile U32 g_testFinish;
extern volatile int g_testResult;

// printf接口内部有调用pthread_mutex锁可能影响测试结果，此处使用prt_log相关接口

#define TEST_IF_ERR_RET(ret, str)  \
    if (ret) {                                                  \
        g_testResult = (int)(ret);                              \
        g_testFinish = 1;                                       \
        PRT_Log(OS_LOG_INFO, OS_LOG_F1, str, sizeof(str) - 1);  \
        return g_testResult;                                    \
    }

#define TEST_IF_ERR_RET_VOID(ret, str)  \
    if (ret) {                                                  \
        g_testResult = (int)(ret);                              \
        g_testFinish = 1;                                       \
        PRT_Log(OS_LOG_INFO, OS_LOG_F1, str, sizeof(str) - 1);  \
        return;                                                 \
    }

#define TEST_IF_ERR_RET_VOID_FMT(ret, fmt, ...)  \
    if (ret) {                                                      \
        g_testResult = (int)(ret);                                  \
        g_testFinish = 1;                                           \
        PRT_LogFormat(OS_LOG_INFO, OS_LOG_F1, fmt, __VA_ARGS__);    \
        return;                                                     \
    }

#define TEST_LOG(str) PRT_Log(OS_LOG_INFO, OS_LOG_F1, str, sizeof(str) - 1)

#define TEST_LOG_FMT(fmt, ...) PRT_LogFormat(OS_LOG_INFO, OS_LOG_F1, fmt, __VA_ARGS__)

typedef int (*test_fn)(void);

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

extern test_case_t g_cases[];

extern int g_test_case_size;

extern void prt_kern_test_end();
extern TskHandle test_start_task(TskEntryFunc func, TskPrior taskPrio, U16 policy);
extern TskHandle test_start_task_param(TskEntryFunc func, TskPrior taskPrio, U16 policy, uintptr_t arg1,
    uintptr_t arg2, uintptr_t arg3, uintptr_t arg4);
#if defined(OS_OPTION_OPENAMP)
unsigned int is_tty_ready(void);
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#endif /* KERN_TEST_PUBLIC_H */
