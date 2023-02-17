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
 * Create: 2022-11-22
 * Description: cache相关处理。
 */

    .section .os.init.text, "ax"

    .global OsCacheConfigInit
    .global OsCacheFlushDcache
    .global OsCacheInvIcache

    .type OsCacheConfigInit, "function"
    .type OsCacheFlushDcache, "function"
    .type OsCacheInvIcache, "function"

.macro GetCacheLineSize size, tmp
    mrs  \tmp, ctr_el0
    lsr  \tmp, \tmp, #16
    and  \tmp, \tmp, #0xf
    mov  \size, #4
    lsl  \size, \size, \tmp
.endm

OsCacheConfigInit:
    mov  w0, #0
    ret

/*
 * 描述：按地址范围进行DCache写回
 * 备注：当前仅用于动态加载模块
 */
OsCacheFlushDcache:
    GetCacheLineSize x2, x3
    sub  x3, x2, #1
    add  x1, x0, x1
    bic  x0, x0, x3
1:  dc   cvac, x0             /* Data Cache Clean by address to Point of Coherency */
    add  x0, x0, x2
    cmp  x0, x1
    b.lo 1b
    dsb  sy
    ret

/*
 * 描述：按地址范围进行ICache无效
 * 备注：当前仅用于动态加载模块
 */
OsCacheInvIcache:
    GetCacheLineSize x2, x3
    sub  x3, x2, #1
    add  x1, x0, x1
    bic  x0, x0, x3
1:  ic   ivau, x0             /* I cache Invalidate by address to Point of Unification */
    add  x0, x0, x2
    cmp  x0, x1
    b.lo 1b
    dsb  sy
    isb  sy
    ret

    .text
