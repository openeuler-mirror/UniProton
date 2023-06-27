#ifndef _CONFORMSNCE_RUN_TEST_H
#define _CONFORMSNCE_RUN_TEST_H

extern int sem_init_7_1();
extern int sem_init_5_1();
extern int sem_init_2_2();
extern int sem_init_3_1();
extern int sem_init_2_1();
extern int sem_init_1_1();
extern int sem_init_5_2();
extern int sem_destroy_3_1();
extern int sem_destroy_4_1();
extern int sem_open_15_1();
extern int sem_open_2_2();
extern int sem_open_6_1();
extern int sem_open_4_1();
extern int sem_open_2_1();
extern int sem_open_1_1();
extern int sem_open_1_2();
extern int sem_open_1_3();
extern int sem_open_1_4();
extern int sem_open_10_1();
extern int sem_close_3_2();
extern int sem_close_3_1();
extern int sem_close_2_1();
extern int sem_close_1_1();
extern int sem_wait_5_1();
extern int sem_wait_11_1();
extern int sem_wait_12_1();
extern int sem_wait_3_1();
extern int sem_wait_1_1();
extern int sem_wait_1_2();
extern int sem_timedwait_7_1();
extern int sem_timedwait_6_2();
extern int sem_timedwait_11_1();
extern int sem_timedwait_2_2();
extern int sem_timedwait_6_1();
extern int sem_timedwait_3_1();
extern int sem_timedwait_4_1();
extern int sem_timedwait_1_1();
extern int sem_timedwait_10_1();
extern int sem_post_4_1();
extern int sem_post_2_1();
extern int sem_post_1_1();
extern int sem_post_1_2();
extern int sem_getvalue_5_1();
extern int sem_getvalue_2_2();
extern int sem_getvalue_4_1();
extern int sem_getvalue_2_1();
extern int sem_getvalue_1_1();

extern int pthread_barrier_destroy_1_1();
extern int pthread_barrier_destroy_2_1();
extern int pthread_barrier_init_1_1();
extern int pthread_barrier_init_3_1();
extern int pthread_barrier_init_4_1();
extern int pthread_barrier_wait_1_1();
extern int pthread_barrier_wait_2_1();
extern int pthread_barrier_wait_3_1();
extern int pthread_barrier_wait_3_2();
extern int pthread_barrier_wait_6_1();
extern int pthread_barrierattr_destroy_1_1();
extern int pthread_barrierattr_getpshared_1_1();
extern int pthread_barrierattr_getpshared_2_1();
extern int pthread_barrierattr_init_1_1();
extern int pthread_barrierattr_init_2_1();
extern int pthread_barrierattr_setpshared_1_1();
extern int pthread_barrierattr_setpshared_2_1();

extern int pthread_cond_broadcast_1_1();
extern int pthread_cond_broadcast_1_2();
extern int pthread_cond_broadcast_2_1();
extern int pthread_cond_broadcast_2_2();
extern int pthread_cond_broadcast_2_3();
extern int pthread_cond_broadcast_4_1();
extern int pthread_cond_broadcast_4_2();
extern int pthread_cond_destroy_1_1();
extern int pthread_cond_destroy_2_1();
extern int pthread_cond_destroy_3_1();
extern int pthread_cond_destroy_4_1();

extern int pthread_cond_init_1_1();
extern int pthread_cond_init_1_2();
extern int pthread_cond_init_1_3();
extern int pthread_cond_init_2_1();
extern int pthread_cond_init_2_2();
extern int pthread_cond_init_3_1();
extern int pthread_cond_init_4_1();
extern int pthread_cond_init_4_2();

extern int pthread_cond_signal_1_1();
extern int pthread_cond_signal_1_2();
extern int pthread_cond_signal_2_1();
extern int pthread_cond_signal_2_2();
extern int pthread_cond_signal_4_1();
extern int pthread_cond_signal_4_2();

extern int pthread_cond_timedwait_1_1();
extern int pthread_cond_timedwait_2_1();
extern int pthread_cond_timedwait_2_2();
extern int pthread_cond_timedwait_2_3();
extern int pthread_cond_timedwait_2_4();
extern int pthread_cond_timedwait_2_5();
extern int pthread_cond_timedwait_2_6();
extern int pthread_cond_timedwait_2_7();
extern int pthread_cond_timedwait_3_1();
extern int pthread_cond_timedwait_4_1();
extern int pthread_cond_timedwait_4_2();
extern int pthread_cond_timedwait_4_3();

extern int pthread_cond_wait_1_1();
extern int pthread_cond_wait_2_1();
extern int pthread_cond_wait_2_2();
extern int pthread_cond_wait_2_3();
extern int pthread_cond_wait_3_1();
extern int pthread_cond_wait_4_1();

extern int pthread_condattr_destroy_1_1();
extern int pthread_condattr_destroy_2_1();
extern int pthread_condattr_destroy_3_1();
extern int pthread_condattr_destroy_4_1();

extern int pthread_condattr_getclock_1_1();
extern int pthread_condattr_getclock_1_2();

extern int pthread_condattr_getpshared_1_1();
extern int pthread_condattr_getpshared_1_2();
extern int pthread_condattr_getpshared_2_1();

extern int pthread_condattr_init_1_1();
extern int pthread_condattr_init_3_1();

extern int pthread_condattr_setclock_1_1();
extern int pthread_condattr_setclock_1_2();
extern int pthread_condattr_setclock_1_3();
extern int pthread_condattr_setclock_2_1();

extern int pthread_condattr_setpshared_1_1();
extern int pthread_condattr_setpshared_1_2();
extern int pthread_condattr_setpshared_2_1();

extern int pthread_getcpuclockid_1_1();
extern int pthread_getcpuclockid_3_1();
extern int pthread_getschedparam_1_1();
extern int pthread_getschedparam_1_2();

extern int pthread_once_1_1();
extern int pthread_once_1_2();
extern int pthread_once_1_3();
extern int pthread_once_2_1();
extern int pthread_once_3_1();
extern int pthread_once_4_1();

extern int pthread_setschedparam_1_1();
extern int pthread_setschedparam_1_2();
extern int pthread_setschedparam_4_1();
extern int pthread_setschedprio_1_1();

extern int pthread_spin_destroy_1_1();
extern int pthread_spin_destroy_3_1();
extern int pthread_spin_init_1_1();
extern int pthread_spin_init_4_1();
extern int pthread_spin_lock_1_2();
extern int pthread_spin_trylock_4_1();
extern int pthread_spin_unlock_1_1();
extern int pthread_spin_unlock_1_2();
extern int pthread_spin_unlock_3_1();

extern int sem_unlink_1_1();
extern int sem_unlink_2_1();
extern int sem_unlink_4_1();
extern int sem_unlink_4_2();
extern int sem_unlink_6_1();
extern int sem_unlink_7_1();
extern int sem_unlink_9_1();

typedef int test_run_main();
test_run_main *run_test_arry_1[] = {
    sem_init_5_1,
    sem_init_2_2,
    sem_init_3_1,
    sem_init_2_1,
    sem_init_1_1,
    sem_init_5_2,
    sem_destroy_3_1,
    sem_destroy_4_1,
    sem_open_15_1,
    sem_open_2_2,
    sem_open_6_1,
    sem_open_4_1,
    sem_open_2_1,
    sem_open_1_1,
    sem_open_1_2,
    sem_open_1_3,
    sem_open_1_4,
    sem_open_10_1,
    sem_close_3_2,
    sem_close_3_1,
    sem_close_2_1,
    sem_close_1_1,
    sem_wait_5_1,
    sem_wait_11_1,
    sem_wait_12_1,
    sem_wait_3_1,
    sem_wait_1_1,
    sem_wait_1_2,
    sem_timedwait_7_1,
    sem_timedwait_6_2,
    sem_timedwait_11_1,
    sem_timedwait_2_2,
    sem_timedwait_6_1,
    sem_timedwait_3_1,
    sem_timedwait_4_1,
    sem_timedwait_1_1,
    sem_timedwait_10_1,
    sem_post_4_1,
    sem_post_2_1,
    sem_post_1_1,
    sem_post_1_2,
    sem_getvalue_5_1,
    sem_getvalue_2_2,
    sem_getvalue_4_1,
    sem_getvalue_2_1,
    sem_getvalue_1_1,

    pthread_barrier_destroy_1_1,
    pthread_barrier_destroy_2_1,
    pthread_barrier_init_1_1,
    pthread_barrier_init_3_1,
    pthread_barrier_wait_2_1,
    pthread_barrierattr_destroy_1_1,
    pthread_barrierattr_getpshared_1_1,
    pthread_barrierattr_init_1_1,
    pthread_barrierattr_init_2_1,
    pthread_barrierattr_setpshared_1_1,
    pthread_barrierattr_setpshared_2_1,

    pthread_cond_broadcast_1_1,
    pthread_cond_broadcast_2_1,
    pthread_cond_broadcast_2_2,
    pthread_cond_broadcast_4_1,

    pthread_cond_destroy_1_1,
    pthread_cond_destroy_3_1,
    pthread_cond_destroy_4_1,

    pthread_cond_init_1_1,
    pthread_cond_init_1_2,
    pthread_cond_init_2_1,
    pthread_cond_init_2_2,
    pthread_cond_init_3_1,

    pthread_cond_signal_1_1,
    pthread_cond_signal_2_1,
    pthread_cond_signal_2_2,
    pthread_cond_signal_4_1,

    pthread_cond_timedwait_1_1,
    pthread_cond_timedwait_2_1,
    pthread_cond_timedwait_2_2,
    pthread_cond_timedwait_3_1,
    pthread_cond_timedwait_4_1,

    pthread_cond_wait_1_1,
    pthread_cond_wait_2_1,

    pthread_condattr_destroy_1_1,
    pthread_condattr_destroy_2_1,
    pthread_condattr_destroy_3_1,
    pthread_condattr_destroy_4_1,

    pthread_condattr_getclock_1_1,
    pthread_condattr_getclock_1_2,

    pthread_condattr_getpshared_1_1,
    pthread_condattr_getpshared_2_1,

    pthread_condattr_init_1_1,
    pthread_condattr_init_3_1,
    pthread_condattr_setclock_1_1,
    pthread_condattr_setclock_1_3,
    pthread_condattr_setclock_2_1,
    pthread_condattr_setpshared_1_1,
    pthread_condattr_setpshared_2_1,

    pthread_getcpuclockid_1_1,
    pthread_getcpuclockid_3_1,
    pthread_getschedparam_1_1,
    pthread_getschedparam_1_2,

    pthread_once_1_1,
    pthread_once_1_2,
    pthread_once_1_3,
    pthread_once_2_1,
    // 暂不支持
    // pthread_once_3_1,
    pthread_once_4_1,

    pthread_setschedparam_1_1,
    pthread_setschedparam_1_2,
    pthread_setschedparam_4_1,

    sem_unlink_1_1,
    sem_unlink_2_1,
    sem_unlink_4_1,
    sem_unlink_4_2,
    sem_unlink_6_1,
    sem_unlink_7_1,
    sem_unlink_9_1,
};

