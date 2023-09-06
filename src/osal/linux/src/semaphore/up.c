#include <linux/semaphore.h>
#include <stdio.h>

void up(struct semaphore *sem)
{
    int ret = sem_post((sem_t *)sem);
    if (ret != 0) {
        printf("[Error] up sema fail\n");
    }
}