#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <unistd.h>
#include <errno.h>

int setresgid(gid_t rgid, gid_t egid, gid_t sgid)
{
    errno = ENOTSUP;
    return -1;
}
