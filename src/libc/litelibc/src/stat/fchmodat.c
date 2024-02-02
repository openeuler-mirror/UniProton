#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include "syscall.h"
#include "kstat.h"
#ifdef OS_OPTION_NUTTX_VFS
#include "nuttx/sys/sys_stat.h"
#endif

int fchmodat(int fd, const char *path, mode_t mode, int flag)
{
#ifdef OS_OPTION_NUTTX_VFS
    return sys_fchmodat(fd, path, mode, flag);
#else
    errno = ENOTSUP;
    return -1;
#endif /* OS_OPTION_NUTTX_VFS */
}
