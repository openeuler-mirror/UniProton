#include "pthread_impl.h"
#include <signal.h>
#include "prt_signal.h"
#include "prt_signal_external.h"

static const unsigned long all_mask[] = {
#if ULONG_MAX == 0xffffffff && _NSIG > 65
    -1UL, -1UL, -1UL, -1UL
#elif ULONG_MAX == 0xffffffff || _NSIG > 65
    -1UL, -1UL
#else
    -1UL
#endif
};

static const unsigned long app_mask[] = {
#if ULONG_MAX == 0xffffffff
#if _NSIG == 65
    0x7fffffff, 0xfffffffc
#else
    0x7fffffff, 0xfffffffc, -1UL, -1UL
#endif
#else
#if _NSIG == 65
    0xfffffffc7fffffff
#else
    0xfffffffc7fffffff, -1UL
#endif
#endif
};

void __block_all_sigs(void *set)
{
    sigprocmask(SIG_BLOCK, (sigset_t *)&all_mask, set);
}

void __block_app_sigs(void *set)
{
    sigprocmask(SIG_BLOCK, (sigset_t *)&app_mask, set);
}

void __restore_sigs(void *set)
{
    signalSet prtSet = ((sigset_t *)set)->__bits[0];
    uintptr_t intSave = OsIntLock();
    struct TagTskCb *runTsk = RUNNING_TASK;
    runTsk->sigMask &= ~prtSet;
    OsIntRestore(intSave);
}
