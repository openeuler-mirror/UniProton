#include <unistd.h>
#include <errno.h>

int setreuid(uid_t ruid, uid_t euid)
{
    errno = ENOTSUP;
    return -1;
}
