#ifndef _CONFORMSNCE_RUN_TEST_H
#define _CONFORMSNCE_RUN_TEST_H

extern int TEST_sigaction_1();
extern int TEST_sigaction_2();
extern int TEST_sigaction_3();
extern int TEST_sigpromask_1();
extern int TEST_sigpromask_2();
extern int TEST_sigpromask_3();
extern int TEST_sigwait_1();
extern int TEST_sigwait_2();
extern int TEST_sigwait_3();
extern int TEST_sigtimedwait_1(void);
extern int TEST_sigtimedwait_2(void);
extern int TEST_sigwaitinfo_1(void);
extern int TEST_sigfillset_1(void);
extern int TEST_sigaddset_1(void);
extern int TEST_sigdelset_1(void);
extern int TEST_sigemptyset_1(void);

typedef int test_run_main();

test_run_main *run_test_arry_1[] = {
    TEST_sigaction_1,
    TEST_sigaction_2,
    TEST_sigaction_3,
    TEST_sigpromask_1,
    TEST_sigpromask_2,
    TEST_sigpromask_3,
    TEST_sigwait_1,
    TEST_sigwait_2,
    TEST_sigwait_3,
    TEST_sigtimedwait_1,
    TEST_sigtimedwait_2,
    TEST_sigwaitinfo_1,
    TEST_sigfillset_1,
    TEST_sigaddset_1,
    TEST_sigdelset_1,
    TEST_sigemptyset_1
};

char run_test_name_1[][50] = {
    "TEST_sigaction_1",
    "TEST_sigaction_2",
    "TEST_sigaction_3",
    "TEST_sigpromask_1",
    "TEST_sigpromask_2",
    "TEST_sigpromask_3",
    "TEST_sigwait_1",
    "TEST_sigwait_2",
    "TEST_sigwait_3",
    "TEST_sigtimedwait_1",
    "TEST_sigtimedwait_2",
    "TEST_sigwaitinfo_1",
    "TEST_sigfillset_1",
    "TEST_sigaddset_1",
    "TEST_sigdelset_1",
    "TEST_sigemptyset_1"
};

#endif