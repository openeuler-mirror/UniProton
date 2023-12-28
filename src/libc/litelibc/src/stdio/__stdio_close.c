#include "stdio_impl.h"
#include "aio_impl.h"
#ifdef OS_OPTION_NUTTX_VFS
#include "nuttx/sys/sys_unistd.h"
#endif

static int dummy(int fd)
{
    return fd;
}

weak_alias(dummy, __aio_close);

int __stdio_close(FILE *f)
{
#ifdef OS_OPTION_NUTTX_VFS
    return sys_close(f->fd);
#else
    return -1;
#endif
}
