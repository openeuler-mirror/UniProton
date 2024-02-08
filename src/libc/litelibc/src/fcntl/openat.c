#define _BSD_SOURCE
#ifdef _GNU_SOURCE
#define HAD_SOURCE
#undef _GNU_SOURCE
#endif
#include <fcntl.h>
#ifdef HAD_SOURCE
#undef HAD_SOURCE
#define _GNU_SOURCE
#endif
#include <stdarg.h>
#include <errno.h>
#include "syscall.h"
#ifdef OS_OPTION_NUTTX_VFS
#include <nuttx/sys/sys_fcntl.h>
#endif

int openat(int fd, const char *filename, int flags, ...)
{
#ifdef OS_OPTION_NUTTX_VFS
    mode_t mode = 0;

    if ((flags & O_CREAT) || (flags & O_TMPFILE) == O_TMPFILE) {
        va_list ap;
        va_start(ap, flags);
        mode = va_arg(ap, mode_t);
        va_end(ap);
    }

    return sys_openat(fd, filename, flags|O_LARGEFILE, mode);
#else
    errno = ENOTSUP;
    return -1;
#endif
}

weak_alias(openat, openat64);
