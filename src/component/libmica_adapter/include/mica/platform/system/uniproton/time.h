/* SPDX-License-Identifier: MulanPSL-2.0 */
#ifndef MICA_UNIPROTON_TIME_H
#define MICA_UNIPROTON_TIME_H
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include "prt_config.h"
#include "prt_sem.h" /* OS_WAIT_FOREVER is defined here, not prt_typedef.h. */
#define MICA_OS_TICK_HZ OS_TICK_PER_SECOND
#define MICA_OS_MAX_FINITE_TICKS (OS_WAIT_FOREVER - 1u)
static inline uint64_t mica_now_ns(void)
{
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0)
        abort(); /* No valid deadline can be manufactured from a failed clock. */
    return (uint64_t)ts.tv_sec * UINT64_C(1000000000) + (uint64_t)ts.tv_nsec;
}
#endif
