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