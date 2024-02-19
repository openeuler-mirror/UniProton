#include <unistd.h>
#include <errno.h>
#ifdef OS_OPTION_NUTTX_VFS
#include <nuttx/sys/sys_unistd.h>
#endif

int unlinkat(int fd, const char *path, int flag)
{
#ifdef OS_OPTION_NUTTX_VFS
    return sys_unlinkat(fd, path, flag);
#else
    errno = ENOTSUP;
    return -1;
#endif
}
