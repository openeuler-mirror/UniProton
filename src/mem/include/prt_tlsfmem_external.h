/*
 * Copyright (c) 2009-2023 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/mulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Create: 2024-06-01
 * Description: TLSF 内存算法对外接口。TLSF 作为独立的一套内存分配算法，
 *              拥有自己的初始化与释放实现，由公共分派层 OsMemInit 按
 *              CONFIG_OS_MEM_ARITH_TLSF 宏选用。
 */
#ifndef PRT_TLSFMEM_EXTERNAL_H
#define PRT_TLSFMEM_EXTERNAL_H

#include "prt_mem_external.h"

/*
 * 模块间函数声明
 */
extern U32 OsTlsfMemInit(uintptr_t addr, U32 size);
extern U32 OsTlsfMemFree(void *addr);

#endif /* PRT_TLSFMEM_EXTERNAL_H */
