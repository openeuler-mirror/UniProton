#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <errno.h>
#ifdef OS_OPTION_NUTTX_VFS
#include "nuttx/fs/ioctl.h"
#endif

int tcsetpgrp(int fd, pid_t pgrp)
{
#ifdef OS_OPTION_NUTTX_VFS
    int pgrp_int = pgrp;
    return sys_ioctl(fd, TIOCSPGRP, &pgrp_int);
#else
    errno = ENOTSUP;
    return -1;
#endif
}
