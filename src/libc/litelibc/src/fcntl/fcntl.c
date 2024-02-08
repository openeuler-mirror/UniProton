#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <fcntl.h>
#include <stdarg.h>
#include <errno.h>
#include "syscall.h"
#ifdef OS_OPTION_NUTTX_VFS
#include <nuttx/sys/sys_fcntl.h>
#include <nuttx/sys/sys_unistd.h>
#endif

int fcntl(int fd, int cmd, ...)
{
#ifdef OS_OPTION_NUTTX_VFS
    unsigned long arg;
    va_list ap;
    va_start(ap, cmd);
    arg = va_arg(ap, unsigned long);
    va_end(ap);
    if (cmd == F_SETFL) arg |= O_LARGEFILE;
    if (cmd == F_SETLKW) return sys_fcntl(fd, cmd, (void *)arg);
    if (cmd == F_GETOWN) {
        struct f_owner_ex ex;
        int ret = sys_fcntl(fd, F_GETOWN_EX, &ex);
        if (ret == -EINVAL) return sys_fcntl(fd, cmd, (void *)arg);
        if (ret) return __syscall_ret(ret);
        return ex.type == F_OWNER_PGRP ? -ex.pid : ex.pid;
    }
    if (cmd == F_DUPFD_CLOEXEC) {
        int ret = sys_fcntl(fd, F_DUPFD_CLOEXEC, arg);
        if (ret != -EINVAL) {
            if (ret >= 0)
                sys_fcntl(ret, F_SETFD, FD_CLOEXEC);
            return __syscall_ret(ret);
        }
        ret = sys_fcntl(fd, F_DUPFD_CLOEXEC, 0);
        if (ret != -EINVAL) {
            if (ret >= 0) sys_close(ret);
            return __syscall_ret(-EINVAL);
        }
        ret = sys_fcntl(fd, F_DUPFD, arg);
        if (ret >= 0) sys_fcntl(ret, F_SETFD, FD_CLOEXEC);
        return __syscall_ret(ret);
    }
    switch (cmd) {
    case F_SETLK:
    case F_GETLK:
    case F_GETOWN_EX:
    case F_SETOWN_EX:
        return sys_fcntl(fd, cmd, (void *)arg);
    default:
        return sys_fcntl(fd, cmd, arg);
    }
#else
    errno = ENOTSUP;
    return -1;
#endif
}
