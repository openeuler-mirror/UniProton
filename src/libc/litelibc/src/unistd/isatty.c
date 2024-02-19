#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#ifdef OS_OPTION_NUTTX_VFS
#include "nuttx/fs/ioctl.h"
#endif

int isatty(int fd)
{
#ifdef OS_OPTION_NUTTX_VFS
    struct winsize wsz;
    unsigned long r = sys_ioctl(fd, TIOCGWINSZ, &wsz);
    if (r == 0) return 1;
    if (errno != EBADF) errno = ENOTTY;
    return 0;
#else
    errno = ENOTSUP;
    return 0;
#endif
}
