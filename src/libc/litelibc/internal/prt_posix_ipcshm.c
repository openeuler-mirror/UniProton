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
 * Create: 2023-10-10
 * Description: os内部ipc shm功能实现
 */

#include "securec.h"
#include "prt_mem_external.h"
#include "prt_ipc_internal.h"

static struct ShmSegCb g_ipcShmSeg[SHMSEG_MAX_SHM_LIMIT];
static int g_used_ids = 0;
static int g_used_shm = 0;

#define SHMALL SHMSEG_MAX_SHM_TOTAL_SIZE
#define SHMMAX SHMSEG_MAX_SHM_TOTAL_SIZE
#define SHMMIN 1
#define SHMMNI SHMSEG_MAX_SHM_LIMIT 
#define SHMSEG SHMSEG_MAX_SHM_LIMIT

static struct ShmSegCb *OsFindShmById(int shmid)
{
    int index = OS_INNER_ID(shmid);
    if (index >= SHMSEG_MAX_SHM_LIMIT || index < 0) {
        return NULL;
    }

    if (g_ipcShmSeg[index].shmid == shmid) {
        return g_ipcShmSeg + index;
    }
    return NULL;
}

static void OsShmFree(struct ShmSegCb *shmSeg, uintptr_t intSave)
{
    void *addr, *shminfo;
    addr = shmSeg->addr;
    shminfo = shmSeg->shminfo;
    shmSeg->key = 0;
    shmSeg->shmid = 0;
    g_used_ids--;
    g_used_shm -= shmSeg->shminfo->shm_segsz;

    OsIntRestore(intSave);
    (void)PRT_MemFree(OS_MID_APP, addr);
    (void)PRT_MemFree(OS_MID_APP, shminfo);
    return;
}

static int OsShmNew(int idx, key_t key, size_t size)
{
    struct shmid_ds *shminfo;
    g_ipcShmSeg[idx].addr = PRT_MemAlloc(OS_MID_APP, OS_MEM_DEFAULT_FSC_PT, size);
    if (!g_ipcShmSeg[idx].addr) {
        return ENOMEM;
    }
    shminfo = PRT_MemAlloc(OS_MID_APP, OS_MEM_DEFAULT_FSC_PT, sizeof(struct shmid_ds));
    g_ipcShmSeg[idx].shminfo = shminfo;
    if (!shminfo) {
        (void)PRT_MemFree(OS_MID_APP, g_ipcShmSeg[idx].addr);
        return ENOMEM;
    }
    g_ipcShmSeg[idx].key = key;
    g_ipcShmSeg[idx].shmid = OS_IPC_ID(idx);
    (void)memset_s(g_ipcShmSeg[idx].addr, size, 0, size);
    (void)memset_s(shminfo, sizeof(struct shmid_ds), 0, sizeof(struct shmid_ds));

    TskHandle pid = RUNNING_TASK->taskPid;
#if defined(_GNU_SOURCE) || defined(_BSD_SOURCE)
    shminfo->shm_perm.key = key;
#else
    shminfo->shm_perm.__key = key;
#endif
    shminfo->shm_perm.mode = 0666;
    shminfo->shm_segsz = size;
    shminfo->shm_lpid = 0;
    shminfo->shm_cpid = pid;
    shminfo->shm_ctime = time(NULL);
    g_used_ids++;
    g_used_shm += size;
    return 0;
}

int OsShmGet(key_t key, int flag, size_t size, int *shmid)
{
    uintptr_t intSave;
    int idx, find, unused, ret;

    if (shmid == NULL) {
        return EINVAL;
    }

    unused = find = -1;
    intSave = OsIntLock();

    for (idx = 0; idx < SHMSEG_MAX_SHM_LIMIT; ++idx) {
        if (g_ipcShmSeg[idx].shmid == 0) {
            // 第一个未使用的id
            unused = (unused == -1) ? idx : unused;
        } else if (g_ipcShmSeg[idx].key == key) { // 找到了key
            find = idx;
        }
    }

    if (key != IPC_PRIVATE) {
        if (find >= 0) { 
            if ((flag & IPC_CREAT) && (flag & IPC_EXCL)) {
                ret = EEXIST;
            } else if (g_ipcShmSeg[find].shminfo->shm_segsz < size) {
                ret = EINVAL;
            } else {
                *shmid = g_ipcShmSeg[find].shmid;
                ret = 0;
            }
            goto ERR_EXIT;
        } else if ((flag & IPC_CREAT) == 0) {
            ret = ENOENT;
            goto ERR_EXIT;
        }
    }

    if (unused == -1 || (size + g_used_shm) > SHMALL) { // 无法找到新的共享内存
        ret = ENOSPC;
        goto ERR_EXIT;
    }

    // 创建新共享内存
    if (size > SHMMAX || size < SHMMIN) {
        ret = EINVAL;
        goto ERR_EXIT;
    }

    if (g_used_ids + 1 > SHMSEG_MAX_SHM_LIMIT || g_used_shm + size > SHMALL) {
        ret = ENOSPC;
        goto ERR_EXIT;
    }

    ret = OsShmNew(unused, key, size);
    if (ret != 0) {
        goto ERR_EXIT;
    }
    *shmid = OS_IPC_ID(unused);
    OsIntRestore(intSave);
    return 0;

ERR_EXIT:
    OsIntRestore(intSave);
    return ret;
}

