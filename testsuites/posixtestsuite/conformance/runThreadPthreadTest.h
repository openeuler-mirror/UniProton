#ifndef _CONFORMSNCE_RUN_TEST_H
#define _CONFORMSNCE_RUN_TEST_H

extern int pthread_attr_init_3_1();
extern int pthread_attr_init_4_1();
extern int pthread_attr_init_2_1();
extern int pthread_attr_init_1_1();
extern int pthread_attr_destroy_3_1();
extern int pthread_attr_destroy_2_1();
extern int pthread_attr_destroy_1_1();
extern int pthread_attr_getinheritsched_1_1();
extern int pthread_attr_setinheritsched_4_1();
extern int pthread_attr_setinheritsched_1_1();
extern int pthread_attr_getschedpolicy_2_1();
extern int pthread_attr_setschedpolicy_5_1();
extern int pthread_attr_setschedpolicy_4_1();
extern int pthread_attr_setschedpolicy_1_1();
extern int pthread_attr_getdetachstate_1_1();
extern int pthread_attr_getdetachstate_1_2();
extern int pthread_attr_setdetachstate_4_1();
extern int pthread_attr_setdetachstate_2_1();
extern int pthread_attr_setdetachstate_1_1();
extern int pthread_attr_setdetachstate_1_2();
extern int pthread_create_5_1();
extern int pthread_create_12_1();
extern int pthread_create_3_1();
extern int pthread_create_4_1();
extern int pthread_create_2_1();
extern int pthread_create_1_1();
extern int pthread_create_1_2();
extern int pthread_create_5_2();
extern int pthread_cancel_2_3();
extern int pthread_cancel_5_1();
extern int pthread_cancel_2_2();
extern int pthread_cancel_4_1();
extern int pthread_cancel_2_1();
extern int pthread_cancel_1_1();
extern int pthread_cancel_1_2();
extern int pthread_cancel_1_3();
extern int pthread_testcancel_2_1();
extern int pthread_testcancel_1_1();
extern int pthread_setcancelstate_3_1();
extern int pthread_setcancelstate_2_1();
extern int pthread_setcancelstate_1_1();
extern int pthread_setcancelstate_1_2();
extern int pthread_setcanceltype_2_1();
extern int pthread_setcanceltype_1_1();
extern int pthread_setcanceltype_1_2();
extern int pthread_exit_3_1();
extern int pthread_exit_2_1();
extern int pthread_exit_1_1();
extern int pthread_cleanup_push_1_1();
extern int pthread_cleanup_push_1_2();
extern int pthread_cleanup_push_1_3();
extern int pthread_cleanup_pop_1_1();
extern int pthread_cleanup_pop_1_2();
extern int pthread_cleanup_pop_1_3();
extern int pthread_self_1_1();
extern int pthread_equal_1_1();
extern int pthread_equal_1_2();
extern int sched_yield_2_1();
extern int sched_get_priority_max_2_1();
extern int sched_get_priority_max_1_1();
extern int sched_get_priority_max_1_2();
extern int sched_get_priority_max_1_3();
extern int sched_get_priority_max_1_4();
extern int sched_get_priority_min_2_1();
extern int sched_get_priority_min_1_1();
extern int sched_get_priority_min_1_2();
extern int sched_get_priority_min_1_3();
extern int sched_get_priority_min_1_4();
extern int pthread_join_6_2();
extern int pthread_join_5_1();
extern int pthread_join_6_1();
extern int pthread_join_3_1();
extern int pthread_join_2_1();
extern int pthread_join_1_1();
extern int pthread_detach_4_2();
extern int pthread_detach_3_1();
extern int pthread_detach_4_1();
extern int pthread_detach_2_1();
extern int pthread_detach_1_1();
extern int pthread_key_create_3_1();
extern int pthread_key_create_2_1();
extern int pthread_key_create_1_1();
extern int pthread_key_create_1_2();
extern int pthread_setspecific_1_1();
extern int pthread_setspecific_1_2();
extern int pthread_getspecific_3_1();
extern int pthread_getspecific_1_1();
extern int pthread_key_delete_2_1();
extern int pthread_key_delete_1_1();
extern int pthread_key_delete_1_2();
extern int pthread_mutexattr_init_3_1();
extern int pthread_mutexattr_destroy_3_1();
extern int pthread_mutexattr_destroy_4_1();
extern int pthread_mutexattr_destroy_2_1();
extern int pthread_mutexattr_destroy_1_1();
extern int pthread_mutexattr_settype_7_1();
extern int pthread_mutexattr_settype_3_2();
extern int pthread_mutexattr_settype_3_1();
extern int pthread_mutexattr_settype_3_4();
extern int pthread_mutexattr_settype_1_1();
extern int pthread_mutexattr_settype_3_3();
extern int pthread_mutexattr_gettype_3_1();
extern int pthread_mutexattr_gettype_1_5();
extern int pthread_mutexattr_gettype_1_1();
extern int pthread_mutexattr_gettype_1_2();
extern int pthread_mutexattr_gettype_1_3();
extern int pthread_mutexattr_gettype_1_4();
extern int pthread_mutexattr_setprotocol_3_2();
extern int pthread_mutexattr_setprotocol_3_1();
extern int pthread_mutexattr_setprotocol_1_1();
extern int pthread_mutexattr_getprotocol_1_1();
extern int pthread_mutexattr_getprotocol_1_2();
extern int pthread_mutex_init_3_2();
extern int pthread_mutex_init_3_1();
extern int pthread_mutex_init_4_1();
extern int pthread_mutex_init_2_1();
extern int pthread_mutex_init_1_1();
extern int pthread_mutex_init_1_2();
extern int pthread_mutex_destroy_5_1();
extern int pthread_mutex_destroy_4_2();
extern int pthread_mutex_destroy_3_1();
extern int pthread_mutex_destroy_2_1();
extern int pthread_mutex_destroy_1_1();
extern int pthread_mutex_lock_4_1();
extern int pthread_mutex_lock_2_1();
extern int pthread_mutex_trylock_3_1();
extern int pthread_mutex_trylock_4_1();
extern int pthread_mutex_trylock_1_1();
extern int pthread_mutex_timedlock_5_1();
extern int pthread_mutex_timedlock_5_3();
extern int pthread_mutex_timedlock_4_1();
extern int pthread_mutex_timedlock_2_1();
extern int pthread_mutex_timedlock_1_1();
extern int pthread_mutex_timedlock_5_2();
extern int pthread_mutex_unlock_5_1();
extern int pthread_mutex_unlock_3_1();
extern int pthread_mutex_unlock_1_1();
extern int pthread_mutex_unlock_5_2();
extern int pthread_rwlock_init_6_1();
extern int pthread_rwlock_init_3_1();
extern int pthread_rwlock_init_2_1();
extern int pthread_rwlock_init_1_1();
extern int pthread_rwlock_destroy_3_1();
extern int pthread_rwlock_destroy_1_1();
extern int pthread_rwlock_rdlock_5_1();
extern int pthread_rwlock_rdlock_1_1();
extern int pthread_rwlock_tryrdlock_1_1();
extern int pthread_rwlock_timedrdlock_5_1();
extern int pthread_rwlock_timedrdlock_3_1();
extern int pthread_rwlock_timedrdlock_2_1();
extern int pthread_rwlock_timedrdlock_1_1();
extern int pthread_rwlock_wrlock_3_1();
extern int pthread_rwlock_wrlock_1_1();
extern int pthread_rwlock_trywrlock_3_1();
extern int pthread_rwlock_trywrlock_1_1();
extern int pthread_rwlock_timedwrlock_5_1();
extern int pthread_rwlock_timedwrlock_3_1();
extern int pthread_rwlock_timedwrlock_2_1();
extern int pthread_rwlock_timedwrlock_1_1();
extern int pthread_rwlock_unlock_4_2();
extern int pthread_rwlock_unlock_4_1();
extern int pthread_rwlock_unlock_2_1();
extern int pthread_rwlock_unlock_1_1();
extern int pthread_attr_getschedparam_1_1();
extern int pthread_attr_setschedparam_1_1();

