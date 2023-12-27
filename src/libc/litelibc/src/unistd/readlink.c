#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#ifdef OS_OPTION_NUTTX_VFS
#include "nuttx/sys/sys_unistd.h"
#endif

ssize_t readlink(const char *restrict path, char *restrict buf, size_t bufsize)
{
#ifdef OS_OPTION_NUTTX_VFS
    return sys_readlink(path, buf, bufsize);
#else
    errno = ENOTSUP;
    return -1;
#endif
}
