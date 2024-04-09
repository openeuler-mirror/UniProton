set(TestCaseName UniPorton_test_posix_interface)
file(GLOB ALL_TIME_SRC
    ./interfaces/clock_gettime/[0-9]**.c
    ./interfaces/clock_settime/[0-9]**.c
    ./interfaces/clock_settime/speculative/[0-9]**.c
    ./interfaces/clock_getres/[0-9]**.c
    ./interfaces/nanosleep/[0-9]**.c
    ./interfaces/timer_create/[0-9]**.c
    ./interfaces/timer_create/speculative/[0-9]**.c
    ./interfaces/timer_delete/[0-9]**.c
    ./interfaces/timer_delete/speculative/[0-9]**.c
    ./interfaces/timer_settime/[0-9]**.c
    ./interfaces/timer_settime/speculative/[0-9]**.c
    ./interfaces/timer_gettime/[0-9]**.c
    ./interfaces/timer_gettime/speculative/[0-9]**.c
    ./interfaces/time/[0-9]**.c
    ./interfaces/gettimeofday/[0-9]**.c
    ./interfaces/usleep/[0-9]**.c
    ./interfaces/getpagesize/[0-9]**.c
    
    ./interfaces/asctime/[0-9]**.c
    ./interfaces/clock/[0-9]**.c
    ./interfaces/clock_getcpuclockid/[0-9]**.c
    ./interfaces/clock_nanosleep/[0-9]**.c
    ./interfaces/ctime/[0-9]**.c
    ./interfaces/difftime/[0-9]**.c
    ./interfaces/gmtime/[0-9]**.c
    ./interfaces/localtime/[0-9]**.c
    ./interfaces/mktime/[0-9]**.c
    ./interfaces/strftime/[0-9]**.c
    ./interfaces/timer_getoverrun/[0-9]**.c
    ./interfaces/timer_getoverrun/speculative/[0-9]**.c
)

file(GLOB ALL_SEM_SRC
    ./interfaces/sem_init/[0-9]**.c
    ./interfaces/sem_destroy/[0-9]**.c
    ./interfaces/sem_open/[0-9]**.c
    ./interfaces/sem_close/[0-9]**.c
    ./interfaces/sem_wait/[0-9]**.c
    ./interfaces/sem_timedwait/[0-9]**.c
    ./interfaces/sem_post/[0-9]**.c
    ./interfaces/sem_getvalue/[0-9]**.c

    ./interfaces/pthread_barrier_destroy/[0-9]**.c
    ./interfaces/pthread_barrier_init/[0-9]**.c
    ./interfaces/pthread_barrier_wait/[0-9]**.c
    ./interfaces/pthread_barrierattr_destroy/[0-9]**.c
    ./interfaces/pthread_barrierattr_getpshared/[0-9]**.c
    ./interfaces/pthread_barrierattr_init/[0-9]**.c
    ./interfaces/pthread_barrierattr_setpshared/[0-9]**.c

    ./interfaces/pthread_cond_broadcast/[0-9]**.c
    ./interfaces/pthread_cond_destroy/[0-9]**.c
    ./interfaces/pthread_cond_destroy/speculative/[0-9]**.c
    ./interfaces/pthread_cond_init/[0-9]**.c
    ./interfaces/pthread_cond_signal/[0-9]**.c
    ./interfaces/pthread_cond_timedwait/[0-9]**.c
    ./interfaces/pthread_cond_wait/[0-9]**.c

    ./interfaces/pthread_condattr_destroy/[0-9]**.c
    ./interfaces/pthread_condattr_getclock/[0-9]**.c
    ./interfaces/pthread_condattr_getpshared/[0-9]**.c
    ./interfaces/pthread_condattr_init/[0-9]**.c
    ./interfaces/pthread_condattr_setclock/[0-9]**.c
    ./interfaces/pthread_condattr_setpshared/[0-9]**.c

    ./interfaces/pthread_getcpuclockid/[0-9]**.c
    ./interfaces/pthread_getcpuclockid/speculative/[0-9]**.c
    ./interfaces/pthread_getschedparam/[0-9]**.c

    ./interfaces/pthread_once/[0-9]**.c

    ./interfaces/pthread_setschedparam/[0-9]**.c

    ./interfaces/pthread_spin_destroy/[0-9]**.c
    ./interfaces/pthread_spin_init/[0-9]**.c
    ./interfaces/pthread_spin_lock/[0-9]**.c
    ./interfaces/pthread_spin_trylock/[0-9]**.c
    ./interfaces/pthread_spin_unlock/[0-9]**.c

    ./interfaces/sem_unlink/[0-9]**.c

)

file(GLOB ALL_PTHREAD_SRC
    ./interfaces/pthread_attr_init/[0-9]**.c
    ./interfaces/pthread_attr_destroy/[0-9]**.c
    ./interfaces/pthread_attr_setstackaddr/[0-9]**.c
    ./interfaces/pthread_attr_getstackaddr/[0-9]**.c
    ./interfaces/pthread_attr_getstacksize/[0-9]**.c
    ./interfaces/pthread_attr_setstacksize/[0-9]**.c
    ./interfaces/pthread_attr_getinheritsched/[0-9]**.c
    ./interfaces/pthread_attr_setinheritsched/[0-9]**.c
    ./interfaces/pthread_attr_getschedpolicy/[0-9]**.c
    ./interfaces/pthread_attr_setschedpolicy/[0-9]**.c
    ./interfaces/pthread_attr_setschedpolicy/speculative/[0-9]**.c
    ./interfaces/pthread_attr_getdetachstate/[0-9]**.c
    ./interfaces/pthread_attr_setdetachstate/[0-9]**.c
    ./interfaces/pthread_create/[0-9]**.c
    ./interfaces/pthread_cancel/[0-9]**.c
    ./interfaces/pthread_testcancel/[0-9]**.c
    ./interfaces/pthread_setcancelstate/[0-9]**.c
    ./interfaces/pthread_setcanceltype/[0-9]**.c
    ./interfaces/pthread_exit/[0-9]**.c
    ./interfaces/pthread_cleanup_push/[0-9]**.c
    ./interfaces/pthread_cleanup_pop/[0-9]**.c
    ./interfaces/pthread_setschedprio/[0-9]**.c
    ./interfaces/pthread_self/[0-9]**.c
    ./interfaces/pthread_equal/[0-9]**.c
    ./interfaces/sched_yield/[0-9]**.c
    ./interfaces/sched_get_priority_max/[0-9]**.c
    ./interfaces/sched_get_priority_min/[0-9]**.c
    ./interfaces/pthread_join/[0-9]**.c
    ./interfaces/pthread_join/speculative/[0-9]**.c
    ./interfaces/pthread_detach/[0-9]**.c
    ./interfaces/pthread_key_create/[0-9]**.c
    ./interfaces/pthread_key_create/speculative/[0-9]**.c
    ./interfaces/pthread_setspecific/[0-9]**.c
    ./interfaces/pthread_getspecific/[0-9]**.c
    ./interfaces/pthread_key_delete/[0-9]**.c
    ./interfaces/pthread_mutexattr_init/[0-9]**.c
    ./interfaces/pthread_mutexattr_destroy/[0-9]**.c
    ./interfaces/pthread_mutexattr_settype/[0-9]**.c
    ./interfaces/pthread_mutexattr_gettype/[0-9]**.c
    ./interfaces/pthread_mutexattr_gettype/speculative/[0-9]**.c
    ./interfaces/pthread_mutexattr_setprotocol/[0-9]**.c
    ./interfaces/pthread_mutexattr_getprotocol/[0-9]**.c
    ./interfaces/pthread_mutex_init/[0-9]**.c
    ./interfaces/pthread_mutex_init/speculative/[0-9]**.c
    ./interfaces/pthread_mutex_destroy/[0-9]**.c
    ./interfaces/pthread_mutex_destroy/speculative/[0-9]**.c
    ./interfaces/pthread_mutex_lock/[0-9]**.c
    ./interfaces/pthread_mutex_trylock/[0-9]**.c
    ./interfaces/pthread_mutex_timedlock/[0-9]**.c
    ./interfaces/pthread_mutex_unlock/[0-9]**.c
    ./interfaces/pthread_rwlock_init/[0-9]**.c
    ./interfaces/pthread_rwlock_destroy/[0-9]**.c
    ./interfaces/pthread_rwlock_rdlock/[0-9]**.c
    ./interfaces/pthread_rwlock_tryrdlock/[0-9]**.c
    ./interfaces/pthread_rwlock_tryrdlock/speculative/[0-9]**.c
    ./interfaces/pthread_rwlock_timedrdlock/[0-9]**.c
    ./interfaces/pthread_rwlock_wrlock/[0-9]**.c
    ./interfaces/pthread_rwlock_trywrlock/[0-9]**.c
    ./interfaces/pthread_rwlock_trywrlock/speculative/[0-9]**.c
    ./interfaces/pthread_rwlock_timedwrlock/[0-9]**.c
    ./interfaces/pthread_rwlock_unlock/[0-9]**.c
    ./interfaces/pthread_attr_getschedparam/[0-9]**.c
    ./interfaces/pthread_attr_setschedparam/[0-9]**.c

    ./interfaces/pthread_attr_getscope/[0-9]**.c
    ./interfaces/pthread_attr_setscope/[0-9]**.c
    ./interfaces/pthread_attr_getstack/[0-9]**.c
    ./interfaces/pthread_attr_setstack/[0-9]**.c

    ./interfaces/pthread_mutex_getprioceiling/[0-9]**.c
    ./interfaces/pthread_mutexattr_getprioceiling/[0-9]**.c
    ./interfaces/pthread_mutexattr_getpshared/[0-9]**.c
    ./interfaces/pthread_mutexattr_setprioceiling/[0-9]**.c
    ./interfaces/pthread_mutexattr_setpshared/[0-9]**.c

    ./interfaces/pthread_rwlockattr_destroy/[0-9]**.c
    ./interfaces/pthread_rwlockattr_getpshared/[0-9]**.c
    ./interfaces/pthread_rwlockattr_init/[0-9]**.c
    ./interfaces/pthread_rwlockattr_setpshared/[0-9]**.c

)

file(GLOB ALL_MALLOC_SRC
    ./interfaces/malloc/malloc_malloc/[0-9]**.c
    ./interfaces/malloc/malloc_calloc/[0-9]**.c
    ./interfaces/malloc/malloc_memalign/[0-9]**.c
    ./interfaces/malloc/malloc_realloc/[0-9]**.c
    ./interfaces/malloc/malloc_reallocarray/[0-9]**.c
    ./interfaces/malloc/malloc_usable_size/[0-9]**.c
)

list(REMOVE_ITEM ALL_TIME_SRC 
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/clock_settime/4-2.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/clock_settime/speculative/4-3.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/clock_settime/speculative/4-4.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/nanosleep/1-3.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/nanosleep/3-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/nanosleep/5-2.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/nanosleep/7-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/timer_create/1-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/timer_create/3-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/timer_create/8-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/timer_create/9-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/timer_create/speculative/2-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/timer_create/speculative/5-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/timer_delete/1-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/timer_delete/speculative/5-2.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/timer_settime/1-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/timer_settime/1-2.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/timer_settime/2-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/timer_settime/3-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/timer_settime/3-2.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/timer_settime/3-3.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/timer_settime/5-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/timer_settime/5-2.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/timer_settime/5-3.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/timer_settime/6-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/timer_settime/9-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/timer_settime/9-2.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/clock_settime/4-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/clock_settime/5-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/clock_settime/5-2.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/clock_settime/7-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/clock_settime/7-2.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/clock_settime/8-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/nanosleep/7-2.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/timer_create/10-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/timer_create/11-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/timer_create/7-1.c

    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/clock_getcpuclockid/2-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/clock_nanosleep/1-3.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/clock_nanosleep/1-4.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/clock_nanosleep/1-5.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/clock_nanosleep/2-2.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/clock_nanosleep/2-3.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/clock_nanosleep/9-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/clock_nanosleep/10-1.c
    
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/strftime/1-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/strftime/2-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/timer_getoverrun/1-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/timer_getoverrun/2-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/timer_getoverrun/2-2.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/timer_getoverrun/2-3.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/timer_getoverrun/3-1.c

    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/timer_getoverrun/speculative/6-2.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/timer_getoverrun/speculative/6-3.c

    # 尚未实现fork，kill，wait，先注释
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/nanosleep/1-2.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/nanosleep/3-2.c
)

list(REMOVE_ITEM ALL_SEM_SRC 
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/sem_init/3-2.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/sem_init/3-3.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/sem_init/6-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/sem_init/7-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/sem_open/5-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/sem_post/5-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/sem_timedwait/2-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/sem_open/3-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/sem_post/5-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/sem_post/6-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/sem_post/8-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/sem_post/8-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/sem_post/8-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/sem_post/8-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/sem_timedwait/9-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/sem_wait/13-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/sem_wait/7-1.c


    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_barrier_init/4-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_barrier_wait/1-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_barrier_wait/3-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_barrier_wait/3-2.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_barrier_wait/6-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_barrierattr_getpshared/2-1.c

    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_cond_broadcast/1-2.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_cond_broadcast/2-3.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_cond_broadcast/4-2.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_cond_destroy/2-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_cond_init/1-3.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_cond_init/4-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_cond_init/4-2.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_cond_signal/1-2.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_cond_signal/4-2.c

    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_cond_timedwait/2-3.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_cond_timedwait/2-4.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_cond_timedwait/2-5.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_cond_timedwait/2-6.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_cond_timedwait/2-7.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_cond_timedwait/4-2.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_cond_timedwait/4-3.c

    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_cond_wait/2-2.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_cond_wait/2-3.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_cond_wait/3-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_cond_wait/4-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_condattr_getpshared/1-2.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_condattr_setclock/1-2.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_condattr_setpshared/1-2.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_getschedparam/1-3.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_getschedparam/4-1.c

    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_once/6-1.c
    
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_setschedparam/5-1.c

    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_spin_init/2-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_spin_init/2-2.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_spin_lock/1-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_spin_lock/3-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_spin_lock/3-2.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_spin_trylock/1-1.c

    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/sem_unlink/2-2.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/sem_unlink/3-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/sem_unlink/5-1.c
)

list(REMOVE_ITEM ALL_PTHREAD_SRC 
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_attr_setstacksize/2-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_create/3-2.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_exit/6-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_mutex_init/5-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_mutex_trylock/1-2.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_mutex_trylock/2-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_mutex_trylock/4-2.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/sched_yield/1-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_key_create/speculative/5-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_mutex_init/speculative/5-2.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_create/1-3.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_mutexattr_settype/2-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_attr_setschedparam/1-2.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_attr_setschedparam/1-3.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_attr_setschedparam/1-4.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_attr_setinheritsched/2-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_attr_setinheritsched/2-2.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_attr_setinheritsched/2-3.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_attr_setinheritsched/2-3.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_attr_setinheritsched/2-4.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_attr_setinheritsched/2-4.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_cancel/3-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_cancel/5-2.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_cancel/5-2.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_cancel/5-2.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_create/10-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_create/11-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_create/1-3.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_create/14-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_create/14-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_create/14-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_create/14-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_create/8-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_create/8-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_create/8-2.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_create/8-2.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_detach/4-3.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_detach/4-3.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_detach/4-3.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_detach/4-3.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_detach/4-3.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_equal/2-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_join/6-3.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_join/6-3.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_join/6-3.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_mutex_init/5-3.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_mutex_init/5-3.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_mutex_lock/3-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_mutex_lock/3-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_mutex_lock/5-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_mutex_lock/5-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_mutex_trylock/4-3.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_mutex_trylock/4-3.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_mutex_trylock/4-3.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_mutexattr_init/1-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_mutexattr_settype/2-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_rwlock_rdlock/2-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_rwlock_rdlock/2-2.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_rwlock_rdlock/4-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_rwlock_rdlock/4-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_rwlock_timedrdlock/6-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_rwlock_timedrdlock/6-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_rwlock_timedrdlock/6-2.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_rwlock_timedrdlock/6-2.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_rwlock_timedwrlock/6-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_rwlock_timedwrlock/6-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_rwlock_timedwrlock/6-2.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_rwlock_timedwrlock/6-2.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_rwlock_unlock/3-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_rwlock_wrlock/2-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_rwlock_wrlock/2-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_setschedprio/1-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_attr_getstacksize/1-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_attr_setstacksize/1-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_attr_setstacksize/4-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_create/1-4.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_create/1-5.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_create/1-5.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_create/1-6.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_create/1-6.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_create/15-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_detach/1-2.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_detach/2-2.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_exit/1-2.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_exit/2-2.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_exit/3-2.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_exit/4-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_exit/5-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_exit/6-2.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_join/1-2.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_join/4-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_mutex_destroy/2-2.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_mutex_destroy/5-2.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_mutex_lock/1-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_mutex_unlock/2-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_rwlock_rdlock/2-3.c

    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_attr_getstack/1-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_attr_setstack/1-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_attr_setstack/2-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_attr_setstack/4-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_attr_setstack/6-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_attr_setstack/7-1.c

    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_rwlockattr_getpshared/2-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pthread_rwlockattr_setpshared/1-1.c

)

list(APPEND ALL_SIGNAL_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/sigwait/sigwait_test.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/sigprocmask/sigprocmask_test.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/sigaction/sigaction_test.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/sigfillset/sigfillset_test.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/sigaddset/sigaddset_test.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/sigdelset/sigdelset_test.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/sigemptyset/sigemptyset_test.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/signal/signal_test.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/pause/pause_test.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/setitimer/1_1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/setitimer/1_2.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/getitimer/1_1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/getitimer/1_2.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/kill/1-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/kill/2-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/raise/1-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/raise/2-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/raise/4-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/raise/6-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/raise/7-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/sigsuspend/7-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/sigdelset/1-3.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/sigdelset/1-4.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/sigdelset/2-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/sighold/1-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/sighold/2-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/sigignore/1-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/sigignore/4-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/sigignore/6-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/sigignore/6-2.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/sigismember/3-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/sigismember/4-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/signal/1-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/signal/2-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/signal/3-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/signal/5-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/signal/6-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/signal/7-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/sigpause/1-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/sigpause/1-2.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/sigpause/2-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/sigpause/3-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/sigpause/4-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/sigrelse/1-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/sigrelse/2-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/sigpending/1-1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/sigpending/1-2.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/sigpending/1-3.c
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/sigpending/2-1.c
)

list(APPEND ALL_PROXY_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/./interfaces/proxy/1-1.c
)

