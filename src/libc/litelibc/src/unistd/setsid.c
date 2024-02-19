#include <unistd.h>
#include <errno.h>

pid_t setsid(void)
{
    errno = ENOTSUP;
    return -1;
}
