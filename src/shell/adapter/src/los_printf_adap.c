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
 * Description: shell printf实现。
 */
#include "string.h"
#include "securec.h"
#include "los_printf.h"
#include "los_memory.h"
#include "uart.h"
#ifdef LOSCFG_FS_VFS
#include "console.h"
#endif
#ifdef LOSCFG_SHELL_DMESG
#include "dmesg_pri.h"
#endif
#ifdef LOSCFG_SHELL_EXCINFO_DUMP
#include "los_exc_pri.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#define SIZEBUF  256

#define UART_WITH_LOCK     1
#define UART_WITHOUT_LOCK  0

typedef enum {
    NO_OUTPUT = 0,
    UART_OUTPUT = 1,
    CONSOLE_OUTPUT = 2,
    EXC_OUTPUT = 3
} OutputType;

STATIC VOID ErrorMsg(VOID)
{
    const CHAR *p = "Output illegal string! vsnprintf_s failed!\n";
    UartPuts(p, (U32)strlen(p), UART_WITH_LOCK);
}

STATIC VOID UartOutput(const CHAR *str, U32 len, bool isLock)
{
#ifdef OS_OPTION_SHELL_DMESG
    if (!OsCheckUartLock()) {
        UartPuts(str, len, isLock);
    }
    if (isLock != UART_WITHOUT_LOCK) {
        (VOID)OsLogMemcpyRecord(str, len);
    }
#else
    UartPuts(str, len, isLock);
#endif
}

STATIC VOID OutputControl(const CHAR *str, U32 len, OutputType type)
{
    switch (type) {
        case CONSOLE_OUTPUT:
#ifdef LOSCFG_KERNEL_CONSOLE
            if (ConsoleEnable() == TRUE) {
                (VOID)write(STDOUT_FILENO, str, (size_t)len);
                break;
            }
#endif
            /* fall-through */
        case UART_OUTPUT:
            UartOutput(str, len, UART_WITH_LOCK);
            break;
        case EXC_OUTPUT:
            UartOutput(str, len, UART_WITHOUT_LOCK);
            break;
        default:
            break;
    }
    return;
}

STATIC VOID OsVprintfFree(CHAR *buf, UINT32 bufLen)
{
    if (bufLen != SIZEBUF) {
        (VOID)LOS_MemFree(m_aucSysMem0, buf);
    }
}

STATIC VOID OsVprintf(const CHAR *fmt, va_list ap, OutputType type)
{
    INT32 len;
    const CHAR *errMsgMalloc = "OsVprintf, malloc failed!\n";
    const CHAR *errMsgLen = "OsVprintf, length overflow!\n";
    CHAR aBuf[SIZEBUF] = {0};
    CHAR *bBuf = NULL;
    UINT32 bufLen = SIZEBUF;

    bBuf = aBuf;
    len = vsnprintf_s(bBuf, bufLen, bufLen - 1, fmt, ap);
    if ((len == -1) && (*bBuf == '\0')) {
        /* parameter is illegal or some features in fmt dont support */
        ErrorMsg();
        return;
    }

    while (len == -1) {
        /* bBuf is not enough */
        OsVprintfFree(bBuf, bufLen);

        bufLen = bufLen << 1;
        if ((INT32)bufLen <= 0) {
            UartPuts(errMsgLen, (UINT32)strlen(errMsgLen), UART_WITH_LOCK);
            return;
        }
        bBuf = (CHAR *)LOS_MemAlloc(m_aucSysMem0, bufLen);
        if (bBuf == NULL) {
            UartPuts(errMsgMalloc, (UINT32)strlen(errMsgMalloc), UART_WITH_LOCK);
            return;
        }
        len = vsnprintf_s(bBuf, bufLen, bufLen - 1, fmt, ap);
        if (*bBuf == '\0') {
            /* parameter is illegal or some features in fmt dont support */
            (VOID)LOS_MemFree(m_aucSysMem0, bBuf);
            ErrorMsg();
            return;
        }
    }
    *(bBuf + len) = '\0';
    OutputControl(bBuf, len, type);
    OsVprintfFree(bBuf, bufLen);
}

__attribute__ ((noinline)) VOID shprintf(const CHAR *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    OsVprintf(fmt, ap, CONSOLE_OUTPUT);
    va_end(ap);
}

VOID LkDprintf(const CHAR *fmt, va_list ap)
{
    OsVprintf(fmt, ap, CONSOLE_OUTPUT);
}

#ifdef LOSCFG_SHELL_DMESG
VOID DmesgPrintf(const CHAR *fmt, va_list ap)
{
    OsVprintf(fmt, ap, CONSOLE_OUTPUT);
}
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */