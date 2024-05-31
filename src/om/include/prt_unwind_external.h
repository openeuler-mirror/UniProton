/*
 * Copyright (c) 2024-2024 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Create: 2024-05-13
 * Description: unwind模块的内部头文件
 */
#ifndef PRT_UNWIND_EXTERNAL_H
#define PRT_UNWIND_EXTERNAL_H

#include "prt_task_external.h"

extern void OsUnwindGetStackTrace(const struct TagTskCb *task, U32 *maxDepth, uintptr_t *list);

#endif /* PRT_HOOK_EXTERNAL_H */
