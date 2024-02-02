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
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <stdint.h>
#include <sys/sysmacros.h>
#include "syscall.h"
#include "kstat.h"
#ifdef OS_OPTION_NUTTX_VFS
#include "nuttx/sys/sys_stat.h"
#endif

int fstatat(int fd, const char *restrict path, struct stat *restrict st, int flag)
{
#ifdef OS_OPTION_NUTTX_VFS
    return sys_fstatat(fd, path, st, flag);
#else
    errno = ENOTSUP;
    return -1;
#endif
}

#if !_REDIR_TIME64
weak_alias(fstatat, fstatat64);
#endif
