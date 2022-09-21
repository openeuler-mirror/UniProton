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
#include "errno.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "vfs_config.h"
#include "vfs_files.h"
#include "vfs_mount.h"
#include "vfs_operations.h"
#include "prt_fs.h"

static struct TagFile g_files[NR_OPEN_DEFAULT];

S32 OsFileToFd(struct TagFile *file)
{
    if (file == NULL) {
        return FS_NOK;
    }
    return file - g_files + MIN_START_FD;
}

struct TagFile *OsFdToFile(S32 fd)
{
    if ((fd < MIN_START_FD) || (fd >= CONFIG_NFILE_DESCRIPTORS)) {
        return NULL;
    }
    return &g_files[fd - MIN_START_FD];
}

struct TagFile *OsVfsGetFile(void)
{
    /* protected by g_fsMutex */
    for (S32 i = 0; i < NR_OPEN_DEFAULT; i++) {
        if (g_files[i].fStatus == FILE_STATUS_NOT_USED) {
            g_files[i].fStatus = FILE_STATUS_INITING;
            return &g_files[i];
        }
    }

    return NULL;
}

struct TagFile *OsVfsGetFileSpec(S32 fd)
{
    if ((fd < MIN_START_FD) || (fd >= CONFIG_NFILE_DESCRIPTORS)) {
        return NULL;
    }
    if (g_files[fd - MIN_START_FD].fStatus == FILE_STATUS_NOT_USED) {
        g_files[fd - MIN_START_FD].fStatus = FILE_STATUS_INITING;
        return &g_files[fd - MIN_START_FD];
    }

    return NULL;
}

void OsVfsPutFile(struct TagFile *file)
{
    if (file == NULL) {
        return;
    }
    file->fFlags = 0;
    file->fFops = NULL;
    file->fData = NULL;
    file->fMp = NULL;
    file->fOffset = 0;
    file->fOwner = -1;
    file->fullPath = NULL;
    file->fStatus = FILE_STATUS_NOT_USED;
}