file(GLOB_RECURSE ALL_RWFLAG_SRC ${CMAKE_CURRENT_SOURCE_DIR}/definitions/rwflag/*.c)

file(GLOB_RECURSE ALL_EXIT_SRC ${CMAKE_CURRENT_SOURCE_DIR}/interfaces/exit/*.c)

if(${CPU_TYPE} STREQUAL "x86_64" OR ${CPU_TYPE} STREQUAL "rk3568_jailhouse" OR ${CPU_TYPE} STREQUAL "rk3588" OR ${CPU_TYPE} STREQUAL "ascend310b" OR ${CPU_TYPE} STREQUAL "hi3093" OR ${CPU_TYPE} STREQUAL "e2000q")
    include_directories(../include)
elseif(${CPU_TYPE} STREQUAL "m4")
    list(APPEND OBJS 
        $<TARGET_OBJECTS:bsp>
        $<TARGET_OBJECTS:config>
    )
endif()

if (${APP} STREQUAL "UniPorton_test_posix_time_interface")
    set(BUILD_APP "UniPorton_test_posix_time_interface")
    set(ALL_SRC runTimeTest.c ${ALL_TIME_SRC})
elseif(${APP} STREQUAL "UniPorton_test_posix_thread_sem_interface")
    set(BUILD_APP "UniPorton_test_posix_thread_sem_interface")
    set(ALL_SRC runThreadSemTest.c ${ALL_SEM_SRC})
elseif(${APP} STREQUAL "UniPorton_test_posix_thread_pthread_interface")
    set(BUILD_APP "UniPorton_test_posix_thread_pthread_interface")
    set(ALL_SRC runThreadPthreadTest.c ${ALL_PTHREAD_SRC})
elseif (${APP} STREQUAL "UniPorton_test_posix_malloc_interface")
    set(BUILD_APP "UniPorton_test_posix_malloc_interface")
    set(ALL_SRC runMallocTest.c ${ALL_MALLOC_SRC})
elseif(${APP} STREQUAL "UniPorton_test_posix_signal_interface")
    set(BUILD_APP "UniPorton_test_posix_signal_interface")
    set(ALL_SRC runSignalTest.c ${ALL_SIGNAL_SRC})
elseif(${APP} STREQUAL "UniPorton_test_proxy_posix_interface")
    set(BUILD_APP "UniPorton_test_proxy_posix_interface")
    set(ALL_SRC runProxyTest.c ${ALL_PROXY_SRC})
elseif(${APP} STREQUAL "UniPorton_test_posix_exit_interface")
    set(BUILD_APP "UniPorton_test_posix_exit_interface")
    set(ALL_SRC runExitTest.c ${ALL_EXIT_SRC})
elseif(${APP} STREQUAL "UniPorton_test_posix_rwflag_interface")
    set(BUILD_APP "UniPorton_test_posix_rwflag_interface")
    set(ALL_SRC runRwflagTest.c ${ALL_RWFLAG_SRC})
else()
    return()
endif()

if (${APP} STREQUAL "UniPorton_test_posix_exit_interface")
    set(ALL_EXE "")
    foreach(EXIT_SRC ${ALL_EXIT_SRC})
        string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}/interfaces/exit/" "" FILE_NAME ${EXIT_SRC})
        string(REPLACE "/" "_" TMP_FILE_NAME1 ${FILE_NAME})
        string(REPLACE "-" "_" TMP_FILE_NAME2 ${TMP_FILE_NAME1})
        string(REPLACE ".c" "" RUN_TEST ${TMP_FILE_NAME2})
        if(${CPU_TYPE} STREQUAL "x86_64")
            add_library("UniPorton_test_posix_exit_${RUN_TEST}" OBJECT ${ALL_SRC})
        elseif(${CPU_TYPE} STREQUAL "m4")
            add_executable("UniPorton_test_posix_exit_${RUN_TEST}" ${EXIT_SRC} runExitTest.c ${CXX_LIB} ${OBJS})
            target_link_libraries("UniPorton_test_posix_exit_${RUN_TEST}" PUBLIC testsuite_support)
        endif()
        string(TOUPPER ${RUN_TEST} DEF_TEST)
        target_compile_definitions("UniPorton_test_posix_exit_${RUN_TEST}" PRIVATE ${DEF_TEST})
        list(APPEND ALL_EXE "UniPorton_test_posix_exit_${RUN_TEST}")
    endforeach()
    if(${CPU_TYPE} STREQUAL "x86_64")
        add_library(posixTest OBJECT ${ALL_SRC})
    elseif(${CPU_TYPE} STREQUAL "m4")
        add_custom_target(${BUILD_APP} ALL COMMAND echo 'build done' DEPENDS  ${ALL_EXE})
    endif()
else()
    if(${CPU_TYPE} STREQUAL "x86_64" OR
    ${CPU_TYPE} STREQUAL "ascend310b")
        add_library(posixTest OBJECT ${ALL_SRC})
    elseif(${CPU_TYPE} STREQUAL "m4")
        add_executable(${BUILD_APP} ${ALL_SRC} ${CXX_LIB} ${OBJS})
        target_link_libraries(${BUILD_APP} PUBLIC testsuite_support)
    elseif(${CPU_TYPE} STREQUAL "armv8")
        add_library(posixTest OBJECT ${ALL_SRC})
    elseif(${CPU_TYPE} STREQUAL "hi3093")
        add_library(posixTest OBJECT ${ALL_SRC})
    endif()
endif()