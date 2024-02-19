#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#ifdef OS_OPTION_NUTTX_VFS
#include <nuttx/sys/sys_unistd.h>
#endif

int faccessat(int fd, const char *filename, int amode, int flag)
{
#ifdef OS_OPTION_NUTTX_VFS
    return sys_faccessat(fd, filename, amode, flag);
#else
    errno = ENOTSUP;
    return -1;
#endif
}
