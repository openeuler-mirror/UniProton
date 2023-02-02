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
 * Create: 2009-12-22
 * Description: 错误处理内部头文件
 */
#ifndef PRT_ERR_EXTERNAL_H
#define PRT_ERR_EXTERNAL_H

#include "prt_err.h"

extern void OsErrHandle(const char *fileName, U32 lineNo, U32 errorNo, U32 paraLen, void *para);
extern void OsErrRecord(U32 errorNo);
/*
 * 模块间宏定义
 */
#define OS_ERR_RECORD_N 3  // 表示保留二进制低n位，计算OS_ERR_RECORD_NUM用，OS_ERR_RECORD_NUM为2的n次方+1

#define OS_ERR_RECORD_NUM \
    ((1U << OS_ERR_RECORD_N) + 1)  // 2的n次方+1，目前n取3，NUM为9，第一次错误固定记录，后面8个循环记录。

#define OS_ERR_MAGIC_WORD 0xa1b2d4f8

#define OS_ERR_LEVEL_HIGH 0
#define OS_ERR_LEVEL_LOW 2
#define OS_LOG_LEVEL_FORCE 0xFF

#define OS_REPORT_ERROR(errNo)                                       \
    do {                                                             \
        OsErrHandle("os_file", OS_ERR_MAGIC_WORD, (errNo), 0, NULL); \
    } while (0)

/* 用于OS_OPTION_NFTL_ERR_POST_PROC */
#define OS_ERROR_TYPE_NUM (ERRTYPE_FATAL >> 24) /* 中断中需要延后处理的错误类型(所有低于FATAL等级的类型)数 */

/* errno中错误大类的类型掩码 */
#define OS_ERROR_TYPE_MASK (0xffU << 24)

#define OS_ERROR_LOG_REPORT(traceLevel, format, ...)

#if defined(OS_DBG)
#define LOG_ADDR_DBG(addr) (addr)
#define OS_LOG_REPORT_DBG(format, ...) OS_ERROR_LOG_REPORT(OS_ERR_LEVEL_HIGH, (format), ##__VA_ARGS__)
#else
#define LOG_ADDR_DBG(addr) ((void)(addr), 0x0U)
#define OS_LOG_REPORT_DBG(format, ...)
#endif

extern void OsErrRecordInCda(U32 errorNo);
extern U32 OsFatalErrClr(void);

// 函数返回值void的，都通过此接口记录返回值到cda
#define OS_ERR_RECORD(errorNo)          \
    do {                                \
        U32 errorNo_ = (errorNo);       \
        if (errorNo_ != OS_OK) {       \
            OsErrRecordInCda(errorNo_); \
        }                               \
    } while (0)

#endif /* PRT_ERR_EXTERNAL_H */
