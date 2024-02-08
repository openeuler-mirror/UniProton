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
#include <errno.h>
#include "syscall.h"
#ifdef OS_OPTION_NUTTX_VFS
#include <nuttx/sys/sys_stat.h>
#include <nuttx/sys/sys_fcntl.h>
#endif

int posix_fallocate(int fd, off_t base, off_t len)
{
#ifdef OS_OPTION_NUTTX_VFS
    return sys_posix_fallocate(fd, base, len);
#else
    return ENOTSUP;
#endif
}

weak_alias(posix_fallocate, posix_fallocate64);
