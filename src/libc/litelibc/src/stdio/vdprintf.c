#include "stdio_impl.h"

int vdprintf(int fd, const char *restrict fmt, va_list ap)
{
    FILE f = {
        .fd = fd, .lbf = EOF, .write = __stdio_write,
        .buf = (void *)fmt, .buf_size = 0,
#ifdef OS_OPTION_NUTTX_VFS
        .owner = -1,
        .mutex = {PTHREAD_MUTEX_RECURSIVE, 0, 0},
#else
        .lock = -1
#endif /* OS_OPTION_NUTTX_VFS */
    };
    return vfprintf(&f, fmt, ap);
}
