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
 * Create: 2024-03-19
 * Description: 日志功能头文件。
 */
#ifndef PRT_LOG_H
#define PRT_LOG_H

#include "prt_typedef.h"
#include "prt_buildef.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

// 需要为日志分配17MB内存
#define SHM_MAP_SIZE 0x1100000UL

enum OsLogLevel {
    OS_LOG_EMERG = 0,
    OS_LOG_ALERT,
    OS_LOG_CRIT,
    OS_LOG_ERR,
    OS_LOG_WARN,
    OS_LOG_NOTICE,
    OS_LOG_INFO,
    OS_LOG_DEBUG,
    OS_LOG_NONE,    /* 仅用于过滤, 并非实际的日志级别 */
};

enum OsLogFacility {
    OS_LOG_F0 = 16,
    OS_LOG_F1,
    OS_LOG_F2,
    OS_LOG_F3,
    OS_LOG_F4,
    OS_LOG_F5,
    OS_LOG_F6,
    OS_LOG_F7,
};

#if defined(OS_OPTION_LOG)

extern U32 PRT_Log(enum OsLogLevel level, enum OsLogFacility facility, const char *str, size_t strLen);

extern U32 PRT_LogFormat(enum OsLogLevel level, enum OsLogFacility facility, const char *fmt, ...)
    __attribute__((format(printf,3,4)));

/* 需要 17MB 共享内存 */
extern U32 PRT_LogInit(uintptr_t memBase);

extern void PRT_LogOn(void);

extern void PRT_LogOff(void);

/* 设定级别的日志以及级别更低的日志将被过滤 */
/* 例如, 设定OS_LOG_NONE, 没有日志会被过滤 */
/* 例如, 设定OS_LOG_NOTICE, NOTICE, INFO, DEBUG级别的日志会被过滤 */
extern U32 PRT_LogSetFilter(enum OsLogLevel level);

/* 针对每种facility设定不同级别 */
extern U32 PRT_LogSetFilterByFacility(enum OsLogFacility facility, enum OsLogLevel level);

extern bool PRT_IsLogInit(void);

extern void PRT_LogGetStatus(U8 *switchStat, U8 *levelStat);

#else

#define PRT_Log(level, facility, str, strLen) 0
#define PRT_LogFormat(...) 0
#define PRT_LogInit(memBase) 0
#define PRT_LogOn()
#define PRT_LogOff()
#define PRT_LogSetFilter(level) 0
#define PRT_LogSetFilterByFacility(facility, level) 0
#define PRT_IsLogInit() true

#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#endif /* PRT_LOG_H */
