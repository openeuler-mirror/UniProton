#include "stdio_impl.h"
#include "pthread_impl.h"
#include <limits.h>
#if defined(OS_OPTION_NUTTX_VFS)
#include "prt_task_external.h"
#endif

void __do_orphaned_stdio_locks()
{
#ifndef OS_OPTION_NUTTX_VFS
    FILE *f;
    for (f=__pthread_self()->stdio_locks; f; f=f->next_locked)
        a_store(&f->lock, 0x40000000);
#endif /* OS_OPTION_NUTTX_VFS */
}

void __unlist_locked_file(FILE *f)
{
#ifdef OS_OPTION_NUTTX_VFS
    if (f->lockcount) {
        if (f->next_locked) f->next_locked->prev_locked = f->prev_locked;
        if (f->prev_locked) f->prev_locked->next_locked = f->next_locked;
        else {
            uintptr_t intSave = OsIntLock();
            RUNNING_TASK->stdio_locks = f->next_locked;
            OsIntRestore(intSave);
        }
    }
#endif /* OS_OPTION_NUTTX_VFS */
}

void __register_locked_file(FILE *f, pthread_t self)
{
#ifdef OS_OPTION_NUTTX_VFS
    f->owner = (long)self;
    f->lockcount = 1;
    f->prev_locked = 0;
    f->next_locked = RUNNING_TASK->stdio_locks;
    if (f->next_locked) f->next_locked->prev_locked = f;
    RUNNING_TASK->stdio_locks = f;
#endif /* OS_OPTION_NUTTX_VFS */
}

int ftrylockfile(FILE *f)
{
#if defined(OS_OPTION_NUTTX_VFS)
    long owner = f->owner;
    uintptr_t intSave = OsIntLock();
    pthread_t self = pthread_self();

    if (owner == (long)self) {
        if (f->lockcount == LONG_MAX) {
            OsIntRestore(intSave);
            return -1;
        }
        f->lockcount++;
        OsIntRestore(intSave);
        return 0;
    }
    if (pthread_mutex_trylock(&f->mutex) != 0) {
        OsIntRestore(intSave);
        return -1;
    }
    __register_locked_file(f, self);
    OsIntRestore(intSave);
    return 0;
#endif /* OS_OPTION_NUTTX_VFS */
    return -1;
}
