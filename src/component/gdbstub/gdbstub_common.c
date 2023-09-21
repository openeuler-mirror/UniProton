/*
 * Copyright (c) 2023-2023 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 * 	http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * See the Mulan PSL v2 for more details.
 * Create: 2023-09-14
 * Description: gdbstub通用部分，包括流程控制，软件断点管理等
 */

#include <stddef.h>
#include <errno.h>
#include "prt_typedef.h"
#include "prt_gdbstub_ext.h"
#include "rsp_utils.h"
#include "arch_interface.h"
#include "gdbstub_common.h"

#define GDB_EXCEPTION_BREAKPOINT            5
#define GDB_PACKET_SIZE                     2048

#define GDB_ENO_NOT_SUPPORT                 2

/* GDB remote serial protocol does not define errors value properly
 * and handle all error packets as the same the code error is not
 * used. There are informal values used by others gdbstub
 * implementation, like qemu. Lets use the same here.
 */
#define GDB_ERROR_GENERAL   "E01"
#define GDB_ERROR_MEMORY    "E14"
#define GDB_ERROR_INVAL     "E22"

#define CHECK_ERROR(condition) {        \
        if ((condition)) {              \
            return -1;                  \
        }                               \
    }

#define CHECK_CHAR(c)                               \
    {                                               \
        CHECK_ERROR(ptr == NULL || *ptr != (c));    \
        ptr++;                                      \
    }

enum LoopState {
    RECEIVING,
    EXIT,
} state;

static STUB_DATA char g_notFirstStart;

/*
 * Holds information about breakpoints.
 */
static STUB_DATA struct GdbBkpt g_breaks[GDB_MAX_BREAKPOINTS] = {
    [0 ... GDB_MAX_BREAKPOINTS - 1] = { .state = BP_UNDEFINED }
};

static STUB_DATA U8 g_serialBuf[GDB_PACKET_SIZE];

static STUB_TEXT int HexCh2Bin(unsigned char ch)
{
    unsigned char cu = ch & 0xdf; // lowercase to uppercase
    return -1 +
        ((ch - '0' +  1) & (unsigned)((ch - '9' - 1) & ('0' - 1 - ch)) >> 8) +
        ((cu - 'A' + 11) & (unsigned)((cu - 'F' - 1) & ('A' - 1 - cu)) >> 8);
}

/*
 * While we find nice hex chars, build a long val.
 * Return number of chars processed.
 */
static STUB_TEXT int OsGdbHex2U64(char **ptr, U64 *val, int maxlen)
{
    int hex;
    int num = 0;
    int negate = 0;

    if (ptr == NULL || *ptr == NULL || val == NULL || maxlen < 0) {
        return 0;
    }

    *val = 0;

    if (**ptr == '-') {
        negate = 1;
        (*ptr)++;
    }
    while (**ptr && num < maxlen) {
        hex = HexCh2Bin(**ptr);
        if (hex < 0)
            break;

        *val = (*val << 4) | hex;
        num++;
        (*ptr)++;
    }

    if (negate)
        *val = -*val;

    return num;
}

#define CHECK_HEX(arg)  do {                                    \
    U64 v = 0;                                                  \
    OsGdbHex2U64((char **)&ptr, &v, 2 * sizeof(v));             \
    CHECK_ERROR(ptr == NULL);                                   \
    arg =( __typeof__(arg)) v;                                  \
} while(0)

#define CHECK_HEXS(arg, cnt)  do {                              \
    U64 v = 0;                                                  \
    cnt = OsGdbHex2U64((char **)&ptr, &v, 2 * sizeof(v));       \
    CHECK_ERROR(ptr == NULL);                                   \
    arg =( __typeof__(arg)) v;                                  \
} while(0)

int __weak STUB_TEXT OsGdbConfigGetMemRegions(struct GdbMemRegion **regions)
{
    (void)(regions);
    return 0;
}

static STUB_TEXT int GdbAddrCheck(uintptr_t addr, int attr)
{
    struct GdbMemRegion *regions = NULL;
    int cnt = OsGdbConfigGetMemRegions(&regions);
    if (!cnt) {
        return -EINVAL;
    }
    for (int i = 0; i < cnt; i++) {
        if (addr >= regions[i].start && 
            addr < regions[i].end && 
            (regions[i].attributes & attr) == attr) {
            return 0;
        }
    }
    return -EINVAL;
}

