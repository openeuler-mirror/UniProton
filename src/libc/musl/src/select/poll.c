#include <poll.h>
#include <time.h>
#include <signal.h>
#include <errno.h>
#include "syscall.h"

#ifdef OS_OPTION_NUTTX_VFS
#include "nuttx/fs/fs_poll.h"
#endif

int poll(struct pollfd *fds, nfds_t n, int timeout)
{
#ifdef OS_OPTION_NUTTX_VFS
    return fs_poll(fds, n, timeout);
#else
#ifdef OS_OPTION_PROXY
    return PRT_ProxyPoll(fds, n, timeout);
#else
    return -EAFNOSUPPORT;
#endif
#endif
}
