#include <sys/uio.h>
#include <errno.h>
#ifdef OS_OPTION_NUTTX_VFS
#include <nuttx/sys/sys_uio.h>
#endif

ssize_t writev(int fd, const struct iovec *iov, int count)
{
#ifdef OS_OPTION_NUTTX_VFS
    return sys_writev(fd, iov, count);
#else
    errno = ENOTSUP;
    return -1;
#endif
}