static STUB_TEXT int GdbInvalidReadAddr(uintptr_t addr)
{
    return GdbAddrCheck(addr, GDB_MEM_REGION_READ);
}

static STUB_TEXT int GdbInvalidWriteAddr(uintptr_t addr)
{
    return GdbAddrCheck(addr, GDB_MEM_REGION_WRITE);
}

static STUB_TEXT int GdbInvalidBkptAddr(uintptr_t addr)
{
    return !GdbAddrCheck(addr, GDB_MEM_REGION_NO_SWBKPT);
}

/*
 * Some architectures need cache flushes when we set/clear a
 * breakpoint:
 */
void __weak STUB_TEXT GdbFlushSwBkptAddr(uintptr_t addr)
{
    (void)(addr);
    /* Force flush instruction cache */
}

/*
 * SW breakpoint management:
 */
static STUB_TEXT int GdbSetSwBkpt(uintptr_t addr)
{
    int breakno = -1;
    int i;

    for (i = 0; i < GDB_MAX_BREAKPOINTS; i++) {
        if (g_breaks[i].state == BP_SET &&
            g_breaks[i].addr == addr)
            return -EEXIST;
    }
    for (i = 0; i < GDB_MAX_BREAKPOINTS; i++) {
        if (g_breaks[i].state == BP_REMOVED &&
            g_breaks[i].addr == addr) {
            breakno = i;
            break;
        }
    }

    if (breakno == -1) {
        for (i = 0; i < GDB_MAX_BREAKPOINTS; i++) {
            if (g_breaks[i].state == BP_UNDEFINED) {
                breakno = i;
                break;
            }
        }
    }

    if (breakno == -1)
        return -E2BIG;

    g_breaks[breakno].state = BP_SET;
    g_breaks[breakno].type = BP_BREAKPOINT;
    g_breaks[breakno].addr = addr;

    return 0;
}

static STUB_TEXT int GdbActivateSwBkpts(void)
{
    int error;
    int ret = 0;
    int i;

    for (i = 0; i < GDB_MAX_BREAKPOINTS; i++) {
        if (g_breaks[i].state != BP_SET)
            continue;

        error = OsGdbArchSetSwBkpt(&g_breaks[i]);
        if (error) {
            ret++;
            continue;
        }

        GdbFlushSwBkptAddr(g_breaks[i].addr);
        g_breaks[i].state = BP_ACTIVE;
    }
    return ret;
}

static STUB_TEXT int GdbDeactivateSwBkpts(void)
{
    int ret = 0;
    int i;

    for (i = 0; i < GDB_MAX_BREAKPOINTS; i++) {
        if (g_breaks[i].state != BP_ACTIVE)
            continue;
        ret = OsGdbArchRemoveSwBkpt(&g_breaks[i]);

        GdbFlushSwBkptAddr(g_breaks[i].addr);
        g_breaks[i].state = BP_SET;
    }
    return ret;
}

static STUB_TEXT int GdbRemoveSwBkpt(uintptr_t addr)
{
    int i;

    for (i = 0; i < GDB_MAX_BREAKPOINTS; i++) {
        if ((g_breaks[i].state == BP_SET) &&
                (g_breaks[i].addr == addr)) {
            g_breaks[i].state = BP_REMOVED;
            return 0;
        }
    }
    return -ENOENT;
}

static STUB_TEXT int GdbResetBkpts(void)
{
    int i;

    /* Clear memory breakpoints. */
    for (i = 0; i < GDB_MAX_BREAKPOINTS; i++) {
        g_breaks[i].state = BP_UNDEFINED;
    }

    return 0;
}

static STUB_TEXT int GdbAddBkpt(U8 type, uintptr_t addr, U32 kind)
{
    if (type != BP_BREAKPOINT) {
        return -GDB_ENO_NOT_SUPPORT;
    }
    if (GdbInvalidBkptAddr(addr)) {
        return -EINVAL;
    }
    return GdbSetSwBkpt(addr);
}

static STUB_TEXT int GdbRemoveBkpt(U8 type, uintptr_t addr, U32 kind)
{
    if (type != BP_BREAKPOINT) {
        return -GDB_ENO_NOT_SUPPORT;
    }
    if (GdbInvalidBkptAddr(addr)) {
        return -EINVAL;
    }

    return GdbRemoveSwBkpt(addr);
}