int OsShmAt(int shmid, const void *shmaddr, int shmflg, void **addr)
{
    (void)shmflg;
    (void)shmaddr;
    uintptr_t intSave;
    struct ShmSegCb *shmSeg;

    if (!addr) {
        return EINVAL;
    }

    *addr = NULL;
    intSave = OsIntLock();
    shmSeg = OsFindShmById(shmid);
    if (!shmSeg) {
        OsIntRestore(intSave);
        return EINVAL;
    }
    shmSeg->shminfo->shm_nattch += 1;
    shmSeg->shminfo->shm_atime = time(NULL);
    *addr = shmSeg->addr;
    OsIntRestore(intSave);
    return 0;
}

int OsShmDt(const void *shmaddr)
{
    int idx;
    uintptr_t intSave;
    struct ShmSegCb *shmSeg = NULL;
    struct shmid_ds *shminfo;
    if (!shmaddr) {
        return EINVAL;
    }

    intSave = OsIntLock();
    for (idx = 0; idx < SHMSEG_MAX_SHM_LIMIT; idx++) {
        if (g_ipcShmSeg[idx].addr == shmaddr && g_ipcShmSeg[idx].shmid != 0){
            shmSeg = g_ipcShmSeg + idx;
        }
    }
    if (!shmSeg) {
        OsIntRestore(intSave);
        return EINVAL;
    }

    shminfo = shmSeg->shminfo;
    shminfo->shm_nattch -= 1;
    shminfo->shm_dtime = time(NULL);
    if (shminfo->shm_nattch != 0 || (shminfo->shm_perm.mode & SHM_DEST) == 0) {
        OsIntRestore(intSave);
        return 0;
    }

    // destory shm
    OsShmFree(shmSeg, intSave);
    return 0;
}

int OsShmDelete(int shmid)
{
    uintptr_t intSave;
    struct ShmSegCb *shmSeg;

    intSave = OsIntLock();
    shmSeg = OsFindShmById(shmid);
    if (!shmSeg) {
        OsIntRestore(intSave);
        return EINVAL;
    }
    shmSeg->shminfo->shm_perm.mode |= SHM_DEST;
    if (shmSeg->shminfo->shm_nattch != 0) {
        OsIntRestore(intSave);
        return 0;
    }

    OsShmFree(shmSeg, intSave);
    return 0;
}

int OsShmGetStat(int shmid, struct shmid_ds *buf)
{
    uintptr_t intSave;
    struct ShmSegCb *shmSeg;

    if (buf == NULL) {
        return EFAULT;
    }

    intSave = OsIntLock();
    shmSeg = OsFindShmById(shmid);
    if (!shmSeg) {
        OsIntRestore(intSave);
        return EINVAL;
    }
    (void)memcpy_s(buf, sizeof(struct shmid_ds), shmSeg->shminfo, sizeof(struct shmid_ds));
    OsIntRestore(intSave);
    return 0;
}

int OsShmGetIpcInfo(struct shminfo *buf)
{
    if (buf == NULL) {
        return EFAULT;
    }

    buf->shmmax = SHMMAX;
    buf->shmmin = SHMMIN;
    buf->shmmni = SHMMNI;
    buf->shmseg = SHMSEG;
    buf->shmall = SHMALL;
    return 0;
}

int OsShmGetShmInfo(struct shm_info *buf)
{
    if (buf == NULL) {
        return EFAULT;
    }

    (void)memset_s(buf, sizeof(struct shm_info), 0, sizeof(struct shm_info));
#ifdef _GNU_SOURCE
    buf->used_ids = g_used_ids;
#else
    buf->__used_ids = g_used_ids;
#endif
    return 0;
}