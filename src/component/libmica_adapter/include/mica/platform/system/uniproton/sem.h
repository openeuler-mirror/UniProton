/* SPDX-License-Identifier: MulanPSL-2.0 */
#ifndef MICA_UNIPROTON_SEM_H
#define MICA_UNIPROTON_SEM_H

#include <errno.h>
#include <stdint.h>
#include <mica/platform/time.h>
#include "prt_sem.h"

typedef SemHandle mica_sem_t;

#define MICA_SEM_TICK_HZ MICA_OS_TICK_HZ
#define MICA_SEM_MAX_FINITE_TICKS MICA_OS_MAX_FINITE_TICKS

static inline int mica_uniproton_sem_error(U32 error)
{
    switch (error)
    {
    case OS_OK:
        return 0;
    case OS_ERRNO_SEM_NO_MEMORY:
        return -ENOMEM;
    case OS_ERRNO_SEM_ALL_BUSY:
        return -ENOSPC;
    case OS_ERRNO_SEM_INVALID:
    case OS_ERRNO_SEM_PTR_NULL:
    case OS_ERRNO_SEM_INPUT_ERROR:
    case OS_ERRNO_SEM_INFO_NULL:
    case OS_ERRNO_SEM_COUNT_GET_PTR_NULL:
        return -EINVAL;
    case OS_ERRNO_SEM_PEND_INTERR:
    case OS_ERRNO_SEM_PEND_IN_LOCK:
    case OS_ERRNO_SEM_MUTEX_NOT_OWNER_POST:
    case OS_ERRNO_SEM_MUTEX_POST_INTERR:
        return -EPERM;
    case OS_ERRNO_SEM_UNAVAILABLE:
        return -EAGAIN;
    case OS_ERRNO_SEM_TIMEOUT:
        return -ETIMEDOUT;
    case OS_ERRNO_SEM_OVERFLOW:
        return -EOVERFLOW;
    case OS_ERRNO_SEM_PENDED:
    case OS_ERRNO_SEM_MUTEX_HOLDING:
        return -EBUSY;
    case OS_ERRNO_SEM_INPUT_BUF_NOT_ENOUGH:
        return -ENOSPC;
    case OS_ERRNO_SEM_REG_ERROR:
        return -EIO;
    default:
        return -EIO;
    }
}

static inline int mica_sem_init(mica_sem_t *sem, unsigned int initial)
{
    if (!sem || initial > OS_SEM_COUNT_MAX)
        return -EINVAL;

    return mica_uniproton_sem_error(PRT_SemCreate(initial, sem));
}

static inline int mica_sem_post(mica_sem_t *sem)
{
    if (!sem)
        return -EINVAL;

    /* Native counting post is IRQ-safe; overflow coalesces a pending wake. */
    U32 error = PRT_SemPost(*sem);

    return error == OS_ERRNO_SEM_OVERFLOW ? 0 : mica_uniproton_sem_error(error);
}

static inline int mica_sem_pend(mica_sem_t *sem, uint32_t ticks)
{
    if (!sem || ticks > MICA_SEM_MAX_FINITE_TICKS)
        return -EINVAL;

    return mica_uniproton_sem_error(PRT_SemPend(*sem, ticks));
}

static inline int mica_sem_destroy(mica_sem_t *sem)
{
    if (!sem)
        return -EINVAL;

    /* Handle zero is valid. The caller owns the separate acquired flag. */
    return mica_uniproton_sem_error(PRT_SemDelete(*sem));
}

#endif
