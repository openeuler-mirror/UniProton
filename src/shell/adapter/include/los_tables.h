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
 * Description: shell los_tables 适配头文件。
 */
#ifndef _LOS_TABLES_H
#define _LOS_TABLES_H

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */


#define OS_STRING(x)  #x
#define X_STRING(x) OS_STRING(x)

#ifndef LOS_LABEL_DEFN
#define LOS_LABEL_DEFN(label) label
#endif

#ifndef LOSARC_P2ALIGNMENT
#ifdef LOSCFG_AARCH64
#define LOSARC_P2ALIGNMENT 3
#else
#define LOSARC_P2ALIGNMENT 2
#endif
#endif

/* Assign a defined variable to a specific section */
#if !defined(LOSBLD_ATTRIB_SECTION)
#define LOSBLD_ATTRIB_SECTION(__sect__) __attribute__((section(__sect__)))
#endif

/*
 * Tell the compiler not to throw away a variable or function. Only known
 * available on 3.3.2 or above. Old version's didn't throw them away,
 * but using the unused attribute should stop warnings.
 */
#define LOSBLD_ATTRIB_USED __attribute__((used))

#ifndef LOS_HAL_TABLE_BEGIN
#define LOS_HAL_TABLE_BEGIN(label, name)                                     \
    __asm__(".section \".uniproton.table." X_STRING(name) ".begin\",\"aw\"\n"   \
            ".globl " X_STRING(LOS_LABEL_DEFN(label)) "\n"                   \
            ".type    " X_STRING(LOS_LABEL_DEFN(label)) ",object\n"          \
            ".p2align " X_STRING(LOSARC_P2ALIGNMENT) "\n"                    \
            X_STRING(LOS_LABEL_DEFN(label)) ":\n"                            \
            ".previous\n"                                                    \
            )
#endif

#ifndef LOS_HAL_TABLE_END
#define LOS_HAL_TABLE_END(label, name)                                       \
    __asm__(".section \".uniproton.table." X_STRING(name) ".finish\",\"aw\"\n"  \
            ".globl " X_STRING(LOS_LABEL_DEFN(label)) "\n"                   \
            ".type    " X_STRING(LOS_LABEL_DEFN(label)) ",object\n"          \
            ".p2align " X_STRING(LOSARC_P2ALIGNMENT) "\n"                    \
            X_STRING(LOS_LABEL_DEFN(label)) ":\n"                            \
            ".previous\n"                                                    \
            )
#endif

#ifndef LOS_HAL_TABLE_ENTRY
#define LOS_HAL_TABLE_ENTRY(name)                                  \
    LOSBLD_ATTRIB_SECTION(".uniproton.table." X_STRING(name) ".data") \
    LOSBLD_ATTRIB_USED
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* _LOS_TABLES_H */