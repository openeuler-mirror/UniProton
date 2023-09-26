#ifndef _CONFORMSNCE_RUN_TEST_H
#define _CONFORMSNCE_RUN_TEST_H

extern int uart_test();

typedef int test_run_main();
test_run_main *run_test_arry_1[] = {
    uart_test,
};

char run_test_name_1[][50] = {
    "uart_test",
};

#endif