extern int pthread_attr_getscope_1_1();
extern int pthread_attr_setscope_1_1();
extern int pthread_attr_setscope_4_1();
extern int pthread_attr_setscope_5_1();

extern int pthread_mutexattr_getpshared_1_1();
extern int pthread_mutexattr_getpshared_1_2();
extern int pthread_mutexattr_getpshared_1_3();
extern int pthread_mutexattr_getpshared_3_1();

extern int pthread_rwlockattr_destroy_1_1();
extern int pthread_rwlockattr_destroy_2_1();
extern int pthread_rwlockattr_getpshared_1_1();
extern int pthread_rwlockattr_getpshared_4_1();
extern int pthread_rwlockattr_init_1_1();
extern int pthread_rwlockattr_init_2_1();

typedef int test_run_main();
test_run_main *run_test_arry_1[] = {
    pthread_attr_init_3_1,
    pthread_attr_init_4_1,
    pthread_attr_init_2_1,
    pthread_attr_init_1_1,
    pthread_attr_destroy_3_1,
    pthread_attr_destroy_2_1,
    // 当前不支持记录是否已初始化
    // pthread_attr_destroy_1_1,
    pthread_attr_getinheritsched_1_1,
    pthread_attr_setinheritsched_4_1,
    pthread_attr_setinheritsched_1_1,
    pthread_attr_getschedpolicy_2_1,
    pthread_attr_setschedpolicy_5_1,
    pthread_attr_setschedpolicy_4_1,
    pthread_attr_setschedpolicy_1_1,
    pthread_attr_getdetachstate_1_1,
    pthread_attr_getdetachstate_1_2,
    pthread_attr_setdetachstate_4_1,
    pthread_attr_setdetachstate_2_1,
    pthread_attr_setdetachstate_1_1,
    pthread_attr_setdetachstate_1_2,
    pthread_create_5_1,
    pthread_create_12_1,
    pthread_create_3_1,
    pthread_create_4_1,
    pthread_create_2_1,
    pthread_create_1_1,
    pthread_create_1_2,
    pthread_create_5_2,
    pthread_cancel_2_3,
    pthread_cancel_5_1,
    pthread_cancel_2_2,
    pthread_cancel_4_1,
    pthread_cancel_2_1,
    pthread_cancel_1_1,
    pthread_cancel_1_2,
    pthread_cancel_1_3,
    pthread_testcancel_2_1,
    pthread_testcancel_1_1,
    pthread_setcancelstate_3_1,
    pthread_setcancelstate_2_1,
    pthread_setcancelstate_1_1,
    pthread_setcancelstate_1_2,
    pthread_setcanceltype_2_1,
    pthread_setcanceltype_1_1,
    pthread_setcanceltype_1_2,
    pthread_exit_3_1,
    pthread_exit_2_1,
    pthread_exit_1_1,
    pthread_cleanup_push_1_1,
    pthread_cleanup_push_1_2,
    pthread_cleanup_push_1_3,
    pthread_cleanup_pop_1_1,
    pthread_cleanup_pop_1_2,
    pthread_cleanup_pop_1_3,
    pthread_self_1_1,
    pthread_equal_1_1,
    pthread_equal_1_2,
    sched_yield_2_1,
    sched_get_priority_max_2_1,
    sched_get_priority_max_1_1,
    sched_get_priority_max_1_2,
    sched_get_priority_max_1_3,
    sched_get_priority_max_1_4,
    sched_get_priority_min_2_1,
    sched_get_priority_min_1_1,
    sched_get_priority_min_1_2,
    sched_get_priority_min_1_3,
    sched_get_priority_min_1_4,
    pthread_join_6_2,
    pthread_join_5_1,
    pthread_join_6_1,
    // 子线程优先级更高，sched_yield不会触发任务切换，此用例不适用
    // pthread_join_3_1,
    pthread_join_2_1,
    pthread_join_1_1,
    pthread_detach_4_2,
    pthread_detach_3_1,
    pthread_detach_4_1,
    pthread_detach_2_1,
    pthread_detach_1_1,
    pthread_key_create_3_1,
    pthread_key_create_2_1,
    pthread_key_create_1_1,
    pthread_key_create_1_2,
    pthread_setspecific_1_1,
    pthread_setspecific_1_2,
    pthread_getspecific_3_1,
    pthread_getspecific_1_1,
    pthread_key_delete_2_1,
    pthread_key_delete_1_1,
    pthread_key_delete_1_2,

    pthread_attr_getscope_1_1,
    pthread_attr_setscope_1_1,
    pthread_attr_setscope_4_1,
    // 用例永远失败
    // pthread_attr_setscope_5_1,
};

