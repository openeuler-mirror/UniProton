/*
 * Copyright (c) 2022-2022 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Create: 2022-11-15
 * Description: 信号量头文件
 */
#ifndef _SEMAPHORE_H
#define _SEMAPHORE_H

#include "time.h"

#ifdef __cplusplus
extern "C" {
#endif

#define O_CREAT        0100
#define O_EXCL         0200
#define O_NOCTTY       0400
#define O_TRUNC       01000
#define O_APPEND      02000
#define O_NONBLOCK    04000
#define O_DSYNC      010000
#define O_SYNC     04010000
#define O_RSYNC    04010000
#define O_DIRECTORY  040000
#define O_NOFOLLOW  0100000
#define O_CLOEXEC  02000000

#define MAX_POSIX_SEMAPHORE_NAME_LEN    31
#define SEM_FAILED                      ((sem_t *)0)

typedef struct {
    unsigned short semHandle;
    unsigned short reserve;
    unsigned int refCount;
} sem_t;

int    sem_destroy(sem_t *);
int    sem_getvalue(sem_t *__restrict, int *__restrict);
int    sem_init(sem_t *, int, unsigned);
int    sem_post(sem_t *);
int    sem_timedwait(sem_t *__restrict, const struct timespec *__restrict);
int    sem_trywait(sem_t *);
int    sem_wait(sem_t *);
sem_t *sem_open(const char *, int, ...);
int    sem_close(sem_t *);
int    sem_unlink(const char *);

#ifdef __cplusplus
}
#endif

#endif /* _SEMAPHORE_H */
