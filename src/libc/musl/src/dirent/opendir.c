#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <dirent.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include "__dirent.h"
#include "syscall.h"
#ifdef OS_OPTION_NUTTX_VFS
#include <nuttx/sys/sys_fcntl.h>
#include <nuttx/sys/sys_unistd.h>
#endif

DIR *opendir(const char *name)
{
#ifdef OS_OPTION_NUTTX_VFS
    int fd;
    DIR *dir;

    if ((fd = sys_open(name, O_RDONLY|O_DIRECTORY|O_CLOEXEC)) < 0)
        return 0;
#ifdef OS_OPTION_PROXY
    if (!(dir = __libc_calloc(1, sizeof *dir))) {
        sys_close(fd);
#else
    if (!(dir = calloc(1, sizeof *dir))) {
        sys_close(fd);
#endif
        return 0;
    }
    dir->fd = fd;
    return dir;
#else
    errno = ENOTSUP;
    return 0;
#endif /* OS_OPTION_NUTTX_VFS */
}
