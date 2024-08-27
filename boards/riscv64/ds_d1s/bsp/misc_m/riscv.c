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
 * Create: 2024-02-22
 * Description: 架构相关的杂项函数
 */
#include "riscv.h"


U64 r_mie()
{
    U64 x;
    OS_EMBED_ASM("csrr %0, mie":"=r"(x)::);
    return x;
}

U64 r_mstatus()
{
    U64 x;
    OS_EMBED_ASM("csrr %0, mstatus":"=r"(x)::);
    return x;
}

void w_mie(U64 x)
{
    OS_EMBED_ASM("csrw mie, %0"::"r"(x):);
}

void w_mstatus(U64 x)
{
    OS_EMBED_ASM("csrw mstatus, %0"::"r"(x):);
}
