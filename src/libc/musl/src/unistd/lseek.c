#define _BSD_SOURCE
#ifdef _GNU_SOURCE
#define HAD_SOURCE
#undef _GNU_SOURCE
#endif
#include <unistd.h>
#ifdef HAD_SOURCE
#undef HAD_SOURCE
#define _GNU_SOURCE
#endif
#include <errno.h>
#include "syscall.h"
#ifdef OS_OPTION_NUTTX_VFS
#include <nuttx/sys/sys_unistd.h>
#endif

off_t __lseek(int fd, off_t offset, int whence)
{
#ifdef OS_OPTION_NUTTX_VFS
    return sys_lseek(fd, offset, whence);
#else
    errno = ENOTSUP;
    return -1;
#endif /* OS_OPTION_NUTTX_VFS */
}

weak_alias(__lseek, lseek);
weak_alias(__lseek, lseek64);
