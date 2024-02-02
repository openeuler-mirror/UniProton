#define _BSD_SOURCE
#ifdef _GNU_SOURCE
#define HAD_SOURCE
#undef _GNU_SOURCE
#endif
#include <sys/stat.h>
#ifdef HAD_SOURCE
#undef HAD_SOURCE
#define _GNU_SOURCE
#endif
#include <errno.h>
#include <fcntl.h>
#include "syscall.h"
#ifdef OS_OPTION_NUTTX_VFS
#include "nuttx/sys/sys_stat.h"
#endif

int fstat(int fd, struct stat *st)
{
    if (fd<0) return __syscall_ret(-EBADF);
#ifdef OS_OPTION_NUTTX_VFS
    return sys_fstat(fd, st);
#else
    errno = ENOTSUP;
    return -1;
#endif
}

#if !_REDIR_TIME64
weak_alias(fstat, fstat64);
#endif