/* Read memory byte-by-byte */
static STUB_TEXT int GdbRawMemRead(U8 *buf, int buf_len, uintptr_t addr, int len)
{
    U8 data;
    int count = 0;
    int pos;

    /* Read from system memory */
    for (pos = 0; pos < len; pos++) {
        data = *(U8 *)(addr + pos);
        count += OsGdbBin2Hex(&data, sizeof(data), buf + count, buf_len - count);
    }

    return count;
}

/* Write memory byte-by-byte */
static STUB_TEXT int GdbRawMemWrite(const U8 *buf, uintptr_t addr, int len)
{
    U8 data;
    int count = 0;

    while (len > 0) {
        int cnt = OsGdbHex2Bin(buf, 2, &data, sizeof(data));
        if (cnt == 0) {
            return -1;
        }
        *(U8 *)addr = data;
        count += cnt;
        addr++;
        buf += 2;
        len--;
    }
    return count;
}

static STUB_TEXT int GdbCmdMemRead(U8 *ptr)
{
    int len;
    uintptr_t addr;
    int ret;

    CHECK_HEX(addr);
    CHECK_CHAR(',');
    CHECK_HEX(len);

    /* Read Memory */

    /*
     * GDB ask the guest to read parameters when
     * the user request backtrace. If the
     * parameter is a NULL pointer this will cause
     * a fault. Just send a packet informing that
     * this address is invalid
     */
    if (GdbInvalidReadAddr(addr)) {
        OsGdbSendPacket(GDB_ERROR_MEMORY, 3);
        return 0;
    }
    ret = GdbRawMemRead(g_serialBuf, sizeof(g_serialBuf), addr, len);
    CHECK_ERROR(!ret);
    OsGdbSendPacket(g_serialBuf, ret);
    return ret;
}

static STUB_TEXT int GdbCmdMemWrite(U8 *ptr)
{
    int len;
    uintptr_t addr;

    CHECK_HEX(addr);
    CHECK_CHAR(',');
    CHECK_HEX(len);
    CHECK_CHAR(':');

    if (GdbInvalidWriteAddr(addr)) {
        OsGdbSendPacket(GDB_ERROR_MEMORY, 3);
        return 0;
    }

    /* Write Memory */
    len = GdbRawMemWrite(ptr, addr, len);
    CHECK_ERROR(len < 0);
    OsGdbSendPacket("OK", 2);
    return 0;
}

static STUB_TEXT int GdbCmdBreak(U8 *ptr)
{
    U32 kind;
    uintptr_t addr;
    U8 type;
    int ret = 0;

    CHECK_HEX(type);
    CHECK_CHAR(',');
    CHECK_HEX(addr);
    CHECK_CHAR(',');
    CHECK_HEX(kind);

    if (g_serialBuf[0] == 'Z') {
        ret = GdbAddBkpt(type, addr, kind);
    } else if (g_serialBuf[0] == 'z') {
        ret = GdbRemoveBkpt(type, addr, kind);
    }

    if (ret == -GDB_ENO_NOT_SUPPORT) {
        /* breakpoint/watchpoint not supported */
        OsGdbSendPacket(NULL, 0);
    } else if (ret < 0) {
        OsGdbSendPacket(GDB_ERROR_INVAL, 3);
    } else {
        OsGdbSendPacket("OK", 2);
    }
    return 0;
}

static STUB_TEXT int GdbCmdReadReg(U8 *ptr)
{
    U32 regno;
    int ret;

    CHECK_HEX(regno);
    ret = OsGdbArchReadReg(regno, g_serialBuf, sizeof(g_serialBuf));
    CHECK_ERROR(ret == 0)
    OsGdbSendPacket(g_serialBuf, ret);
    return 0;
}

static STUB_TEXT int GdbCmdWriteReg(U8 *ptr, int len)
{
    U32 regno;
    int ret;
    int cnt;

    CHECK_HEXS(regno, cnt);
    CHECK_CHAR('='); 
    ret = OsGdbArchWriteReg(regno, ptr, len - cnt - 1);
    CHECK_ERROR(ret < 0)
    OsGdbSendPacket("OK", 2);
    return 0;
}

/**
 * Synchronously communicate with gdb on the host
 */
