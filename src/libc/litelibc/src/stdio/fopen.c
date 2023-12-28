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
#include <string.h>
#include <errno.h>

#ifdef OS_OPTION_NUTTX_VFS
#include <nuttx/sys/sys_fcntl.h>
#include <nuttx/sys/sys_unistd.h>
#endif

FILE *fopen(const char *restrict filename, const char *restrict mode)
{
#ifdef OS_OPTION_NUTTX_VFS
    FILE *f;
    int fd;
    int flags;

    /* Check for valid initial mode character */
    if (!strchr("rwa", *mode)) {
        errno = EINVAL;
        return 0;
    }

    /* Compute the flags to pass to open() */
    flags = __fmodeflags(mode);

    fd = sys_open(filename, flags, 0666);
    if (fd < 0) return 0;
    if (flags & O_CLOEXEC)
        sys_fcntl(fd, F_SETFD, FD_CLOEXEC);

    f = __fdopen(fd, mode);
    if (f) return f;

    sys_close(fd);
    return 0;
#else
    errno = ENOTSUP;
    return 0;
#endif /* OS_OPTION_NUTTX_VFS */
}

weak_alias(fopen, fopen64);
