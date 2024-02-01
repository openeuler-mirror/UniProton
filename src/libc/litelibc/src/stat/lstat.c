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
#include <fcntl.h>
#include <errno.h>
#ifdef OS_OPTION_NUTTX_VFS
#include "nuttx/sys/sys_stat.h"
#endif

int lstat(const char *restrict path, struct stat *restrict buf)
{
#ifdef OS_OPTION_NUTTX_VFS
    return sys_lstat(path, buf);
#else
    errno = ENOTSUP;
    return -1;
#endif
}

#if !_REDIR_TIME64
weak_alias(lstat, lstat64);
#endif
