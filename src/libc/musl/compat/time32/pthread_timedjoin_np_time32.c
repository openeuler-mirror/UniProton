#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include "time32.h"
#include <time.h>
#include <pthread.h>

int __pthread_timedjoin_np_time32(pthread_t t, void **res, const struct timespec32 *at32)
{
    return pthread_timedjoin_np(t, res, !at32 ? 0 : (&(struct timespec){
        .tv_sec = at32->tv_sec, .tv_nsec = at32->tv_nsec}));
}
