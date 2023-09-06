#ifndef _LINUX_RTMUTEX_H
#define _LINUX_RTMUTEX_H

#include "pthread.h"

struct lock_class_key {};

/**
 * The rt_mutex structure
 *
 * @wait_lock:    spinlock to protect the structure
 * @waiters:    rbtree root to enqueue waiters in priority order;
 *              caches top-waiter (leftmost node).
 * @owner:    the mutex owner
 */
struct rt_mutex {
    unsigned char type;
    unsigned char magic;
    unsigned short mutex_sem;
};

void __rt_mutex_init(struct rt_mutex *lock, const char *name, struct lock_class_key *key);
void rt_mutex_lock(struct rt_mutex *lock);
void rt_mutex_unlock(struct rt_mutex *lock);

#define rt_mutex_init(mutex)            __rt_mutex_init(mutex, NULL, NULL)

#endif