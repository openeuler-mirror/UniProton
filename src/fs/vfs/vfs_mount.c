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
 * Create: 2022-09-21
 * Description: 文件系统vfs层
 */

#include "stdlib.h"
#include "string.h"
#include "securec.h"
#include "vfs_mount.h"
#include "vfs_files.h"
#include "vfs_maps.h"
#include "vfs_config.h"
#include "vfs_operations.h"
#include "prt_fs.h"

static struct TagMountPoint *g_mountPoints;

static void OsMpDeleteFromList(struct TagMountPoint *mp)
{
    struct TagMountPoint *prev = NULL;

    if (g_mountPoints == mp) {
        g_mountPoints = mp->mNext;
        return;
    }

    for (prev = g_mountPoints; prev != NULL; prev = prev->mNext) {
        if (prev->mNext != mp) {
            continue;
        }

        prev->mNext = mp->mNext;
        break;
    }
}

static S32 OsVfsFindMpByPath(const char *mPath, const char *iPath)
{
    const char *t = NULL;
    S32 matches = 0;
    do {
        while ((*mPath == '/') && (*(mPath + 1) != '/')) {
            mPath++;
        }
        while ((*iPath == '/') && (*(iPath + 1) != '/')) {
            iPath++;
        }

        t = strchr(mPath, '/');
        if (t == NULL) {
            t = strchr(mPath, '\0');
        }
        if ((t == mPath) || (t == NULL)) {
            break;
        }
        if (strncmp(mPath, iPath, (size_t)(t - mPath)) != 0) {
            return 0;
        }

        iPath += (t - mPath);
        if ((*iPath != '\0') && (*iPath != '/')) {
            return 0;
        }

        matches += (t - mPath);
        mPath += (t - mPath);
    } while (*mPath != '\0');

    return matches;
}

struct TagMountPoint *OsVfsFindMp(const char *path, const char **pathInMp)
{
    struct TagMountPoint *mp = g_mountPoints;
    struct TagMountPoint *bestMp = NULL;
    const char *iPath = NULL;
    S32 bestMatches = 0;
    S32 matches;

    if (path == NULL) {
        return NULL;
    }

    if (pathInMp != NULL) {
        *pathInMp = NULL;
    }

    while ((mp != NULL) && (mp->mPath != NULL)) {
        matches = OsVfsFindMpByPath(mp->mPath, path);
        if (matches > bestMatches) {
            bestMatches = matches;
            bestMp = mp;

            iPath = path + matches;
            while ((*iPath == '/') && (*(iPath + 1) != '/')) {
                iPath++;
            }

            if (pathInMp != NULL) {
                *pathInMp = path;
            }
        }
        mp = mp->mNext;
    }
    return bestMp;
}

S32 OsVfsFindMountPoint(const char *fsType)
{
    struct TagMountPoint *iter = NULL;
    for (iter = g_mountPoints; iter != NULL; iter = iter->mNext) {
        if ((iter->mFs != NULL) && (iter->mFs->fsType != NULL) &&
            strcmp(iter->mFs->fsType, fsType) == 0) {
            return FS_OK;
        }
    }
    return FS_NOK;
}

static void OsVfsFreeMp(struct TagMountPoint *mp)
{
    if (mp == NULL) {
        return;
    }
    if (mp->mPath != NULL) {
        free((void *)mp->mPath);
    }
    if (mp->mDev != NULL) {
        free((void *)mp->mDev);
    }
    free(mp);
}

