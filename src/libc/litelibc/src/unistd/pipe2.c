#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#ifdef OS_OPTION_NUTTX_VFS
#include <nuttx/sys/sys_unistd.h>
#endif

int pipe2(int fd[2], int flag)
{
#ifdef OS_OPTION_NUTTX_VFS
    return sys_pipe2(fd, flag);
#else
    errno = ENOTSUP;
    return -1;
#endif
}
