#include <stdio.h>
#include <stdint.h>

int uniproton_testapi(int argc, char **argv);
int uniproton_testchar(void);
int uniproton_testdict(void);
int uniproton_testrecurse(int argc, char **argv);

static int uniproton_testchar_arg(int argc, char **argv) {
    return uniproton_testchar();
}

static int uniproton_testdict_arg(int argc, char **argv) {
    return uniproton_testdict();
}

int uniproton_runtest(int argc, char **argv);

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
    TEST_CASE_Y(uniproton_runtest),
    TEST_CASE_Y(uniproton_testrecurse),
    TEST_CASE_Y(uniproton_testchar_arg),
    TEST_CASE_Y(uniproton_testdict_arg),
    TEST_CASE_Y(uniproton_testapi),
};

int xml2_test_entry()
{
    int i = 0;
    int cnt = 0;
    int fails = 0;
    int len = sizeof(g_cases) / sizeof(test_case_t);
    printf("\n===test start=== \n");
    for (i = 0, cnt = 0; i < len; i++) {
        test_case_t *tc = &g_cases[i];
        printf("\n===%s start === \n", tc->name);
        if (tc->skip) {
            continue;
        }
        cnt++;
        int ret = tc->fn(0, NULL);
        if (ret) {
            fails++;
            printf("\n===%s failed ===\n", tc->name);
        } else {
            printf("\n===%s success ===\n", tc->name);
        }
    }
    printf("\n===test end, total: %d, fails:%d ===\n", cnt, fails);
    return cnt;
}

void Init(uintptr_t param1, uintptr_t param2, uintptr_t param3, uintptr_t param4)
{
    (void)(param1);
    (void)(param2);
    (void)(param3);
    (void)(param4);
    xml2_test_entry();
}