/* SPDX-License-Identifier: MulanPSL-2.0 */
#ifndef MICA_UNIPROTON_THREAD_H
#define MICA_UNIPROTON_THREAD_H
#ifndef _GNU_SOURCE
#error "Compile UniProton consumers with -D_GNU_SOURCE before all system headers"
#endif
#include <errno.h>
#include <pthread.h>
#include "prt_task.h"
#include <mica/platform/time.h>
typedef pthread_t mica_thread_t;
typedef void *(*mica_thread_entry_t)(void *);
static inline int mica_thread_create(mica_thread_t *out, mica_thread_entry_t entry, void *arg)
{
    if (!out || !entry)
        return -EINVAL;
    pthread_attr_t attr;
    int error = pthread_attr_init(&attr);
    if (error)
        return -error;
    /* NULL attr uses mutable pthread_setattr_default_np state on UniProton. */
    error = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    if (!error)
        error = pthread_create(out, &attr, entry, arg);
    (void)pthread_attr_destroy(&attr);
    return -error;
}
static inline int mica_thread_is_self(mica_thread_t thread)
{
    return pthread_equal(pthread_self(), thread);
}
/* Sole operation owner joins once; acquired/joined progress belongs to caller. */
static inline int mica_thread_join(mica_thread_t thread, mica_deadline_t deadline)
{
    if (mica_thread_is_self(thread))
        return -EDEADLK;
    for (;;) {
        int error = pthread_tryjoin_np(thread, NULL);
        if (!error)
            return 0;
        /* UniProton tryjoin reports EDEADLK for a zero-time join-sem miss.
         * Self has been excluded; caller owns the unique joinable handle. */
        if (error != EDEADLK)
            return -error;
        if (mica_now_ns() >= deadline)
            return -ETIMEDOUT;
        if (PRT_TaskDelay(1) != OS_OK)
            return -EIO;
    }
}
#endif
