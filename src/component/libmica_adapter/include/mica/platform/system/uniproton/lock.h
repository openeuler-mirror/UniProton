/* SPDX-License-Identifier: MulanPSL-2.0 */
#ifndef MICA_UNIPROTON_LOCK_H
#define MICA_UNIPROTON_LOCK_H
#include <mica/platform/time.h>
#include <mica/platform/system/uniproton/sem.h>
typedef SemHandle mica_lock_t;
static inline int mica_lock_init(mica_lock_t *lock)
{
    return lock ? mica_uniproton_sem_error(PRT_SemMutexCreate(lock)) : -EINVAL;
}
static inline int mica_lock_acquire(mica_lock_t *lock, mica_deadline_t deadline)
{
    if (!lock)
        return -EINVAL;
    for (;;) {
        uint64_t now = mica_now_ns();
        uint32_t ticks = now >= deadline ? 0 : mica_timeout_ticks(
            deadline - now, MICA_OS_TICK_HZ, MICA_OS_MAX_FINITE_TICKS);
        int ret = mica_uniproton_sem_error(PRT_SemPend(*lock, ticks));
        if (ret == -EAGAIN && !ticks)
            return -ETIMEDOUT; /* Only the expired zero-time attempt may miss. */
        if (ret != -ETIMEDOUT)
            return ret; /* Unexpected positive-time EAGAIN is not a retry. */
        if (mica_now_ns() >= deadline)
            return -ETIMEDOUT;
    }
}
static inline int mica_lock_release(mica_lock_t *lock)
{
    return lock ? mica_uniproton_sem_error(PRT_SemPost(*lock)) : -EINVAL;
}
static inline int mica_lock_destroy(mica_lock_t *lock)
{
    return lock ? mica_uniproton_sem_error(PRT_SemDelete(*lock)) : -EINVAL;
}
#endif