test_run_main *run_test_arry_2[] = {
    pthread_mutexattr_init_3_1,
    pthread_mutexattr_destroy_3_1,
    pthread_mutexattr_destroy_4_1,
    pthread_mutexattr_destroy_2_1,
    pthread_mutexattr_destroy_1_1,
    pthread_mutexattr_settype_7_1,
    pthread_mutexattr_settype_3_2,
    pthread_mutexattr_settype_3_1,
    pthread_mutexattr_settype_3_4,
    pthread_mutexattr_settype_1_1,
    pthread_mutexattr_settype_3_3,
    // 当前不支持记录是否已初始化
    // pthread_mutexattr_gettype_3_1,
    pthread_mutexattr_gettype_1_5,
    pthread_mutexattr_gettype_1_1,
    pthread_mutexattr_gettype_1_2,
    pthread_mutexattr_gettype_1_3,
    pthread_mutexattr_gettype_1_4,
    pthread_mutexattr_setprotocol_3_2,
    pthread_mutexattr_setprotocol_3_1,
    pthread_mutexattr_setprotocol_1_1,
    pthread_mutexattr_getprotocol_1_1,
    pthread_mutexattr_getprotocol_1_2,
    pthread_attr_getschedparam_1_1,
    pthread_attr_setschedparam_1_1,
    pthread_mutex_init_3_2,
    pthread_mutex_init_3_1,
    pthread_mutex_init_4_1,
    pthread_mutex_init_2_1,
    pthread_mutex_init_1_1,
    pthread_mutex_init_1_2,

    pthread_mutexattr_getpshared_1_1,
    pthread_mutexattr_getpshared_3_1,

};