char run_test_name_1[][50] = {
    "sem_init_5_1",
    "sem_init_2_2",
    "sem_init_3_1",
    "sem_init_2_1",
    "sem_init_1_1",
    "sem_init_5_2",
    "sem_destroy_3_1",
    "sem_destroy_4_1",
    "sem_open_15_1",
    "sem_open_2_2",
    "sem_open_6_1",
    "sem_open_4_1",
    "sem_open_2_1",
    "sem_open_1_1",
    "sem_open_1_2",
    "sem_open_1_3",
    "sem_open_1_4",
    "sem_open_10_1",
    "sem_close_3_2",
    "sem_close_3_1",
    "sem_close_2_1",
    "sem_close_1_1",
    "sem_wait_5_1",
    "sem_wait_11_1",
    "sem_wait_12_1",
    "sem_wait_3_1",
    "sem_wait_1_1",
    "sem_wait_1_2",
    "sem_timedwait_7_1",
    "sem_timedwait_6_2",
    "sem_timedwait_11_1",
    "sem_timedwait_2_2",
    "sem_timedwait_6_1",
    "sem_timedwait_3_1",
    "sem_timedwait_4_1",
    "sem_timedwait_1_1",
    "sem_timedwait_10_1",
    "sem_post_4_1",
    "sem_post_2_1",
    "sem_post_1_1",
    "sem_post_1_2",
    "sem_getvalue_5_1",
    "sem_getvalue_2_2",
    "sem_getvalue_4_1",
    "sem_getvalue_2_1",
    "sem_getvalue_1_1",

    "pthread_barrier_destroy_1_1",
    "pthread_barrier_destroy_2_1",
    "pthread_barrier_init_1_1",
    "pthread_barrier_init_3_1",
    "pthread_barrier_wait_2_1",
    "pthread_barrierattr_destroy_1_1",
    "pthread_barrierattr_getpshared_1_1",
    "pthread_barrierattr_init_1_1",
    "pthread_barrierattr_init_2_1",
    "pthread_barrierattr_setpshared_1_1",
    "pthread_barrierattr_setpshared_2_1",

    "pthread_cond_broadcast_1_1",
    "pthread_cond_broadcast_2_1",
    "pthread_cond_broadcast_2_2",
    "pthread_cond_broadcast_4_1",

    "pthread_cond_destroy_1_1",
    "pthread_cond_destroy_3_1",
    "pthread_cond_destroy_4_1",

    "pthread_cond_init_1_1",
    "pthread_cond_init_1_2",
    "pthread_cond_init_2_1",
    "pthread_cond_init_2_2",
    "pthread_cond_init_3_1",

    "pthread_cond_signal_1_1",
    "pthread_cond_signal_2_1",
    "pthread_cond_signal_2_2",
    "pthread_cond_signal_4_1",

    "pthread_cond_timedwait_1_1",
    "pthread_cond_timedwait_2_1",
    "pthread_cond_timedwait_2_2",
    "pthread_cond_timedwait_3_1",
    "pthread_cond_timedwait_4_1",

    "pthread_cond_wait_1_1",
    "pthread_cond_wait_2_1",

    "pthread_condattr_destroy_1_1",
    "pthread_condattr_destroy_2_1",
    "pthread_condattr_destroy_3_1",
    "pthread_condattr_destroy_4_1",

    "pthread_condattr_getclock_1_1",
    "pthread_condattr_getclock_1_2",

    "pthread_condattr_getpshared_1_1",
    "pthread_condattr_getpshared_2_1",

    "pthread_condattr_init_1_1",
    "pthread_condattr_init_3_1",
    "pthread_condattr_setclock_1_1",
    "pthread_condattr_setclock_1_3",
    "pthread_condattr_setclock_2_1",
    "pthread_condattr_setpshared_1_1",
    "pthread_condattr_setpshared_2_1",

    "pthread_getcpuclockid_1_1",
    "pthread_getcpuclockid_3_1",
    "pthread_getschedparam_1_1",
    "pthread_getschedparam_1_2",

    "pthread_once_1_1",
    "pthread_once_1_2",
    "pthread_once_1_3",
    "pthread_once_2_1",
    // 暂不支持
    // "pthread_once_3_1",
    "pthread_once_4_1",

    "pthread_setschedparam_1_1",
    "pthread_setschedparam_1_2",
    "pthread_setschedparam_4_1",

    "sem_unlink_1_1",
    "sem_unlink_2_1",
    "sem_unlink_4_1",
    "sem_unlink_4_2",
    "sem_unlink_6_1",
    "sem_unlink_7_1",
    "sem_unlink_9_1",

};

#endif