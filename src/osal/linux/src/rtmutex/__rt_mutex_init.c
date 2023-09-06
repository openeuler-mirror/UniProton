#include <stdio.h>
#include <linux/rtmutex.h>

void __rt_mutex_init(struct rt_mutex *lock, const char *name, struct lock_class_key *key)
{
    (void)name;
    (void)key;
    int ret = pthread_mutex_init((pthread_mutex_t *)lock, NULL);
    if (ret != 0) {
        printf("[Error] mutex init fail\n");
    }
}