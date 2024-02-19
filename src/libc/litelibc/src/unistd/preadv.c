#define _BSD_SOURCE
#ifdef _GNU_SOURCE
#define HAD_SOURCE
#undef _GNU_SOURCE
#endif
#include <sys/uio.h>
#ifdef HAD_SOURCE
#undef HAD_SOURCE
#define _GNU_SOURCE
#endif
#include <unistd.h>
#include <errno.h>
#ifdef OS_OPTION_NUTTX_VFS
#include <nuttx/sys/sys_uio.h>
#endif

ssize_t preadv(int fd, const struct iovec *iov, int count, off_t ofs)
{
#ifdef OS_OPTION_NUTTX_VFS
    return sys_preadv(fd, iov, count, ofs);
#else
    errno = ENOTSUP;
    return 0;
#endif
}

weak_alias(preadv, preadv64);
