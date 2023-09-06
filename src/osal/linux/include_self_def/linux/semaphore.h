/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __LINUX_SEMAPHORE_H
#define __LINUX_SEMAPHORE_H

#include <semaphore.h>
#include <linux/list.h>
#include <linux/compiler_types.h>

/* Please don't access any members of this structure directly */
struct semaphore {
    unsigned short semHandle;
    unsigned short reserve;
    unsigned int refCount;
};

static inline void sema_init(struct semaphore *sem, int val)
{
    int ret = sem_init((sem_t *)sem, 0, val);
    if (ret != 0) {
        printk("[Error] init sema fail\n");
    }
}

int __must_check down_interruptible(struct semaphore *sem);
void down(struct semaphore *sem);
void up(struct semaphore *sem);

#endif /* __LINUX_SEMAPHORE_H */