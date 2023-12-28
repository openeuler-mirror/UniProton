#include "stdio_impl.h"
#include "pthread_impl.h"
#include "prt_task_external.h"

void flockfile(FILE *f)
{
    if (!ftrylockfile(f)) return;
    __lockfile(f);
    uintptr_t intSave = OsIntLock();
    __register_locked_file(f, pthread_self());
    OsIntRestore(intSave);
}
