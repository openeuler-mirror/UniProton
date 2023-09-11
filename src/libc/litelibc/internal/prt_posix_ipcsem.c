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
 * Create: 2023-08-11
 * Description: os内部ipc sem功能实现
 */
#include "securec.h"
#include "prt_mem_external.h"
#include "prt_ipc_internal.h"
#include "kal_ipc.h"

static struct SemSetCb g_ipcSemSet[SEMSET_MAX_SYS_LIMIT];

int OsSemSetGet(key_t key, int nsems, int flag, int mode, int *semid)
{
    (void)mode;
    uintptr_t intSave;
    int idx, find, unused, ret, i;
    
    if (nsems > SEMSET_MAX_SEM_NUM || nsems < 0 || semid == NULL) {
        return EINVAL;
    }
    unused = find = -1;
    intSave = OsIntLock();
    for (idx = 0; idx < SEMSET_MAX_SYS_LIMIT; ++idx) {
        if (g_ipcSemSet[idx].semid == 0) {
            // 第一个未使用的id
            unused = (unused == -1) ? idx : unused;
        } else if (g_ipcSemSet[idx].key == key) { // 找到了key
            find = idx;
        }
    }
    if (key != IPC_PRIVATE) {
        if (find >= 0) { 
            *semid = g_ipcSemSet[find].semid;
            OsIntRestore(intSave);
            return (flag & (IPC_CREAT | IPC_EXCL)) ? EEXIST : 0;
        } else if ((flag & IPC_CREAT) == 0) {
            OsIntRestore(intSave);
            return ENOENT;
        }
    }
    if (unused == -1) { // 无法找到新的信号量集合
        OsIntRestore(intSave);
        return ENOSPC;
    }
    if (nsems == 0) {
        OsIntRestore(intSave);
        return EINVAL;
    }
    g_ipcSemSet[unused].key = key;
    g_ipcSemSet[unused].semid = *semid = OS_IPC_ID(unused);
    g_ipcSemSet[unused].num = (U32)nsems;
    for (idx = 0; idx < nsems; ++idx) {
        ret = KAL_SemCreate(flag, mode, 0, (uintptr_t *)&g_ipcSemSet[unused].handle[idx]);
        if (ret != 0) {
            // 之前创建成功的都删除
            g_ipcSemSet[unused].semid = g_ipcSemSet[unused].key = g_ipcSemSet[unused].num = 0;
            for (i = 0; i < idx; ++i) {
                (void)KAL_SemDelete((uintptr_t)g_ipcSemSet[unused].handle[i]);
            }
            OsIntRestore(intSave);
            return ret;
        }
    }
    OsIntRestore(intSave);
    return 0;
}

int OsSemSetStat(int semid, struct semid_ds *buf)
{
    int idx;
    uintptr_t intSave;
    if (buf == NULL) {
        return EFAULT;
    }
    idx = OS_IPC_INNER_ID(semid);
    if (idx < 0 || idx >= SEMSET_MAX_SYS_LIMIT) {
        return EINVAL;
    }
    intSave = OsIntLock();
    if (g_ipcSemSet[idx].semid == 0) {
        OsIntRestore(intSave);
        return EIDRM;
    }
    buf->sem_nsems = g_ipcSemSet[idx].num;
    OsIntRestore(intSave);
    return 0;
}

int OsSemSetGetVal(int semid, int nth, int *value)
{
    int idx, ret;
    uintptr_t intSave;
    if (value == NULL) {
        return EFAULT;
    }
    idx = OS_IPC_INNER_ID(semid);
    if (idx < 0 || idx >= SEMSET_MAX_SYS_LIMIT || nth < 0 || nth >= SEMSET_MAX_SEM_NUM) {
        return EINVAL;
    }
    intSave = OsIntLock();
    if (g_ipcSemSet[idx].semid == 0) {
        OsIntRestore(intSave);
        return EIDRM;
    }
    if (nth >= g_ipcSemSet[idx].num) {
        OsIntRestore(intSave);
        return EINVAL;
    }
    ret = KAL_SemGetValue((uintptr_t)g_ipcSemSet[idx].handle[nth], value);
    OsIntRestore(intSave);
    return ret;
}

int OsSemSetGetAll(int semid, unsigned short *array)
{
    int idx, ret, i;
    uintptr_t intSave;
    if (array == NULL) {
        return EFAULT;
    }
    idx = OS_IPC_INNER_ID(semid);
    if (idx < 0 || idx >= SEMSET_MAX_SYS_LIMIT) {
        return EINVAL;
    }
    intSave = OsIntLock();
    if (g_ipcSemSet[idx].semid == 0) {
        OsIntRestore(intSave);
        return EIDRM;
    }
    for (i = 0; i < g_ipcSemSet[idx].num; ++i) {
        ret = KAL_SemGetValue((uintptr_t)g_ipcSemSet[idx].handle[i], (int *)&array[i]);
        if (ret != 0) {
            OsIntRestore(intSave);
            return ret;
        }
    }
    OsIntRestore(intSave);
    return 0;
}

int OsSemSetDelete(int semid)
{
    int idx, ret, i;
    uintptr_t intSave;

    idx = OS_IPC_INNER_ID(semid);
    if (idx < 0 || idx >= SEMSET_MAX_SYS_LIMIT) {
        return EINVAL;
    }
    intSave = OsIntLock();
    if (g_ipcSemSet[idx].semid == 0) {
        OsIntRestore(intSave);
        return EIDRM;
    }
    for (i = 0; i < g_ipcSemSet[idx].num; ++i) {
        ret = KAL_SemDelete((uintptr_t)g_ipcSemSet[idx].handle[i]);
        if (ret != 0) {
            OsIntRestore(intSave);
            return ret;
        }
    }
    g_ipcSemSet[idx].key = 0;
    g_ipcSemSet[idx].semid = 0;
    g_ipcSemSet[idx].num = 0;
    OsIntRestore(intSave);
    return 0;
}

static int OsSemSetTimeOpOne(int idx, unsigned short num, short op, short flg, const struct timespec *timeout)
{
    SemHandle sem;
    if (num >= g_ipcSemSet[idx].num) {
        return EFBIG;
    }
    sem = g_ipcSemSet[idx].handle[num];
    if (op > 0) {
        return KAL_SemPost(sem, op, flg);
    } else if (op < 0) {
        return KAL_SemWait(sem, -op, flg, timeout);
    }
    return ENOTSUP;
}

int OsSemSetTimeOp(int semid, struct sembuf *sops, size_t nsops, const struct timespec *timeout)
{
    int idx, ret, i;
    uintptr_t intSave;
    if (sops == NULL) {
        return EFAULT;
    }
    if (nsops != 1) {
        return ENOTSUP;
    }
    if (nsops > SEMSET_MAX_SEM_NUM) {
        return E2BIG;
    }
    idx = OS_IPC_INNER_ID(semid);
    if (idx < 0 || idx >= SEMSET_MAX_SYS_LIMIT) {
        return EINVAL;
    }
    intSave = OsIntLock();
    if (g_ipcSemSet[idx].semid == 0) {
        OsIntRestore(intSave);
        return EIDRM;
    }
    for (i = 0; i < nsops; ++i) {
        ret = OsSemSetTimeOpOne(idx, sops[i].sem_num, sops[i].sem_op, sops[i].sem_flg, timeout);
        if (ret != 0) {
            OsIntRestore(intSave);
            return ret;
        }
    }
    OsIntRestore(intSave);
    return 0;
}