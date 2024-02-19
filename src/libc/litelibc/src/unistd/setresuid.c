#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <unistd.h>
#include <errno.h>

int setresuid(uid_t ruid, uid_t euid, uid_t suid)
{
    errno = ENOTSUP;
    return -1;
}
