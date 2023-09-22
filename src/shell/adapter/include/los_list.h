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
 * Create: 2023-08-25
 * Description: shell los_list 适配头文件。
 */
#ifndef _LOS_LIST_H
#define _LOS_LIST_H

#include "prt_list_external.h"
#include "los_base.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#ifndef LOS_ListEmpty
#define LOS_ListEmpty ListEmpty
#endif

#ifndef LOS_ListInit
#define LOS_ListInit INIT_LIST_OBJECT
#endif

#ifndef LOS_DL_LIST_FOR_EACH_ENTRY
#define LOS_DL_LIST_FOR_EACH_ENTRY LIST_FOR_EACH
#endif

#ifndef LOS_DL_LIST_ENTRY
#define LOS_DL_LIST_ENTRY LIST_COMPONENT
#endif

#ifndef LOS_ListDelete
#define LOS_ListDelete ListDelete
#endif

typedef struct TagListObject LOS_DL_LIST;

VOID LOS_ListTailInsert(LOS_DL_LIST *list, LOS_DL_LIST *node);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* _LOS_LIST_H */