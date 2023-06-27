#include "pthread.h"
#include "prt_posix_internal.h"

int OsCondParamCheck(pthread_cond_t *cond)
{
    int ret;
    pthread_cond_t tmp = PTHREAD_COND_INITIALIZER;

    if (cond == NULL) {
        return EINVAL;
    }

    if (memcmp(cond, &tmp, sizeof(pthread_cond_t)) == 0) {
        ret = pthread_cond_init(cond, NULL);
        if (ret != OS_OK) {
            return EINVAL;
        }
    }

    return OS_OK;
}
