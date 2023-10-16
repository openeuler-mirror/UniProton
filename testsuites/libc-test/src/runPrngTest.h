#ifndef _CONFORMSNCE_RUN_TEST_H
#define _CONFORMSNCE_RUN_TEST_H

extern int random_test();

typedef int test_run_main();

test_run_main *run_test_arry_1[] = {
    random_test,
};

char run_test_name_1[][50] = {
    "random_test",
};

#endif