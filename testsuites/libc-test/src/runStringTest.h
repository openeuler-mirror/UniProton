#ifndef _CONFORMSNCE_RUN_TEST_H
#define _CONFORMSNCE_RUN_TEST_H

extern int string_memcpy_test();
extern int string_memmem_test();
extern int string_memset_test();
extern int string_strchr_test();
extern int string_strcspn_test();
extern int string_strstr_test();
extern int string_test();

typedef int test_run_main();

test_run_main *run_test_arry_1[] = {
    string_memcpy_test,
    string_memmem_test,
    string_memset_test,
    string_strchr_test,
    string_strcspn_test,
    string_strstr_test,
    string_test,
};

char run_test_name_1[][50] = {
    "string_memcpy_test",
    "string_memmem_test",
    "string_memset_test",
    "string_strchr_test",
    "string_strcspn_test",
    "string_strstr_test",
    "string_test",
};

#endif