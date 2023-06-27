/*
 * Copyright (c) 2022-2023 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Create: 2023-05-29
 * Description: 内部锁定义
 */
#ifndef PRT_LOCK_H
#define PRT_LOCK_H

#include <stdint.h>
#include <limits.h>
#include <prt_typedef.h>

extern int PRT_Sem_Lock(U32 *lock);
extern int PRT_Sem_Unlock(const U32 *lock);

#define LITE_LOCK_INITIALIZER UINT_MAX
#define LITE_LOCK_INIT(x) U32 x = LITE_LOCK_INITIALIZER

#define LITE_LOCK(x) PRT_Sem_Lock(&(x))
#define LITE_UNLOCK(x) PRT_Sem_Unlock(&(x))

#endif