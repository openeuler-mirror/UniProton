#include <unistd.h>
#include <errno.h>

int setuid(uid_t uid)
{
    errno = ENOTSUP;
    return -1;
}
