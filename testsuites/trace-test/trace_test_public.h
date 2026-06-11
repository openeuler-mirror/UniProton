#ifndef TRACE_TEST_PUBLIC_H
#define TRACE_TEST_PUBLIC_H

#include "prt_buildef.h"
#include "prt_task.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

extern volatile U32 g_testFinish;
extern volatile int g_traceTestResult;

extern U32 PRT_Printf(const char *format, ...);

#define TEST_IF_ERR_RET(ret, str)  \
    if (ret) {                                                  \
        g_traceTestResult = (int)(ret);                         \
        g_testFinish = 1;                                       \
        PRT_Printf("%s\n", str);                                \
        return g_traceTestResult;                               \
    }

#define TEST_LOG(str) PRT_Printf("%s", str)

#define TEST_LOG_FMT(fmt, ...) PRT_Printf(fmt, ##__VA_ARGS__)

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

extern void prt_kern_test_end(void);
extern TskHandle test_start_task(TskEntryFunc func, TskPrior taskPrio, U16 policy);
extern TskHandle test_start_task_param(TskEntryFunc func, TskPrior taskPrio, U16 policy, uintptr_t arg1,
    uintptr_t arg2, uintptr_t arg3, uintptr_t arg4);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* TRACE_TEST_PUBLIC_H */