static STUB_TEXT int GdbSerialStub()
{
    state = RECEIVING;

    /* Only send exception if this is not the first
     * GDB break.
     */
    if (g_notFirstStart) {
        // GDB_EXCEPTION_BREAKPOINT: DEBUG & BREAKPOINT:
        OsGdbSendException(g_serialBuf, sizeof(g_serialBuf), GDB_EXCEPTION_BREAKPOINT);
    } else {
        g_notFirstStart = 1;
    }

    while (state == RECEIVING) {
        U8 *ptr;
        int len;
        int ret;

        ret = OsGdbGetPacket(g_serialBuf, sizeof(g_serialBuf), &len);
        if ((ret == -GDB_RSP_ENO_CHKSUM) || (ret == -GDB_RSP_ENO_2BIG)) {
            /*
             * Send error and wait for next packet.
             *
             */
            OsGdbSendPacket(GDB_ERROR_GENERAL, 3);
            continue;
        }

        if (len == 0) {
            continue;
        }

        ptr = g_serialBuf;
        ret = 0;
        switch (*ptr++) {

        /**
         * Read from the memory
         * Format: m addr,length
         */
        case 'm':
            ret = GdbCmdMemRead(ptr);
            break;

        /**
         * Write to memory
         * Format: M addr,length:val
         */
        case 'M':
            ret = GdbCmdMemWrite(ptr);
            break;

        /*
         * Continue ignoring the optional address
         * Format: c addr
         */
        case 'c':
            OsGdbArchContinue();
            /* We reset PC passively when receiving P/G packet. */
            OsGdbSendPacket("OK", 2);
            state = EXIT;
            break;

        /*
         * Step one instruction ignoring the optional address
         * s addr..addr
         */
        case 's':
            OsGdbArchStep();
            state = EXIT;
            break;

        /*
         * Read all registers
         * Format: g
         */
        case 'g':
            len = OsGdbArchReadAllRegs(g_serialBuf, sizeof(g_serialBuf));
            CHECK_ERROR(len == 0);
            OsGdbSendPacket(g_serialBuf, len);
            break;

        /**
         * Write the value of the CPU registers
         * Format: G XX...
         */
        case 'G':
            len = OsGdbArchWriteAllRegs(ptr, len - 1);
            CHECK_ERROR(len == 0);
            OsGdbSendPacket("OK", 2);
            break;

        /*
         * Read register
         * Format: p N
         */
        case 'p':
            ret = GdbCmdReadReg(ptr);
            break;

        /**
         * Write the value of the CPU register
         * Format: P N...=XXX...
         */
        case 'P':
            ret = GdbCmdWriteReg(ptr, len - 1);
            break;

        /*
         * Breakpoints
         */
        case 'z': /* __fallthrough */
        case 'Z':
            ret = GdbCmdBreak(ptr);
            break;

        /* What cause the pause  */
        case '?':
            OsGdbSendException(g_serialBuf, sizeof(g_serialBuf),
                       GDB_EXCEPTION_BREAKPOINT);
            break;

        case 'H':
            if (*ptr == 'g' || *ptr == 'c' || *ptr == 's') {
                OsGdbSendPacket("OK", 2);
            } else {
                OsGdbSendPacket(NULL, 0);
            }
            break;

        /* Exit debug  */
        case 'k':
            GdbResetBkpts();
            OsGdbArchContinue();
            OsGdbSendPacketNoAck("OK", 2);
            state = EXIT;
            break;

        /*
         * Not supported action
         */
        default:
            OsGdbSendPacket(NULL, 0);
            break;
        } /* switch */

        /*
         * If this is an recoverable error, send an error message to
         * GDB and continue the debugging session.
         */
        if (ret < 0) {
            OsGdbSendPacket(GDB_ERROR_GENERAL, 3);
            state = RECEIVING;
        }

    } /* while */
    return 0;
}

STUB_TEXT void OsGdbHandleException(void *stk)
{
    OsGdbArchPrepare(stk);
    GdbDeactivateSwBkpts();
    GdbSerialStub();
    GdbActivateSwBkpts();
    OsGdbArchFinish(stk);
}

extern int OsGdbIOInit();

STUB_TEXT int OsGdbStubInit(void)
{
    if (OsGdbIOInit()) {
        return -1;
    }
    OsGdbArchInit();
    return 0;
}

