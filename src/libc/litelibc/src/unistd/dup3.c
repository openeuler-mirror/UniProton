#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include "syscall.h"

int __dup3(int old, int new, int flags)
{
    (void)flags; //不支持O_CLOEXEC
    if (old == new) return __syscall_ret(-EINVAL);
    return dup2(old, new);
}

weak_alias(__dup3, dup3);