S32 OsVfsMount(const char *source, const char *target,
                           const char *filesystemtype, uintptr_t mountflags,
                           const void *data)
{
    S32 ret;
    struct TagMountPoint *mp = NULL;
    struct TagFsMap *mFs = NULL;
    const char *pathInMp = NULL;

    if ((target == NULL) || (target[0] != '/')) {
        return FS_NOK;
    }
    (void)OsVfsLock();
    /* 确认是否已经被挂载 */
    mp = OsVfsFindMp(target, &pathInMp);
    if (mp != NULL && pathInMp != NULL) {
        OsVfsUnlock();
        return FS_NOK;
    }

    mFs = OsVfsGetFsMap(filesystemtype);
    if ((mFs == NULL) || (mFs->fsMops == NULL) || (mFs->fsMops->mount == NULL)) {
        OsVfsUnlock();
        return FS_NOK;
    }

    mp = (struct TagMountPoint *)malloc(sizeof(struct TagMountPoint));
    if (mp == NULL) {
        OsVfsUnlock();
        return FS_NOK;
    }
    if (memset_s(mp, sizeof(struct TagMountPoint), 0, sizeof(struct TagMountPoint)) != EOK) {
        free(mp);
        OsVfsUnlock();
        return FS_NOK;
    }

    mp->mFs = mFs;
    mp->mDev = NULL;
    if (source != NULL) {
        mp->mDev = strdup(source);
        if (mp->mDev == NULL) {
            OsVfsFreeMp(mp);
            OsVfsUnlock();
            return FS_NOK;
        }
    }
    mp->mPath = strdup(target);
    if (mp->mPath == NULL) {
        OsVfsFreeMp(mp);
        OsVfsUnlock();
        return FS_NOK;
    }

    ret = mp->mFs->fsMops->mount(mp, mountflags, data);
    if (ret != 0) {
        OsVfsFreeMp(mp);
        OsVfsUnlock();
        return FS_NOK;
    }
    mp->mRefs = 0;
    mp->mWriteEnable = (mountflags & MS_RDONLY) ? FALSE : TRUE;
    mp->mFs->fsRefs++;
    mp->mNext = g_mountPoints;
    g_mountPoints = mp;
    OsVfsUnlock();
    return FS_OK;
}

S32 OsVfsUmount(const char *target)
{
    struct TagMountPoint *mp = NULL;
    const char *pathInMp = NULL;
    S32 ret = FS_NOK;

    if (target == NULL) {
        return ret;
    }

    (void)OsVfsLock();
    mp = OsVfsFindMp(target, &pathInMp);
    if ((mp == NULL) || (mp->mRefs != 0) ||
        (mp->mFs == NULL) || (mp->mFs->fsMops == NULL) ||
        (mp->mFs->fsMops->umount == NULL)) {
        OsVfsUnlock();
        return ret;
    }

    ret = mp->mFs->fsMops->umount(mp);
    if (ret != 0) {
        OsVfsUnlock();
        return ret;
    }

    OsMpDeleteFromList(mp);
    mp->mFs->fsRefs--;
    free((void *)mp->mPath);
    free((void *)mp->mDev);
    free(mp);

    OsVfsUnlock();
    return FS_OK;
}

static void OsCloseFdsInMp(struct TagMountPoint *mp)
{
    for (S32 fd = 0; fd < NR_OPEN_DEFAULT; fd++) {
        struct TagFile *f = OsFdToFile(fd);
        if (f == NULL) {
            continue;
        }
        if ((f->fMp == mp) &&
            (f->fFops != NULL) &&
            (f->fFops->close != NULL)) {
            (void)f->fFops->close(f);
        }
    }
}
S32 OsVfsUmount2(const char *target, S32 flag)
{
    struct TagMountPoint *mp = NULL;
    const char *pathInMp = NULL;
    S32 ret = FS_NOK;

    if (target == NULL) {
        return ret;
    }

    (void)OsVfsLock();
    mp = OsVfsFindMp(target, &pathInMp);
    if ((mp == NULL) || (mp->mRefs != 0) ||
        (mp->mFs == NULL) || (mp->mFs->fsMops == NULL) ||
        (mp->mFs->fsMops->umount2 == NULL)) {
        OsVfsUnlock();
        return ret;
    }

    /* 关闭当前挂载节点下的所有文件 */
    if ((U32)flag & MNT_FORCE) {
        OsCloseFdsInMp(mp);
    }

    ret = mp->mFs->fsMops->umount2(mp, flag);
    if (ret != 0) {
        OsVfsUnlock();
        return FS_NOK;
    }

    OsMpDeleteFromList(mp);
    mp->mFs->fsRefs--;
    free((void *)mp->mPath);
    free((void *)mp->mDev);
    free(mp);

    OsVfsUnlock();
    return FS_OK;
}

