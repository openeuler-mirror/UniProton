#include <unistd.h>
#include <errno.h>

int setregid(gid_t rgid, gid_t egid)
{
    errno = ENOTSUP;
    return -1;
}
