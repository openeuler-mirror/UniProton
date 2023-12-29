#include "stdio_impl.h"
#include <stdlib.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include "libc.h"
#ifdef OS_OPTION_NUTTX_VFS
#include <nuttx/sys/sys_fcntl.h>
#include <nuttx/fs/ioctl.h>
#endif

FILE *__fdopen(int fd, const char *mode)
{
#ifdef OS_OPTION_NUTTX_VFS
    FILE *f;
    struct winsize wsz;
    pthread_mutexattr_t attr;

    /* Check for valid initial mode character */
    if (!strchr("rwa", *mode)) {
        errno = EINVAL;
        return 0;
    }

    /* Allocate FILE+buffer or fail */
    if (!(f=malloc(sizeof *f + UNGET + BUFSIZ))) return 0;

    /* Zero-fill only the struct, not the buffer */
    memset(f, 0, sizeof *f);

    /* Impose mode restrictions */
    if (!strchr(mode, '+')) f->flags = (*mode == 'r') ? F_NOWR : F_NORD;

    /* Apply close-on-exec flag */
    if (strchr(mode, 'e')) sys_fcntl(fd, F_SETFD, FD_CLOEXEC);

    /* Set append mode on fd if opened for append */
    if (*mode == 'a') {
        int flags = sys_fcntl(fd, F_GETFL);
        if (!(flags & O_APPEND))
            sys_fcntl(fd, F_SETFL, flags | O_APPEND);
        f->flags |= F_APP;
    }

    f->fd = fd;
    f->buf = (unsigned char *)f + sizeof *f + UNGET;
    f->buf_size = BUFSIZ;

    /* Activate line buffered mode for terminals */
    f->lbf = EOF;
    if (!(f->flags & F_NOWR) && !sys_ioctl(fd, TIOCGWINSZ, &wsz))
        f->lbf = '\n';

    /* Initialize op ptrs. No problem if some are unneeded. */
    f->read = __stdio_read;
    f->write = __stdio_write;
    f->seek = __stdio_seek;
    f->close = __stdio_close;

    f->owner = -1;

    if (pthread_mutexattr_init(&attr) != 0 
            || pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE) != 0
            || pthread_mutex_init(&f->mutex, &attr) != 0) {
        free(f);
        return 0;
    }

    /* Add new FILE to open file list */
    return __ofl_add(f);
#else
    errno = ENOTSUP;
    return 0;
#endif /* OS_OPTION_NUTTX_VFS */
}

weak_alias(__fdopen, fdopen);