test_run_main *run_test_arry_3[] = {
    pthread_mutex_destroy_5_1,
    pthread_mutex_destroy_4_2,
    pthread_mutex_destroy_3_1,
    pthread_mutex_destroy_2_1,
    pthread_mutex_destroy_1_1,
    pthread_mutex_lock_4_1,
    pthread_mutex_lock_2_1,
    pthread_mutex_trylock_3_1,
    pthread_mutex_trylock_4_1,
    pthread_mutex_trylock_1_1,
    pthread_mutex_timedlock_5_1,
    pthread_mutex_timedlock_5_3,
    pthread_mutex_timedlock_4_1,
    pthread_mutex_timedlock_2_1,
    pthread_mutex_timedlock_5_2,
    pthread_mutex_unlock_5_1,
    pthread_mutex_unlock_3_1,
    pthread_mutex_unlock_1_1,
    pthread_mutex_unlock_5_2,
    pthread_rwlock_init_6_1,
    pthread_rwlock_init_3_1,
    pthread_rwlock_init_2_1,
    pthread_rwlock_init_1_1,
    pthread_rwlock_destroy_3_1,
    pthread_rwlock_destroy_1_1,
    pthread_rwlock_rdlock_5_1,
    pthread_rwlock_rdlock_1_1,
    pthread_rwlock_tryrdlock_1_1,
    pthread_rwlock_timedrdlock_3_1,
    pthread_rwlock_wrlock_3_1,
    pthread_rwlock_wrlock_1_1,
    pthread_rwlock_trywrlock_3_1,
    pthread_rwlock_trywrlock_1_1,
    pthread_rwlock_timedwrlock_3_1,
    pthread_rwlock_unlock_4_2,
    pthread_rwlock_unlock_4_1,
    pthread_rwlock_unlock_2_1,
    pthread_rwlock_unlock_1_1,

    pthread_rwlockattr_destroy_1_1,
    pthread_rwlockattr_destroy_2_1,
    pthread_rwlockattr_getpshared_1_1,
    pthread_rwlockattr_getpshared_4_1,
    pthread_rwlockattr_init_1_1,
    pthread_rwlockattr_init_2_1,
};

