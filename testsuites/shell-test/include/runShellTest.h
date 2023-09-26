#ifndef _SHELL_RUN_TEST_H
#define _SHELL_RUN_TEST_H

#include "stdarg.h"
#include "prt_buildef.h"
#include "prt_typedef.h"

extern int test_shell_1();

typedef int test_run_main();

test_run_main *run_shell_test_arry_1[] = {
    test_shell_1,
};

char run_shell_test_name_1[][50] = {
    "test_shell_1", 
};

#endif /* _SHELL_RUN_TEST_H */