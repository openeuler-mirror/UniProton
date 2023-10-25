/*
 * Copyright (c) 2023-2023 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Create: 2023-08-10
 * Description: ipc内部用函数声明
 */
#ifndef PRT_IPC_INTERNAL_H
#define PRT_IPC_INTERNAL_H

#include <errno.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <time.h>
#include "prt_typedef.h"
#include "prt_buildef.h"
#include "prt_sem_external.h"

/**
 * @ingroup ipc
 * Default mode mask for a ipc
 */
#define IPC_DEFAULT_MODE_MASK   0x1ff

/**
 * @ingroup msg queue
 * Maximum number of message queues
 * @see OS_QUEUE_MAX_SUPPORT_NUM
 */
#define MSGQUE_MAX_SYS_LIMIT    8

/**
 * @ingroup msg queue
 * Maximum number of messages in a message queue
 */
#define MSGQUE_MAX_MSG_NUM      16

/**
 * @ingroup msg queue
 * Maximum size of a single message in a message queue
 */
#define MSGQUE_MAX_MSG_SIZE     64

/**
 * @ingroup semset
 * Maximum number of semaphores set 
 * @see OS_SEM_MAX_SUPPORT_NUM
 */
#define SEMSET_MAX_SYS_LIMIT    16

/**
 * @ingroup semset
 * Maximum number semaphores per semaphore set
 */
#define SEMSET_MAX_SEM_NUM 8

/**
 * @ingroup shm segment
 * Maximum number of shared memory segment 
 */
#define SHMSEG_MAX_SHM_LIMIT    128

/**
 * @ingroup shm segment
 * Maximum total size of all shared memory segment 
 */
#define SHMSEG_MAX_SHM_TOTAL_SIZE   0x5000

#define OS_IPC_INNER_ID(id)  ((id) - 1)
#define OS_IPC_ID(innerId)  ((innerId) + 1)
#define OS_INNER_ID(ipcId)  ((ipcId) - 1)

struct MsgQueCb {
    key_t key;
    int msgid;
};

struct SemSetCb {
    key_t key;
    int semid;
    SemHandle handle[SEMSET_MAX_SEM_NUM];
    U32 num;
};

struct ShmSegCb {
    key_t key;
    int shmid;
    void *addr;
    struct shmid_ds *shminfo;
};

union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

// Records the mapping between keys and msgids;
void OsMsgQueAddKeyId(key_t key, int msgid);
void OsMsgQueDelById(int msgid);
int OsMsgQueKey2Id(key_t key);

int OsSemSetGet(key_t key, int nsems, int flag, int mode, int *semid);
int OsSemSetStat(int semid, struct semid_ds *buf);
int OsSemSetGetVal(int semid, int nth, int *value);
int OsSemSetDelete(int semid);
int OsSemSetGetAll(int semid, unsigned short *array);
int OsSemSetTimeOp(int semid, struct sembuf *sops, size_t nops, const struct timespec *timeout);

int OsShmGet(key_t key, int flag, size_t size, int *shmid);
int OsShmAt(int shmid, const void *shmaddr, int shmflg, void **addr);
int OsShmDt(const void *shmaddr);
int OsShmDelete(int shmid);
int OsShmGetStat(int shmid, struct shmid_ds *buf);
int OsShmGetIpcInfo(struct shminfo *buf);
int OsShmGetShmInfo(struct shm_info *buf);

#endif

