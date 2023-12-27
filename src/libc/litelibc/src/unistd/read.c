#include <unistd.h>
#include <errno.h>

#ifdef OS_OPTION_NUTTX_VFS
#include "nuttx/sys/sys_unistd.h"
#endif

ssize_t read(int fd, void *buf, size_t count)
{
#ifdef OS_OPTION_NUTTX_VFS
    return sys_read(fd, buf, count);
#else
    errno = ENOTSUP;
    return -1;
#endif
}
