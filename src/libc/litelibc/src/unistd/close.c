#include <unistd.h>
#include <errno.h>
#include "aio_impl.h"

#ifdef OS_OPTION_NUTTX_VFS
#include "nuttx/sys/sys_unistd.h"
#endif

static int dummy(int fd)
{
    return fd;
}

weak_alias(dummy, __aio_close);

int close(int fd)
{
#ifdef OS_OPTION_NUTTX_VFS
    return sys_close(fd);
#else
    errno = ENOTSUP;
	return -1;
#endif
}
