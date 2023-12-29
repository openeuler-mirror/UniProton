#include "stdio_impl.h"
#include "pthread_impl.h"

#ifdef __GNUC__
__attribute__((__noinline__))
#endif
static int locking_putc(int c, FILE *f)
{
#ifdef OS_OPTION_NUTTX_VFS
    FLOCK(f);
    c = putc_unlocked(c, f);
    FUNLOCK(f);
    return c;
#else
    if (a_cas(&f->lock, 0, MAYBE_WAITERS-1)) __lockfile(f);
    c = putc_unlocked(c, f);
    if (a_swap(&f->lock, 0) & MAYBE_WAITERS)
        __wake(&f->lock, 1, 1);
    return c;
#endif /* OS_OPTION_NUTTX_VFS */
}

static inline int do_putc(int c, FILE *f)
{
#ifdef OS_OPTION_NUTTX_VFS
    long owner = f->owner;
    pthread_t self = pthread_self();
    if (owner == (long)self) {
        return putc_unlocked(c, f);
    }
    return locking_putc(c, f);
#else
    int l = f->lock;
    if (l < 0 || l && (l & ~MAYBE_WAITERS) == __pthread_self()->tid)
        return putc_unlocked(c, f);
    return locking_putc(c, f);
#endif /* OS_OPTION_NUTTX_VFS */
}
