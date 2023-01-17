/*
 * Copyright (c) 2009-2023 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Create: 2009-09-17
 * Description: Fsc 内存实现
 */
#ifndef PRT_FSCMEM_EXTERNAL_H
#define PRT_FSCMEM_EXTERNAL_H

#include "prt_mem_external.h"

/*
 * 模块间宏定义
 */
#define OS_FSC_MEM_SLICE_HEAD_SIZE sizeof(struct TagFscMemCtrl)

/*
 * 模块间函数声明
 */
extern U32 OsFscMemFree(void *addr);

#endif /* PRT_FSCMEM_EXTERNAL_H */
