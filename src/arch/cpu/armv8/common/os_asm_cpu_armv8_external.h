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
 * Description: ARMV8 公共汇编宏文件。
 */
#ifndef OS_ASM_CPU_ARMV8_EXTERNAL_H
#define OS_ASM_CPU_ARMV8_EXTERNAL_H

/*
 *  描述: 读取当前核号
 *        使用mpidr 寄存器 (64bit) 根据核的线程模式获取核号
 *        bit 63-40 39-32   31  30 29~25  24 23-16  15-8   7~0
 *             res0  aff3  res1  u  res0  mt  aff2  aff1  aff0
 */
.macro OsAsmGetCoreId, xArg
    mrs \xArg, mpidr_el1
#if (OS_MAX_CORE_NUM > 4)
    orr \xArg, \xArg, \xArg, lsr #0xe
    orr \xArg, \xArg, \xArg, lsr #0x8
    and \xArg, \xArg, #0xf /* 截取核号部分 */
#else
    orr \xArg, \xArg, \xArg, lsr #0x8
    and \xArg, \xArg, #0xff /* 截取核号部分 */
#endif
.endm

#endif /* OS_ASM_CPU_ARMV8_EXTERNAL_H */
