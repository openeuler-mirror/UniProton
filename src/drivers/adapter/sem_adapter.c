#include <unistd.h>
#include <nuttx/semaphore.h>
#include "prt_posix_internal.h"

int nxsem_init(FAR sem_t *sem, int pshared, unsigned int value)
{
    return sem_init(sem, pshared, value);
}

int nxsem_destroy(FAR sem_t *sem)
{
    return sem_destroy(sem);
}

int nxsem_wait(FAR sem_t *sem)
{
    return (sem_wait(sem) == 0) ? 0 : -errno;
}

int nxsem_trywait(FAR sem_t *sem)
{
    return (sem_trywait(sem) == 0) ? 0 : -errno;
}

int nxsem_tickwait(FAR sem_t *sem, uint32_t delay)
{
    U32 ret;
    if (delay == 0) {
        return sem_trywait(sem);
    }

    ret = PRT_SemPend(sem->semHandle, delay);
    if (ret == OS_ERRNO_SEM_TIMEOUT) {
        return -ETIMEDOUT;
    } else if (ret != OS_OK) {
        return -EINVAL;
    }

    return OS_OK;
}

int nxsem_post(FAR sem_t *sem)
{
    return (sem_post(sem) == 0) ? 0 : -errno;
}

int nxsem_get_value(FAR sem_t *sem, FAR int *sval)
{
    return sem_getvalue(sem, sval);
}

int nxsem_reset(FAR sem_t *sem, int16_t count)
{
    PRT_TaskLock();
    uintptr_t intSave = OsIntLock();

    while (sem->refCount == 0 && count > 0) {
        nxsem_post(sem);
        count--;
    }

    if (sem->refCount == 0) {
        sem->refCount = count;
    }

    OsIntRestore(intSave);
    PRT_TaskUnlock();

    return OS_OK;
}

int nxsig_usleep(useconds_t usec)
{
    return usleep(usec);
}
