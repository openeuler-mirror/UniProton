#ifndef _CONFORMSNCE_RUN_TEST_H
#define _CONFORMSNCE_RUN_TEST_H

extern int inode_initialize_test();
extern int inode_addref_test();
extern int inode_nextname_test();

typedef int test_run_main();
test_run_main *run_test_arry_1[] = {
    inode_initialize_test,
    inode_addref_test,
    inode_nextname_test,
};

char run_test_name_1[][50] = {
    "inode_initialize_test",
    "inode_addref_test",
    "inode_nextname_test",
};

#endif