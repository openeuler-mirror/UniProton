#define _BSD_SOURCE
#ifdef _GNU_SOURCE
#define HAD_SOURCE
#undef _GNU_SOURCE
#endif
#include <unistd.h>
#ifdef HAD_SOURCE
#undef HAD_SOURCE
#define _GNU_SOURCE
#endif
#include <errno.h>

#ifdef OS_OPTION_NUTTX_VFS
#include "nuttx/sys/sys_unistd.h"
#endif

ssize_t pread(int fd, void *buf, size_t size, off_t ofs)
{
#ifdef OS_OPTION_NUTTX_VFS
    return sys_pread(fd, buf, size, ofs);
#else
    errno = ENOTSUP;
	return -1;
#endif
}

weak_alias(pread, pread64);