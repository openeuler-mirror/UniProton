#include <stdio.h>
#include <linux/rtmutex.h>

void rt_mutex_lock(struct rt_mutex *lock)
{
    int ret = pthread_mutex_lock((pthread_mutex_t *)lock);
    if (ret != 0) {
        printf("[Error] mutex lock fail\n");
    }
}