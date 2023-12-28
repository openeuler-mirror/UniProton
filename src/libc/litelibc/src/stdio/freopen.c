#define _BSD_SOURCE
#ifdef _GNU_SOURCE
#define HAD_SOURCE
#undef _GNU_SOURCE
#endif
#include <stdio.h>
#ifdef HAD_SOURCE
#undef HAD_SOURCE
#define _GNU_SOURCE
#endif
#include "stdio_impl.h"
#include <fcntl.h>
#include <unistd.h>
#ifdef OS_OPTION_NUTTX_VFS
#include <nuttx/sys/sys_fcntl.h>
#include <nuttx/sys/sys_unistd.h>
#endif

/* The basic idea of this implementation is to open a new FILE,
 * hack the necessary parts of the new FILE into the old one, then
 * close the new FILE. */

/* Locking IS necessary because another thread may provably hold the
 * lock, via flockfile or otherwise, when freopen is called, and in that
 * case, freopen cannot act until the lock is released. */

FILE *freopen(const char *restrict filename, const char *restrict mode, FILE *restrict f)
{
#ifdef OS_OPTION_NUTTX_VFS
    int fl = __fmodeflags(mode);
    FILE *f2;

    FLOCK(f);

    fflush(f);

    if (!filename) {
        if (fl&O_CLOEXEC)
            sys_fcntl(f->fd, F_SETFD, FD_CLOEXEC);
        fl &= ~(O_CREAT|O_EXCL|O_CLOEXEC);
        if (sys_fcntl(f->fd, F_SETFL, fl) < 0)
            goto fail;
    } else {
        f2 = fopen(filename, mode);
        if (!f2) goto fail;
        if (f2->fd == f->fd) f2->fd = -1; /* avoid closing in fclose */
        else if (sys_dup2(f2->fd, f->fd)<0) goto fail2;

        f->flags = (f->flags & F_PERM) | f2->flags;
        f->read = f2->read;
        f->write = f2->write;
        f->seek = f2->seek;
        f->close = f2->close;

        fclose(f2);
    }

    FUNLOCK(f);
    return f;

fail2:
    fclose(f2);
fail:
    fclose(f);
#endif /* OS_OPTION_NUTTX_VFS */
    return NULL;
}

weak_alias(freopen, freopen64);
