#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include "syscall.h"

#ifdef OS_OPTION_NUTTX_VFS
#include <nuttx/sys/sys_fcntl.h>
#include <nuttx/sys/sys_unistd.h>
#endif

int remove(const char *path)
{
#ifdef OS_OPTION_NUTTX_VFS
    int r = sys_unlink(path);
    if (r == -1 && errno == EISDIR) sys_rmdir(path);
    return r;
#endif /* OS_OPTION_NUTTX_VFS */
    errno = ENOTSUP;
    return -1;
}
