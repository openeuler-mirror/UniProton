#include <unistd.h>
#include <errno.h>
#ifdef OS_OPTION_NUTTX_VFS
#include <nuttx/sys/sys_unistd.h>
#endif

int pipe(int fd[2])
{
#ifdef OS_OPTION_NUTTX_VFS
    return sys_pipe2(fd, 0);
#else
    errno = ENOTSUP;
    return -1;
#endif
}
