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
 * Description: 读写锁模块内部头文件。
 */
#ifndef PRT_RWLOCK_INTERNAL_H
#define PRT_RWLOCK_INTERNAL_H

#include "prt_buildef.h"
#include "prt_typedef.h"
#include "prt_list_external.h"

#define RWLOCK_COUNT_MASK 0x0000FFFFU
#define RWLOCK_MAGIC_NUM  0xFDCAU

enum RwlockMode {
    RWLOCK_NONE_MODE,
    RWLOCK_READ_MODE,
    RWLOCK_WRITE_MODE,
    RWLOCK_READFIRST_MODE,
    RWLOCK_WRITEFIRST_MODE
};

enum RwlockType {
    RWLOCK_RD,
    RWLOCK_TRYRD,
    RWLOCK_TIMERD,
    RWLOCK_WR,
    RWLOCK_TRYWR,
    RWLOCK_TIMEWR
};

#if defined(OS_POSIX_TYPE_NEWLIB)
typedef struct prt_pthread_rwlock_s {
    U32 rw_magic : 16;
    U32 index : 16;
    int rw_count;
    void *rw_owner;
    struct prt_pthread_rwlock_s *next;
    struct TagListObject rw_write;
    struct TagListObject rw_read;
} prt_pthread_rwlock_t;
#else
typedef pthread_rwlock_t prt_pthread_rwlock_t;
#endif

extern U32 OsRwLockRdPend(prt_pthread_rwlock_t *rwl, U32 timeout, U32 rwType);
extern U32 OsRwLockWrPend(prt_pthread_rwlock_t *rwl, U32 timeout, U32 rwType);
extern U32 OsRwLockUnlock(prt_pthread_rwlock_t *rwl, bool *needSched);

#endif /* PRT_RWLOCK_INTERNAL_H */
