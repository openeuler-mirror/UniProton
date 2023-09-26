#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include <semaphore.h>
#include <pthread.h>
#include <stdbool.h>
#include <errno.h>
#include "prt_typedef.h"
#include "prt_timer.h"
#include "tmacros.h"

pthread_t latency_task, display_task;
sem_t display_sem;
static int run;

#define PERIODIC_NS     1000000
#define HOST_HZ_TO_US   1799
#define NS_PER_US       1000
#define NS_PER_MS       1000000

struct latency_info_t {
    int value_0_100[100];
    int out_of_range;
    int worst_value;
    U64 timestamp_expected;
    U64 timestamp_now;
    U64 diff_ns;
} latency_info;

static __inline __attribute__((always_inline)) U64 __tsc(void)
{
    U32 low, high;

    __asm__ __volatile__ ("mfence");
    __asm__ __volatile__ ("rdtsc" : "=a" (low), "=d" (high));
    return (U64)high << 32 | low;
}

static U64 current_time_ns(void)
{
    U64 ts;

    ts = __tsc();
    return (ts * NS_PER_US) / HOST_HZ_TO_US;
}

static struct timespec to_ts(U64 ns)
{
    struct timespec ts;
    ts.tv_sec = ns / 1000000000ULL;
    ts.tv_nsec = ns % 1000000000ULL;
    return ts;
}

/* nanosleep接口优化 */
#define PTHREAD_OP_FAIL (-1)
#include "prt_tick.h"
#include "prt_sys.h"

extern struct TickModInfo g_tickModInfo;

int TS_nanosleep(const struct timespec *rqtp, struct timespec *rmtp)
{
    U64 nanosec;
    U64 tick;
    U64 cur_cycle = __tsc();
    U64 except_cycle;
    const U32 nsPerTick = OS_SYS_NS_PER_SECOND / g_tickModInfo.tickPerSecond;

    nanosec = (U64)rqtp->tv_sec * OS_SYS_NS_PER_SECOND + (U64)rqtp->tv_nsec;
    except_cycle = cur_cycle + ((nanosec * HOST_HZ_TO_US) / NS_PER_US);
    tick = nanosec / nsPerTick;

    if (tick >= U32_MAX) {
        errno = EINVAL;
        return PTHREAD_OP_FAIL;
    }

    if (PRT_TaskDelay((U32)tick) == OS_OK) {
        if (rmtp != NULL) {
            rmtp->tv_sec = rmtp->tv_nsec = 0;
        }
        cur_cycle = __tsc();
        while (cur_cycle < except_cycle) {
            cur_cycle = __tsc();
        }
        return OS_OK;
    }

    return PTHREAD_OP_FAIL;
}
/* nanosleep接口优化 end */

static void *latency_func(void* arg)
{
    U64 diff_ns, diff_us;
    U32 count = 0;
    int warmup = 0;
    U64 timestamp_expected;
    struct timespec wait_ns = to_ts(PERIODIC_NS);

    while (run) {
        timestamp_expected = current_time_ns() + PERIODIC_NS;
        TS_nanosleep(&wait_ns, NULL);
        ++count;
        if (warmup) {
            if (count * (PERIODIC_NS / NS_PER_MS) > 3000) { //warmup 3 secs
                timestamp_expected = current_time_ns() + PERIODIC_NS;
                warmup = 0;
            }
            continue;
        }

        latency_info.timestamp_now = current_time_ns();
        latency_info.timestamp_expected = timestamp_expected;
        diff_ns = latency_info.timestamp_now - latency_info.timestamp_expected;
        latency_info.diff_ns = diff_ns;
        diff_us = diff_ns / NS_PER_US;

        if (diff_us < 100) {
            ++latency_info.value_0_100[diff_us];
        } else {
            ++latency_info.out_of_range;
        }

        if (diff_us > latency_info.worst_value) {
            latency_info.worst_value = diff_us;
        }

        timestamp_expected += PERIODIC_NS;

        if (!((count * (PERIODIC_NS / NS_PER_MS)) % 1000)) { // display per sec
            sem_post(&display_sem);
        }
        
        if (count >= 10000) { // 18000000
			sem_post(&display_sem);
			run = false;
		}
    }
    
    return 0;
}

static void *display_func(void *arg)
{
    while (run) {
        sem_wait(&display_sem);
        int i;
        for(i = 0; i < 100; i++) {
            printf("%d: %d\n", i, latency_info.value_0_100[i]);
        }
        printf(">=100: %d\n", latency_info.out_of_range);
        printf("the worst delay: %d\n\n", latency_info.worst_value);
        
        printf("timestamp_expected: %llu, timestamp_now: %llu\n", 
            latency_info.timestamp_expected, 
            latency_info.timestamp_now);

        // printf("press 'enter' key to exit\n\n");
    }
    
    return 0;
}

int Init(uintptr_t param1, uintptr_t param2, uintptr_t param3, uintptr_t param4)
{
    U32 ret;
    struct sched_param schedparam = { 0 };
    
    memset(&latency_info, 0, sizeof(struct latency_info_t));
    
    sem_init(&display_sem, 0, 0);
    
    pthread_attr_t latency_attr, display_attr;
    pthread_attr_init(&latency_attr);
    pthread_attr_init(&display_attr);
    schedparam.sched_priority = 0; // highest
    ret = pthread_attr_setschedparam(&latency_attr, &schedparam);
    if (ret != 0) {
        printf("pthread_attr_setschedparam err: %x\n", ret);
        return -1;
    }
    
    pthread_attr_setschedpolicy(&latency_attr, SCHED_FIFO);
    
    run = 1;

    pthread_create(&latency_task, &latency_attr, latency_func, NULL);
    pthread_create(&display_task, &display_attr, display_func, NULL);
    
    // run = 0;
    // sem_post(&display_sem);
    
    // pthread_join(latency_task, NULL);
    // pthread_join(display_task, NULL);
    
    // sem_destroy(&display_sem);
    
    return 0;
}