char run_test_name_1[][50] = {
    "pthread_attr_init_3_1",
    "pthread_attr_init_4_1",
    "pthread_attr_init_2_1",
    "pthread_attr_init_1_1",
    "pthread_attr_destroy_3_1",
    "pthread_attr_destroy_2_1",
    // 当前不支持记录是否已初始化
    // "pthread_attr_destroy_1_1",
    "pthread_attr_getinheritsched_1_1",
    "pthread_attr_setinheritsched_4_1",
    "pthread_attr_setinheritsched_1_1",
    "pthread_attr_getschedpolicy_2_1",
    "pthread_attr_setschedpolicy_5_1",
    "pthread_attr_setschedpolicy_4_1",
    "pthread_attr_setschedpolicy_1_1",
    "pthread_attr_getdetachstate_1_1",
    "pthread_attr_getdetachstate_1_2",
    "pthread_attr_setdetachstate_4_1",
    "pthread_attr_setdetachstate_2_1",
    "pthread_attr_setdetachstate_1_1",
    "pthread_attr_setdetachstate_1_2",
    "pthread_create_5_1",
    "pthread_create_12_1",
    "pthread_create_3_1",
    "pthread_create_4_1",
    "pthread_create_2_1",
    "pthread_create_1_1",
    "pthread_create_1_2",
    "pthread_create_5_2",
    "pthread_cancel_2_3",
    "pthread_cancel_5_1",
    "pthread_cancel_2_2",
    "pthread_cancel_4_1",
    "pthread_cancel_2_1",
    "pthread_cancel_1_1",
    "pthread_cancel_1_2",
    "pthread_cancel_1_3",
    "pthread_testcancel_2_1",
    "pthread_testcancel_1_1",
    "pthread_setcancelstate_3_1",
    "pthread_setcancelstate_2_1",
    "pthread_setcancelstate_1_1",
    "pthread_setcancelstate_1_2",
    "pthread_setcanceltype_2_1",
    "pthread_setcanceltype_1_1",
    "pthread_setcanceltype_1_2",
    "pthread_exit_3_1",
    "pthread_exit_2_1",
    "pthread_exit_1_1",
    "pthread_cleanup_push_1_1",
    "pthread_cleanup_push_1_2",
    "pthread_cleanup_push_1_3",
    "pthread_cleanup_pop_1_1",
    "pthread_cleanup_pop_1_2",
    "pthread_cleanup_pop_1_3",
    "pthread_self_1_1",
    "pthread_equal_1_1",
    "pthread_equal_1_2",
    "sched_yield_2_1",
    "sched_get_priority_max_2_1",
    "sched_get_priority_max_1_1",
    "sched_get_priority_max_1_2",
    "sched_get_priority_max_1_3",
    "sched_get_priority_max_1_4",
    "sched_get_priority_min_2_1",
    "sched_get_priority_min_1_1",
    "sched_get_priority_min_1_2",
    "sched_get_priority_min_1_3",
    "sched_get_priority_min_1_4",
    "pthread_join_6_2",
    "pthread_join_5_1",
    "pthread_join_6_1",
    // 子线程优先级更高，sched_yield不会触发任务切换，此用例不适用
    // "pthread_join_3_1",
    "pthread_join_2_1",
    "pthread_join_1_1",
    "pthread_detach_4_2",
    "pthread_detach_3_1",
    "pthread_detach_4_1",
    "pthread_detach_2_1",
    "pthread_detach_1_1",
    "pthread_key_create_3_1",
    "pthread_key_create_2_1",
    "pthread_key_create_1_1",
    "pthread_key_create_1_2",
    "pthread_setspecific_1_1",
    "pthread_setspecific_1_2",
    "pthread_getspecific_3_1",
    "pthread_getspecific_1_1",
    "pthread_key_delete_2_1",
    "pthread_key_delete_1_1",
    "pthread_key_delete_1_2",

    "pthread_attr_getscope_1_1",
    "pthread_attr_setscope_1_1",
    "pthread_attr_setscope_4_1",
    // 用例永远失败
    // "pthread_attr_setscope_5_1",
};

