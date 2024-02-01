#include <sys/stat.h>
#include <errno.h>
#include "syscall.h"
#ifdef OS_OPTION_NUTTX_VFS
#include "nuttx/sys/sys_stat.h"
#endif

int mkdirat(int fd, const char *path, mode_t mode)
{
#ifdef OS_OPTION_NUTTX_VFS
    return sys_mkdirat(fd, path, mode);
#else
    errno = ENOTSUP;
    return -1;
#endif
}
