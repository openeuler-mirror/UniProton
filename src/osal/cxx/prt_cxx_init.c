/*
 * Copyright (c) 2009-2022 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Create: 2022-08-12
 * Description: C++ adaption
 */
#include <stdio.h>
#include "prt_sys.h"

extern void (*__init_array_start []) (void);
extern void (*__init_array_end []) (void);
extern void (*__fini_array_start []) (void);
extern void (*__fini_array_end []) (void);

void __sync_synchronize(void) {}

void PRT_CppSystemInit(void)
{
    U32 count;
    U32 i;
    count = __init_array_end - __init_array_start;
    for (i = 0; i < count; i++) {
        __init_array_start[i] ();
    }
}

void PRT_CppSystemFini(void)
{
    U32 count;
    U32 i;
    count = __fini_array_end - __fini_array_start;
    for (i = 0; i < count; i++) {
        __fini_array_start[i] ();
    }
}