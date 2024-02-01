#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#ifdef OS_OPTION_NUTTX_VFS
#include "nuttx/sys/sys_stat.h"
#endif

int lchmod(const char *path, mode_t mode)
{
#ifdef OS_OPTION_NUTTX_VFS
    return sys_lchmod(path, mode);
#else
    errno = ENOTSUP;
    return -1;
#endif
}
