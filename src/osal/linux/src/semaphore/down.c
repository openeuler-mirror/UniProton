#include <linux/semaphore.h>
#include <stdio.h>

void down(struct semaphore *sem)
{
    int ret = sem_wait((sem_t *)sem);
    if (ret != 0) {
        printf("[Error] down fail\n");
    }
}