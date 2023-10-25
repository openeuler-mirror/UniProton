#ifndef _CONFORMSNCE_RUN_TEST_H
#define _CONFORMSNCE_RUN_TEST_H

extern int ipc_msg_test();
extern int ipc_sem_test();
extern int ipc_shm_test();

typedef int test_run_main();

test_run_main *run_test_arry_1[] = {
    ipc_shm_test,
    ipc_msg_test,
    ipc_sem_test,
};

char run_test_name_1[][50] = {
    "ipc_shm_test",
    "ipc_msg_test", 
    "ipc_sem_test",
};

#endif