#ifndef _DEFINITION_RUN_RWFALG_TEST_H
#define _DEFINITION_RUN_RWFALG_TEST_H

extern int TEST_rwflag_1();
extern int TEST_rwflag_2();
extern int TEST_rwflag_3();
extern int TEST_rwflag_4();
extern int TEST_rwflag_5();
extern int TEST_rwflag_6();

typedef int test_run_main();

test_run_main *run_test_arry_1[] = {
    TEST_rwflag_1,
    TEST_rwflag_2,
    TEST_rwflag_3,
    TEST_rwflag_4,
    TEST_rwflag_5,
    TEST_rwflag_6
};

char run_test_name_1[][50] = {
    "TEST_rwflag_1",
    "TEST_rwflag_2",
    "TEST_rwflag_3",
    "TEST_rwflag_4",
    "TEST_rwflag_5",
    "TEST_rwflag_6"
};

#endif /* _DEFINITION_RUN_RWFALG_TEST_H */