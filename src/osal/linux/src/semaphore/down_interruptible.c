#include <linux/semaphore.h>
#include <stdio.h>

int __must_check down_interruptible(struct semaphore *sem)
{
    int ret = sem_wait((sem_t *)sem);
    if (ret != 0) {
        printf("[Error] down_interruptible fail\n");
        return ret;
    }
    return 0;
}