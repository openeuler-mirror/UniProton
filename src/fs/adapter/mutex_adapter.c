#include "nuttx/mutex.h"
#include "prt_sem_external.h"

int nxmutex_init(FAR mutex_t *mutex)
{
    return pthread_mutex_init(mutex, NULL);
}

int nxmutex_lock(FAR mutex_t *mutex)
{
    return pthread_mutex_lock(mutex);
}

int nxmutex_unlock(FAR mutex_t *mutex)
{
    return pthread_mutex_unlock(mutex);
}

int nxmutex_destroy(FAR mutex_t *mutex)
{
    return pthread_mutex_destroy(mutex);
}

int nxrmutex_init(FAR rmutex_t *rmutex)
{
    pthread_mutexattr_t attr = {
        .protocol = PTHREAD_PRIO_NONE,
        .type = PTHREAD_MUTEX_RECURSIVE
    };
    return pthread_mutex_init(rmutex, &attr);
}

int nxrmutex_lock(FAR rmutex_t *rmutex)
{
    return pthread_mutex_lock(rmutex);
}

int nxrmutex_unlock(FAR rmutex_t *rmutex)
{
    return pthread_mutex_unlock(rmutex);
}

void nxmutex_reset(FAR mutex_t *mutex)
{
    SemHandle semHandle = mutex->mutex_sem;
    struct TagSemCb *semCb = NULL;
    semCb = GET_SEM(semHandle);
    int count = 1;

    PRT_TaskLock();
    uintptr_t intSave = OsIntLock();

    while (semCb->semCount == 0 && count > 0) {
        PRT_SemPost(mutex->mutex_sem);
        count--;
    }

    if (semCb->semCount == 0) {
        semCb->semCount = count;
    }

    OsIntRestore(intSave);
    PRT_TaskUnlock();
}

int nxrmutex_destroy(FAR rmutex_t *rmutex)
{
    return pthread_mutex_destroy(rmutex);
}

bool nxmutex_is_hold(FAR mutex_t *mutex)
{
    return GET_SEM(mutex->mutex_sem)->semOwner == RUNNING_TASK->taskPid;
}

bool nxrmutex_is_hold(FAR rmutex_t *rmutex)
{
    return GET_SEM(rmutex->mutex_sem)->semOwner == RUNNING_TASK->taskPid;
}

int nxrmutex_breaklock(FAR rmutex_t *rmutex, FAR unsigned int *count)
{
    int ret = 0;
    *count = 0;
    if (nxrmutex_is_hold(rmutex)) {
        return ret;
    }
    *count = GET_SEM(rmutex->mutex_sem)->semCount;
    ret = nxmutex_unlock(rmutex);
    if (ret != 0) {
        GET_SEM(rmutex->mutex_sem)->semCount = *count;
    }

    return ret;
}

int nxrmutex_trylock(FAR rmutex_t *rmutex)
{
    return pthread_mutex_trylock(rmutex);
}

int nxrmutex_restorelock(FAR rmutex_t *rmutex, unsigned int count)
{
    int ret = OK;
    if (count != 0) {
        ret = nxmutex_lock(rmutex);
        if (ret != 0) {
            GET_SEM(rmutex->mutex_sem)->semCount = count;
        }
    }

    return ret;
}

int nxmutex_timedlock(FAR mutex_t *mutex, unsigned int timeout)
{
    struct timespec time;
    time.tv_sec = 0;
    time.tv_nsec = timeout * NSEC_PER_MSEC;
    return pthread_mutex_timedlock(mutex, &time);
}