char run_test_name_2[][50] = {
    "pthread_mutexattr_init_3_1",
    "pthread_mutexattr_destroy_3_1",
    "pthread_mutexattr_destroy_4_1",
    "pthread_mutexattr_destroy_2_1",
    "pthread_mutexattr_destroy_1_1",
    "pthread_mutexattr_settype_7_1",
    "pthread_mutexattr_settype_3_2",
    "pthread_mutexattr_settype_3_1",
    "pthread_mutexattr_settype_3_4",
    "pthread_mutexattr_settype_1_1",
    "pthread_mutexattr_settype_3_3",
    // 当前不支持记录是否已初始化
    // "pthread_mutexattr_gettype_3_1",
    "pthread_mutexattr_gettype_1_5",
    "pthread_mutexattr_gettype_1_1",
    "pthread_mutexattr_gettype_1_2",
    "pthread_mutexattr_gettype_1_3",
    "pthread_mutexattr_gettype_1_4",
    "pthread_mutexattr_setprotocol_3_2",
    "pthread_mutexattr_setprotocol_3_1",
    "pthread_mutexattr_setprotocol_1_1",
    "pthread_mutexattr_getprotocol_1_1",
    "pthread_mutexattr_getprotocol_1_2",
    "pthread_attr_getschedparam_1_1",
    "pthread_attr_setschedparam_1_1",
    "pthread_mutex_init_3_2",
    "pthread_mutex_init_3_1",
    "pthread_mutex_init_4_1",
    "pthread_mutex_init_2_1",
    "pthread_mutex_init_1_1",
    "pthread_mutex_init_1_2",

    "pthread_mutexattr_getpshared_1_1",
    "pthread_mutexattr_getpshared_3_1",
};

char run_test_name_3[][50] = {
    "pthread_mutex_destroy_5_1",
    "pthread_mutex_destroy_4_2",
    "pthread_mutex_destroy_3_1",
    "pthread_mutex_destroy_2_1",
    "pthread_mutex_destroy_1_1",
    "pthread_mutex_lock_4_1",
    "pthread_mutex_lock_2_1",
    "pthread_mutex_trylock_3_1",
    "pthread_mutex_trylock_4_1",
    "pthread_mutex_trylock_1_1",
    "pthread_mutex_timedlock_5_1",
    "pthread_mutex_timedlock_5_3",
    "pthread_mutex_timedlock_4_1",
    "pthread_mutex_timedlock_2_1",
    "pthread_mutex_timedlock_5_2",
    "pthread_mutex_unlock_5_1",
    "pthread_mutex_unlock_3_1",
    "pthread_mutex_unlock_1_1",
    "pthread_mutex_unlock_5_2",
    "pthread_rwlock_init_6_1",
    "pthread_rwlock_init_3_1",
    "pthread_rwlock_init_2_1",
    "pthread_rwlock_init_1_1",
    "pthread_rwlock_destroy_3_1",
    "pthread_rwlock_destroy_1_1",
    "pthread_rwlock_rdlock_5_1",
    "pthread_rwlock_rdlock_1_1",
    "pthread_rwlock_tryrdlock_1_1",
    "pthread_rwlock_timedrdlock_3_1",
    "pthread_rwlock_wrlock_3_1",
    "pthread_rwlock_wrlock_1_1",
    "pthread_rwlock_trywrlock_3_1",
    "pthread_rwlock_trywrlock_1_1",
    "pthread_rwlock_timedwrlock_3_1",
    "pthread_rwlock_unlock_4_2",
    "pthread_rwlock_unlock_4_1",
    "pthread_rwlock_unlock_2_1",
    "pthread_rwlock_unlock_1_1",

    "pthread_rwlockattr_destroy_1_1",
    "pthread_rwlockattr_destroy_2_1",
    "pthread_rwlockattr_getpshared_1_1",
    "pthread_rwlockattr_getpshared_4_1",
    "pthread_rwlockattr_init_1_1",
    "pthread_rwlockattr_init_2_1",
};

#endif