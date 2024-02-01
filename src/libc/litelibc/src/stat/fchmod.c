#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include "syscall.h"
#ifdef OS_OPTION_NUTTX_VFS
#include "nuttx/sys/sys_stat.h"
#endif

int fchmod(int fd, mode_t mode)
{
#ifdef OS_OPTION_NUTTX_VFS
    return sys_fchmod(fd, mode);
#else
    errno = ENOTSUP;
    return -1;
#endif /* OS_OPTION_NUTTX_VFS */
}
