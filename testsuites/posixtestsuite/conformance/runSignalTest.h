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
#if defined(OS_OPTION_SMP)
extern int TEST_sigwait_7();
#endif
extern int TEST_sigtimedwait_1(void);
extern int TEST_sigtimedwait_2(void);
extern int TEST_sigwaitinfo_1(void);
extern int TEST_sigfillset_1(void);
extern int TEST_sigaddset_1(void);
extern int TEST_sigdelset_1(void);
extern int TEST_sigemptyset_1(void);

extern int TEST_pause(void);
extern int TEST_signal_1(void);
extern int TEST_signal_2(void);
extern int TEST_signal_3(void);

extern int setitimer_1_1(void);
extern int setitimer_1_2(void);
extern int getitimer_1_1(void);
extern int getitimer_1_2(void);

extern int kill_1_1(void);
extern int kill_2_1(void);

extern int raise_1_1(void);
extern int raise_2_1(void);
extern int raise_4_1(void);
extern int raise_6_1(void);
extern int raise_7_1(void);

extern int sigsuspend_7_1(void);
#if (defined(OS_OPTION_SMP) && (OS_SYS_CORE_RUN_NUM > 1))
extern int sigsuspend_7_2(void);
#endif

extern int sigdelset_1_3(void);
extern int sigdelset_1_4(void);
extern int sigdelset_2_1(void);

extern int sighold_1_1(void);
extern int sighold_2_1(void);

extern int sigignore_1_1();
extern int sigignore_4_1();
extern int sigignore_6_1();
extern int sigignore_6_2();

extern int sigismember_3_1();
extern int sigismember_4_1();

extern int signal_1_1();
extern int signal_2_1();
extern int signal_3_1();
extern int signal_5_1();
extern int signal_6_1();
extern int signal_7_1();

extern int sigpause_1_1();
extern int sigpause_1_2();
extern int sigpause_2_1();
extern int sigpause_3_1();
extern int sigpause_4_1();

extern int sigrelse_1_1();
extern int sigrelse_2_1();

extern int sigpending_1_1();
extern int sigpending_1_2();
extern int sigpending_1_3();
extern int sigpending_2_1();

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
#if (defined(OS_OPTION_SMP) && (OS_SYS_CORE_RUN_NUM > 1))
    TEST_sigwait_7,
#endif
    TEST_sigtimedwait_1,
    TEST_sigtimedwait_2,
    TEST_sigwaitinfo_1,
    TEST_sigfillset_1,
    TEST_sigaddset_1,
    TEST_sigdelset_1,
    TEST_sigemptyset_1,
    TEST_signal_1,
    TEST_signal_2,
    TEST_signal_3,
    TEST_pause,
    setitimer_1_1,
    setitimer_1_2,
    getitimer_1_1,
    getitimer_1_2,
    kill_1_1,
    kill_2_1,
    raise_1_1,
    raise_2_1,
    raise_4_1,
    raise_6_1,
    raise_7_1,
    sigsuspend_7_1,
#if (defined(OS_OPTION_SMP) && (OS_SYS_CORE_RUN_NUM > 1))
    sigsuspend_7_2,
#endif
    sigdelset_1_3,
    sigdelset_1_4,
    sigdelset_2_1,
    sighold_1_1,
    sighold_2_1,
    sigignore_1_1,
    sigignore_4_1,
    sigignore_6_1,
    sigignore_6_2,
    sigismember_3_1,
    sigismember_4_1,
    signal_1_1,
    signal_2_1,
    signal_3_1,
    signal_5_1,
    signal_6_1,
    signal_7_1,
    sigpause_1_1,
    sigpause_1_2,
    sigpause_2_1,
    sigpause_3_1,
    sigpause_4_1,
    sigrelse_1_1,
    sigrelse_2_1,
    sigpending_1_1,
    sigpending_1_2,
    sigpending_1_3,
    sigpending_2_1,
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
#if (defined(OS_OPTION_SMP) && (OS_SYS_CORE_RUN_NUM > 1))
    "TEST_sigwait_7",
#endif
    "TEST_sigtimedwait_1",
    "TEST_sigtimedwait_2",
    "TEST_sigwaitinfo_1",
    "TEST_sigfillset_1",
    "TEST_sigaddset_1",
    "TEST_sigdelset_1",
    "TEST_sigemptyset_1",
    "TEST_signal_1",
    "TEST_signal_2",
    "TEST_signal_3",
    "TEST_pause",
    "setitimer_1_1",
    "setitimer_1_2",
    "getitimer_1_1",
    "getitimer_1_2",
    "kill_1_1",
    "kill_2_1",
    "raise_1_1",
    "raise_2_1",
    "raise_4_1",
    "raise_6_1",
    "raise_7_1",
    "sigsuspend_7_1",
#if (defined(OS_OPTION_SMP) && (OS_SYS_CORE_RUN_NUM > 1))
    "sigsuspend_7_2",
#endif
    "sigdelset_1_3",
    "sigdelset_1_4",
    "sigdelset_2_1",
    "sighold_1_1",
    "sighold_2_1",
    "sigignore_1_1",
    "sigignore_4_1",
    "sigignore_6_1",
    "sigignore_6_2",
    "sigismember_3_1",
    "sigismember_4_1",
    "signal_1_1",
    "signal_2_1",
    "signal_3_1",
    "signal_5_1",
    "signal_6_1",
    "signal_7_1",
    "sigpause_1_1",
    "sigpause_1_2",
    "sigpause_2_1",
    "sigpause_3_1",
    "sigpause_4_1",
    "sigrelse_1_1",
    "sigrelse_2_1",
    "sigpending_1_1",
    "sigpending_1_2",
    "sigpending_1_3",
    "sigpending_2_1",
};

#endif