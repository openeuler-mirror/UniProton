#include <sys/stat.h>
#include <errno.h>
#ifdef OS_OPTION_NUTTX_VFS
#include "nuttx/sys/sys_stat.h"
#endif

int futimens(int fd, const struct timespec times[2])
{
#ifdef OS_OPTION_NUTTX_VFS
    return sys_futimens(fd, times);
#else
    errno = ENOTSUP;
    return -1;
#endif
}
