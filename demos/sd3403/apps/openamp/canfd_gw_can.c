/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2026. All rights reserved.
 * Description: OpenArm 网关的 MCP2518FD CAN FD 收发层实现（见 canfd_gw_can.h）
 */
#include "canfd_gw_can.h"
#include "cpu_config.h"

#define GC_RX_FIFO          1U
#define GC_TX_FIFO          2U
#define GC_CHUNK            15U         /* 单笔对象读写 <= 2 + 15×16 = 242B（FIFO 256B） */
#define GC_STA_NE           (1U << 0)   /* RX 非空 / TX 未满 */
#define GC_STA_EMPTY_FULL   (1U << 2)   /* RX 满 / TX 空 */
#define GC_STA_OV           (1U << 3)
#define GC_STA_CI(s)        (((s) >> 8) & 0x1FU)
#define GC_EXPECT_RXB       0x010U      /* TXQ（1×16B）排在 RX FIFO1 前 */
#define GC_EXPECT_TXB       0x210U
#define GC_UINC             0x01U       /* FIFOCON 字节 1：UINC */
#define GC_UINC_TXREQ       0x03U
#define GC_FRESET           0x04U
#define GC_TDC_AUTO         (2U << 16)  /* TDCMOD：自动测 TDCV */
#define GC_OBJ_T1           (8U | (1U << 6) | (1U << 7))    /* DLC 8、BRS、FDF */

#define GC_MCP_TSCON        0x014U
#define GC_MCP_INT_IE       0x01EU      /* CiINT 高半字：中断使能 */
#define GC_MCP_TREC         0x034U
#define GC_MCP_IOCON        0xE04U
#define GC_MCP_ECCCON       0xE0CU
#define GC_CICON_STEF       (1U << 19)
/* CiCON 字节 3 = TXBWS[3:0] | ABAT | REQOP[2:0]。本层 TXBWS 恒为 0（初始化写入），
 * 改模式直接写常量：读-改-写会把读错的字节（勘误 DS80000789 #1）写回芯片 */
#define GC_CICON_B3_ABAT    0x08U
#define GC_OPMOD_NORMAL     0U
#define GC_OPMOD_CONFIG     4U
#define GC_MODE_TIMEOUT     (SYS_COUNTER_FREQ / 10U)
#define GC_QUAL_ROUNDS      4U
#define GC_QUAL_CHUNK       128U

static U8 g_gcPat[GW_MCP_RAM_SIZE];
static U8 g_gcGot[GW_MCP_RAM_SIZE];
static U32 g_gcRng;

static U32 GcRand(void)
{
    g_gcRng ^= g_gcRng << 13;
    g_gcRng ^= g_gcRng >> 17;
    g_gcRng ^= g_gcRng << 5;
    return g_gcRng;
}

static U8 GcRd8(struct GwSpiBus *b, U32 addr)
{
    U8 v = 0;

    (void)GwMcpRead(b, addr, &v, 1U);
    return v;
}

/* 读 CiCON 可能读错（勘误 DS80000789 #1）：连续两次读到目标模式才算到达 */
static S32 GcWaitOpmod(struct GwSpiBus *b, U32 mode)
{
    U64 t0 = GwSpiNow();
    U32 hits = 0;

    while (hits < 2U) {
        hits = (((GwMcpRd32(b, GW_MCP_CICON) >> 21) & 7U) == mode) ? hits + 1U : 0U;
        if (hits < 2U && GwSpiNow() - t0 > GC_MODE_TIMEOUT) {
            return -1;
        }
    }
    return 0;
}

/* 12.5MHz 资格：SFR 已知值读 150 次 + 报文 RAM 随机图案写读回，0 错才用 12.5MHz。
 * 引脚驱动档须在启动前由 Linux 侧设到 >=3；没设时这里
 * 判出误码并退回 6.25MHz，误码可能把 READ 头变成 WRITE 写坏 SFR，故退回后再 RESET。 */
static void GcQualify(struct GwCanLane *ln)
{
    struct GwSpiBus *b = &ln->bus;
    U32 con = GwMcpRd32(b, GW_MCP_CICON);
    U32 osc = GwMcpRd32(b, GW_MCP_OSC);
    U32 devid = GwMcpRd32(b, GW_MCP_DEVID);
    U32 err0 = b->timeouts;
    U32 sfrErr = 0;
    U32 ramErr = 0xFFFFFFFFU;
    U32 k;
    U32 r;
    U32 off;

    (void)GwSpiSetClock(b, GW_SPI_FAST_HZ);
    for (k = 0; k < 50U; k++) {
        sfrErr += (GwMcpRd32(b, GW_MCP_CICON) != con) ? 1U : 0U;
        sfrErr += (GwMcpRd32(b, GW_MCP_OSC) != osc) ? 1U : 0U;
        sfrErr += (GwMcpRd32(b, GW_MCP_DEVID) != devid) ? 1U : 0U;
    }
    if (sfrErr == 0U) {
        ramErr = 0;
        for (r = 0; r < GC_QUAL_ROUNDS; r++) {
            g_gcRng = 0x9E3779B9U * (r + 1U) + b->id;
            for (k = 0; k < GW_MCP_RAM_SIZE; k++) {
                g_gcPat[k] = (U8)GcRand();
            }
            for (off = 0; off < GW_MCP_RAM_SIZE; off += GC_QUAL_CHUNK) {
                (void)GwMcpWrite(b, GW_MCP_RAM + off, g_gcPat + off, GC_QUAL_CHUNK);
            }
            for (off = 0; off < GW_MCP_RAM_SIZE; off += GC_QUAL_CHUNK) {
                (void)GwMcpRead(b, GW_MCP_RAM + off, g_gcGot + off, GC_QUAL_CHUNK);
            }
            for (k = 0; k < GW_MCP_RAM_SIZE; k++) {
                ramErr += (g_gcPat[k] != g_gcGot[k]) ? 1U : 0U;
            }
        }
    }
    if (sfrErr != 0U || ramErr != 0U || b->timeouts != err0) {
        (void)GwSpiSetClock(b, GW_SPI_SAFE_HZ);
        GwMcpReset(b);
    }
    ln->qualSfrErr = sfrErr;
    ln->qualRamErr = ramErr;
}

/* 16 位帧资格：报文 RAM 随机图案 16 位写、8 位读，再 8 位写、16 位读，0 错才用。
 * 字节序若反了，命令头会被芯片当成别的指令（如 RESET），故不过时同样 RESET 再配置。 */
static void GcQualifyWide(struct GwCanLane *ln)
{
    struct GwSpiBus *b = &ln->bus;
    U32 err0 = b->timeouts;
    U32 err = 0;
    U32 k;
    U32 r;
    U32 off;

    for (r = 0; r < GC_QUAL_ROUNDS; r++) {
        g_gcRng = 0x7F4A7C15U * (r + 1U) + b->id;
        for (k = 0; k < GW_MCP_RAM_SIZE; k++) {
            g_gcPat[k] = (U8)GcRand();
        }
        for (off = 0; off < GW_MCP_RAM_SIZE; off += GC_QUAL_CHUNK) {
            b->wide = 1U;
            (void)GwMcpWrite(b, GW_MCP_RAM + off, g_gcPat + off, GC_QUAL_CHUNK);
            b->wide = 0;
            (void)GwMcpRead(b, GW_MCP_RAM + off, g_gcGot + off, GC_QUAL_CHUNK);
        }
        for (k = 0; k < GW_MCP_RAM_SIZE; k++) {
            err += (g_gcPat[k] != g_gcGot[k]) ? 1U : 0U;
            g_gcPat[k] = (U8)~g_gcPat[k];
        }
        for (off = 0; off < GW_MCP_RAM_SIZE; off += GC_QUAL_CHUNK) {
            b->wide = 0;
            (void)GwMcpWrite(b, GW_MCP_RAM + off, g_gcPat + off, GC_QUAL_CHUNK);
            b->wide = 1U;
            (void)GwMcpRead(b, GW_MCP_RAM + off, g_gcGot + off, GC_QUAL_CHUNK);
        }
        for (k = 0; k < GW_MCP_RAM_SIZE; k++) {
            err += (g_gcPat[k] != g_gcGot[k]) ? 1U : 0U;
        }
    }
    b->wide = (err == 0U && b->timeouts == err0) ? 1U : 0U;
    if (b->wide == 0U) {
        GwMcpReset(b);
    }
    ln->qualWideErr = err;
}

S32 GwCanInit(struct GwCanLane *ln)
{
    struct GwSpiBus *b = &ln->bus;
    U32 off;
    U32 err0;
    U8 v;
    U8 ie[2];

    ln->init = GW_CAN_ERR_CLOCK;
    ln->txqHead = 0;
    ln->txqCount = 0;
    if (GwSpiInit(b, GW_SPI_SAFE_HZ) == 0U) {
        return ln->init;
    }
    GwMcpReset(b);
    if (GwMcpRd32(b, GW_MCP_CICON) != GW_MCP_CICON_RESET) {
        ln->init = GW_CAN_ERR_CHIP;
        return ln->init;
    }
    GcQualify(ln);
    GcQualifyWide(ln);
    err0 = b->timeouts;

    GwMcpWr8(b, GC_MCP_ECCCON, (U8)(GcRd8(b, GC_MCP_ECCCON) | 0x01U));     /* ECCEN */
    for (off = 0; off < GW_MCP_RAM_SIZE; off += GC_QUAL_CHUNK) {
        (void)GwMcpWrite(b, GW_MCP_RAM + off, NULL, GC_QUAL_CHUNK);           /* RAM 清零 */
    }
    GwMcpWr32(b, GW_MCP_CICON, GW_MCP_CICON_RESET & ~GC_CICON_STEF);
    GwMcpWr32(b, GW_MCP_FIFOCON(GC_TX_FIFO), ((GW_CAN_TX_DEPTH - 1U) << 24) |
        (3U << 21) | (1U << 16) | (1U << 7));                               /* TXAT 3、TXPRI 1、TXEN */
    GwMcpWr32(b, GW_MCP_FIFOCON(GC_RX_FIFO), (GW_CAN_RX_DEPTH - 1U) << 24);
    GwMcpWr8(b, GC_MCP_TSCON + 2U, (U8)(GcRd8(b, GC_MCP_TSCON + 2U) & ~0x01U));  /* TBCEN=0 */
    GwMcpWr32(b, GW_MCP_FLTOBJ0, 0U);
    GwMcpWr32(b, GW_MCP_MASK0, 0U);
    GwMcpWr8(b, GW_MCP_FLTCON0, (U8)(0x80U | GC_RX_FIFO));                  /* 过滤器 0 全收 → FIFO1 */
    GwMcpWr32(b, GW_MCP_NBTCFG, 0x001C0909U);                               /* 1Mbit/s，40TQ，75% */
    GwMcpWr32(b, GW_MCP_DBTCFG, 0x00040101U);                               /* 5Mbit/s，8TQ */
    GwMcpWr32(b, GW_MCP_TDC, GC_TDC_AUTO | ((ln->tdco & 0x7FU) << 8));      /* TDC 自动 */
    GwMcpWr8(b, GC_MCP_IOCON + 3U, (U8)(GcRd8(b, GC_MCP_IOCON + 3U) & ~0x03U));  /* PM0/PM1=中断脚 */
    (void)GwMcpRead(b, GC_MCP_INT_IE, ie, 2U);
    ie[0] |= 0x03U;                                                         /* TXIE、RXIE */
    (void)GwMcpWrite(b, GC_MCP_INT_IE, ie, 2U);
    v = GcRd8(b, GW_MCP_FIFOCON(GC_RX_FIFO));
    GwMcpWr8(b, GW_MCP_FIFOCON(GC_RX_FIFO), (U8)(v | 0x09U));               /* TFNRFNIE、RXOVIE */
    if (b->timeouts != err0) {
        ln->init = GW_CAN_ERR_SPI;
        return ln->init;
    }
    GwMcpWr8(b, GW_MCP_CICON + 3U, GC_OPMOD_NORMAL);
    if (GcWaitOpmod(b, GC_OPMOD_NORMAL) != 0) {
        ln->init = GW_CAN_ERR_MODE;
        return ln->init;
    }
    /* FIFOUA 配置模式下不保证可读，进正常模式后再取基址。此时两个 FIFO 都还没 UINC 过：
     * RX 的 UA（读尾）与 TX 的 UA（写头）都在 FIFO 基址；其间收到的帧只移动 RX 写头 */
    ln->rxBase = GwMcpRd32(b, GW_MCP_FIFOUA(GC_RX_FIFO));
    ln->txBase = GwMcpRd32(b, GW_MCP_FIFOUA(GC_TX_FIFO));
    if (ln->rxBase != GC_EXPECT_RXB || ln->txBase != GC_EXPECT_TXB) {
        ln->init = GW_CAN_ERR_LAYOUT;
        return ln->init;
    }
    ln->rxCon0 = GcRd8(b, GW_MCP_FIFOCON(GC_RX_FIFO));
    ln->txCon0 = GcRd8(b, GW_MCP_FIFOCON(GC_TX_FIFO));
    ln->init = GW_CAN_OK;
    return ln->init;
}

S32 GwCanSend(struct GwCanLane *ln, U32 id, const U8 data[8])
{
    struct GwCanTxEntry *e;
    U32 k;

    if (ln->txqCount >= GW_CAN_TXQ_DEPTH) {
        ln->txQueueFull++;
        return -1;
    }
    e = &ln->txq[(ln->txqHead + ln->txqCount) % GW_CAN_TXQ_DEPTH];
    e->id = id;
    for (k = 0; k < 8U; k++) {
        e->data[k] = data[k];
    }
    ln->txqCount++;
    return 0;
}

static void GcObjMake(U8 *o, U32 id, const U8 *data)
{
    U32 k;

    GwLe32Put(o, id & 0x7FFU);
    GwLe32Put(o + 4, GC_OBJ_T1);
    for (k = 0; k < 8U; k++) {
        o[8U + k] = data[k];
    }
}

/* 每路至多一笔（len[k]=0 不发）：先全部起笔、线上同时移位，再轮流收齐 */
static void GcRun(struct GwCanLane *const *ln, U32 n, const U32 *len, S32 *ret)
{
    struct GwSpiBus *b[GW_CAN_LANES] = { 0 };
    S32 r[GW_CAN_LANES];
    U32 m = 0;
    U32 k;

    for (k = 0; k < n; k++) {
        ret[k] = 0;
        if (len[k] != 0U) {
            GwSpiStart(&ln[k]->bus, len[k]);
            b[m++] = &ln[k]->bus;
        }
    }
    GwSpiFinishAll(b, m, r);
    for (k = 0, m = 0; k < n; k++) {
        if (len[k] != 0U) {
            ret[k] = r[m++];
        }
    }
}

/* 组一笔 UINC（flags 为 FIFOCON 字节 1：UINC / UINC|TXREQ），返回长度。16 位帧时从
 * 字节 0 写起凑成 4 字节：字节 0 只有配置模式才可改的 TXEN/RTREN/RXTSEN 与中断使能，
 * 回写初始化末读回的原值，不改变任何设置 */
static U32 GcUinc(struct GwCanLane *ln, U32 fifo, U8 flags)
{
    U8 d[2];

    if (ln->bus.wide == 0U) {
        return GwMcpPrepare(&ln->bus, GW_MCP_INS_WRITE, GW_MCP_FIFOCON(fifo) + 1U, &flags, 1U);
    }
    d[0] = (fifo == GC_RX_FIFO) ? ln->rxCon0 : ln->txCon0;
    d[1] = flags;
    return GwMcpPrepare(&ln->bus, GW_MCP_INS_WRITE, GW_MCP_FIFOCON(fifo), d, 2U);
}

static U32 GcObjDecode(const U8 *o, struct GwCanFrame *f)
{
    U32 r0 = GwLe32Get(o);
    U32 r1 = GwLe32Get(o + 4);
    U32 k;

    f->dlc = (U8)(r1 & 0xFU);
    f->ide = (U8)((r1 >> 4) & 1U);
    f->rtr = (U8)((r1 >> 5) & 1U);
    f->fdf = (U8)((r1 >> 7) & 1U);
    f->id = (f->ide != 0U) ? ((((r0 & 0x7FFU) << 18) | ((r0 >> 11) & 0x3FFFFU)) & 0x1FFFFFFFU) :
        (r0 & 0x7FFU);
    if (f->dlc > 8U) {
        return 0;                       /* PLSIZE_8 装不下：计错丢弃 */
    }
    for (k = 0; k < 8U; k++) {
        f->data[k] = (f->rtr == 0U && k < f->dlc) ? o[8U + k] : 0U;
    }
    return 1;
}

/* 由 FIFO1 STA 与读尾算占用（空为 0，非空且头尾重合 = 满） */
static U32 GcRxOcc(U32 sta, U32 tail)
{
    U32 occ;

    if ((sta & GC_STA_NE) == 0U) {
        return 0;
    }
    occ = (GC_STA_CI(sta) - tail) & (GW_CAN_RX_DEPTH - 1U);
    return (occ == 0U) ? GW_CAN_RX_DEPTH : occ;
}

/* 由 FIFO1 STA+UA 算读尾与占用；不可读（空、越界）返回 0 */
static U32 GcRxPlan(struct GwCanLane *ln, const U8 *sa, U32 *tail)
{
    U32 sta = GwLe32Get(sa);
    U32 rel = GwLe32Get(sa + 4) - ln->rxBase;

    if ((sta & GC_STA_OV) != 0U) {
        ln->rxOverflow++;
        GwMcpWr8(&ln->bus, GW_MCP_FIFOSTA(GC_RX_FIFO), 0U);  /* 清 RXOVIF，字节内其余位只读 */
    }
    if ((sta & GC_STA_NE) == 0U) {
        return 0;
    }
    if (rel >= GW_CAN_RX_DEPTH * GW_CAN_STRIDE || (rel % GW_CAN_STRIDE) != 0U) {
        ln->layoutErr++;
        return 0;
    }
    *tail = rel / GW_CAN_STRIDE;
    return GcRxOcc(sta, *tail);
}

void GwCanReceiveAll(struct GwCanLane *const *ln, U32 n, struct GwCanFrame *const *out, U32 *got)
{
    U32 len[GW_CAN_LANES] = { 0 };
    U32 occ[GW_CAN_LANES];
    U32 tail[GW_CAN_LANES];
    U32 done[GW_CAN_LANES];
    U32 cnt[GW_CAN_LANES];
    S32 ret[GW_CAN_LANES];
    U32 more;
    U32 most = 0;
    U32 i;
    U32 k;

    for (k = 0; k < n; k++) {
        got[k] = 0;
        occ[k] = 0;
        tail[k] = 0;
        done[k] = 0;
        len[k] = (ln[k]->init == GW_CAN_OK) ? GwMcpPrepare(&ln[k]->bus, GW_MCP_INS_READ,
            GW_MCP_FIFOSTA(GC_RX_FIFO), NULL, 8U) : 0U;
    }
    GcRun(ln, n, len, ret);
    for (k = 0; k < n; k++) {
        if (len[k] != 0U && ret[k] == 0) {
            occ[k] = GcRxPlan(ln[k], ln[k]->bus.rx + GW_MCP_HDR, &tail[k]);
        }
        len[k] = (occ[k] != 0U) ? GwMcpPrepare(&ln[k]->bus, GW_MCP_INS_READ,
            GW_MCP_FIFOSTA(GC_RX_FIFO), NULL, 4U) : 0U;
    }
    /* 勘误 #6：再读一次 STA。两读之间没有 UINC，占用只增不减，首读大于再读必是读错 */
    GcRun(ln, n, len, ret);
    for (k = 0; k < n; k++) {
        U32 again;

        if (len[k] == 0U) {
            continue;
        }
        again = (ret[k] == 0) ? GcRxOcc(GwLe32Get(ln[k]->bus.rx + GW_MCP_HDR), tail[k]) : 0U;
        if (again < occ[k]) {
            ln[k]->rxStaGlitch += (ret[k] == 0) ? 1U : 0U;
            occ[k] = again;
        }
        if (occ[k] > ln[k]->maxRxOcc) {
            ln[k]->maxRxOcc = occ[k];
        }
    }
    /* 对象整批读：一笔 <=15 帧，跨 FIFO 尾拆笔 */
    do {
        more = 0;
        for (k = 0; k < n; k++) {
            U32 idx = (tail[k] + done[k]) & (GW_CAN_RX_DEPTH - 1U);
            len[k] = 0;
            if (done[k] >= occ[k]) {
                continue;
            }
            cnt[k] = occ[k] - done[k];
            if (cnt[k] > GW_CAN_RX_DEPTH - idx) {
                cnt[k] = GW_CAN_RX_DEPTH - idx;
            }
            if (cnt[k] > GC_CHUNK) {
                cnt[k] = GC_CHUNK;
            }
            len[k] = GwMcpPrepare(&ln[k]->bus, GW_MCP_INS_READ,
                GW_MCP_RAM + ln[k]->rxBase + idx * GW_CAN_STRIDE, NULL, cnt[k] * GW_CAN_STRIDE);
        }
        GcRun(ln, n, len, ret);
        for (k = 0; k < n; k++) {
            if (len[k] == 0U) {
                continue;
            }
            if (ret[k] != 0) {
                occ[k] = 0;                             /* 读失败不出队，下一拍重读 */
                continue;
            }
            for (i = 0; i < cnt[k] * GW_CAN_STRIDE; i++) {
                ln[k]->obj[done[k] * GW_CAN_STRIDE + i] = ln[k]->bus.rx[GW_MCP_HDR + i];
            }
            done[k] += cnt[k];
            more += (done[k] < occ[k]) ? 1U : 0U;
        }
    } while (more != 0U);
    for (k = 0; k < n; k++) {
        for (i = 0; i < occ[k]; i++) {
            if (GcObjDecode(ln[k]->obj + i * GW_CAN_STRIDE, &out[k][got[k]]) != 0U) {
                got[k]++;
            } else {
                ln[k]->rxDropped++;
            }
        }
        most = (occ[k] > most) ? occ[k] : most;
        ln[k]->rxFrames += got[k];
    }
    for (i = 0; i < most; i++) {
        for (k = 0; k < n; k++) {
            len[k] = (i < occ[k]) ? GcUinc(ln[k], GC_RX_FIFO, GC_UINC) : 0U;
        }
        GcRun(ln, n, len, ret);
    }
}

U32 GwCanReceive(struct GwCanLane *ln, struct GwCanFrame *out)
{
    U32 got;

    GwCanReceiveAll(&ln, 1U, &out, &got);
    return got;
}

/* 由 FIFO2 STA 与写头算待发帧数 */
static U32 GcTxPending(U32 sta, U32 head)
{
    if ((sta & GC_STA_EMPTY_FULL) != 0U) {
        return 0;
    }
    if ((sta & GC_STA_NE) == 0U) {
        return GW_CAN_TX_DEPTH;
    }
    return (head - GC_STA_CI(sta)) & (GW_CAN_TX_DEPTH - 1U);
}

/* 由 FIFO2 STA+UA 与再读的 STA（sta2）算本拍发几帧（组好对象）与写头相对偏移；0=不发。
 * 两读之间芯片只会发走帧，待发数只减不增：取大者，再读更大必是首读读错 */
static U32 GcTxPlan(struct GwCanLane *ln, const U8 *sa, U32 sta2, U32 *rel, U32 *first)
{
    U32 head;
    U32 pending;
    U32 again;
    U32 n;
    U32 k;

    *rel = GwLe32Get(sa + 4) - ln->txBase;
    if (*rel >= GW_CAN_TX_DEPTH * GW_CAN_STRIDE || (*rel % GW_CAN_STRIDE) != 0U) {
        ln->layoutErr++;
        return 0;
    }
    /* TX FIFO：FIFOCI=发送尾、UA=写头，同样相对 FIFO 基址 */
    head = *rel / GW_CAN_STRIDE;
    pending = GcTxPending(GwLe32Get(sa), head);
    again = GcTxPending(sta2, head);
    if (again > pending) {
        ln->txStaGlitch++;
        pending = again;
    }
    n = GW_CAN_TX_DEPTH - pending;
    if (n > GC_CHUNK) {
        n = GC_CHUNK;                                   /* 16 帧整 FIFO 一笔写不下 */
    }
    if (n > ln->txqCount) {
        n = ln->txqCount;
    }
    if (n < ln->txqCount) {
        ln->txFifoBusy++;
    }
    for (k = 0; k < n; k++) {
        const struct GwCanTxEntry *e = &ln->txq[(ln->txqHead + k) % GW_CAN_TXQ_DEPTH];
        GcObjMake(ln->obj + k * GW_CAN_STRIDE, e->id, e->data);
    }
    *first = GW_CAN_TX_DEPTH - head;
    if (*first > n) {
        *first = n;
    }
    return n;
}

void GwCanFlushAll(struct GwCanLane *const *ln, U32 n)
{
    U32 len[GW_CAN_LANES] = { 0 };
    U32 cnt[GW_CAN_LANES];
    U32 rel[GW_CAN_LANES];
    U32 first[GW_CAN_LANES];
    S32 ret[GW_CAN_LANES];
    U8 sa[GW_CAN_LANES][8];
    U32 most = 0;
    U32 i;
    U32 k;

    for (k = 0; k < n; k++) {
        cnt[k] = 0;
        rel[k] = 0;
        first[k] = 0;
        len[k] = (ln[k]->init == GW_CAN_OK && ln[k]->txqCount != 0U) ? GwMcpPrepare(&ln[k]->bus,
            GW_MCP_INS_READ, GW_MCP_FIFOSTA(GC_TX_FIFO), NULL, 8U) : 0U;
    }
    GcRun(ln, n, len, ret);
    for (k = 0; k < n; k++) {
        if (len[k] != 0U && ret[k] == 0) {
            for (i = 0; i < 8U; i++) {
                sa[k][i] = ln[k]->bus.rx[GW_MCP_HDR + i];
            }
        }
        len[k] = (len[k] != 0U && ret[k] == 0) ? GwMcpPrepare(&ln[k]->bus, GW_MCP_INS_READ,
            GW_MCP_FIFOSTA(GC_TX_FIFO), NULL, 4U) : 0U;
    }
    GcRun(ln, n, len, ret);                             /* 勘误 #6：再读一次 STA */
    for (k = 0; k < n; k++) {
        if (len[k] != 0U && ret[k] == 0) {
            cnt[k] = GcTxPlan(ln[k], sa[k], GwLe32Get(ln[k]->bus.rx + GW_MCP_HDR), &rel[k], &first[k]);
        }
        len[k] = (cnt[k] != 0U) ? GwMcpPrepare(&ln[k]->bus, GW_MCP_INS_WRITE,
            GW_MCP_RAM + ln[k]->txBase + rel[k], ln[k]->obj, first[k] * GW_CAN_STRIDE) : 0U;
    }
    GcRun(ln, n, len, ret);
    for (k = 0; k < n; k++) {
        if (len[k] != 0U && ret[k] != 0) {
            cnt[k] = 0;                                 /* 未 UINC 的对象芯片不会发，下一拍重写 */
        }
        len[k] = (cnt[k] > first[k]) ? GwMcpPrepare(&ln[k]->bus, GW_MCP_INS_WRITE,
            GW_MCP_RAM + ln[k]->txBase, ln[k]->obj + first[k] * GW_CAN_STRIDE,
            (cnt[k] - first[k]) * GW_CAN_STRIDE) : 0U;
    }
    GcRun(ln, n, len, ret);
    for (k = 0; k < n; k++) {
        if (len[k] != 0U && ret[k] != 0) {
            cnt[k] = first[k];
        }
        most = (cnt[k] > most) ? cnt[k] : most;
    }
    for (i = 0; i < most; i++) {
        for (k = 0; k < n; k++) {
            len[k] = (i < cnt[k]) ? GcUinc(ln[k], GC_TX_FIFO,
                (i + 1U == cnt[k]) ? GC_UINC_TXREQ : GC_UINC) : 0U;
        }
        GcRun(ln, n, len, ret);
    }
    for (k = 0; k < n; k++) {
        ln[k]->txqHead = (ln[k]->txqHead + cnt[k]) % GW_CAN_TXQ_DEPTH;
        ln[k]->txqCount -= cnt[k];
        ln[k]->txFrames += cnt[k];
    }
}

void GwCanFlush(struct GwCanLane *ln)
{
    GwCanFlushAll(&ln, 1U);
}

void GwCanDropQueued(struct GwCanLane *ln)
{
    ln->txqHead = (ln->txqHead + ln->txqCount) % GW_CAN_TXQ_DEPTH;
    ln->txqCount = 0;
}

/* FRESET 在配置模式下置位后不清零（实板：配置模式里轮询会等满 GC_MODE_TIMEOUT），
 * 回到正常模式才完成。故配置模式里只写 FRESET，回正常模式后再用本函数等它清零 */
static void GcFifoResetWait(struct GwSpiBus *b, U32 fifo)
{
    U64 t0 = GwSpiNow();

    while ((GcRd8(b, GW_MCP_FIFOCON(fifo) + 1U) & GC_FRESET) != 0U &&
        GwSpiNow() - t0 < GC_MODE_TIMEOUT) {
    }
}

S32 GwCanSetTdco(struct GwCanLane *ln, U32 tdco)
{
    struct GwSpiBus *b = &ln->bus;

    ln->tdco = tdco;
    if (ln->init != GW_CAN_OK) {
        return ln->init;
    }
    GwMcpWr8(b, GW_MCP_CICON + 3U, GC_CICON_B3_ABAT | GC_OPMOD_CONFIG);
    if (GcWaitOpmod(b, GC_OPMOD_CONFIG) != 0) {
        ln->init = GW_CAN_ERR_MODE;
        return ln->init;
    }
    GwMcpWr32(b, GW_MCP_TDC, GC_TDC_AUTO | ((tdco & 0x7FU) << 8));          /* 位时序只能在配置模式写 */
    GwMcpWr8(b, GW_MCP_FIFOCON(GC_RX_FIFO) + 1U, GC_FRESET);
    GwMcpWr8(b, GW_MCP_FIFOCON(GC_TX_FIFO) + 1U, GC_FRESET);
    GwCanDropQueued(ln);
    GwMcpWr8(b, GW_MCP_CICON + 3U, GC_OPMOD_NORMAL);
    if (GcWaitOpmod(b, GC_OPMOD_NORMAL) != 0) {
        ln->init = GW_CAN_ERR_MODE;
        return ln->init;
    }
    GcFifoResetWait(b, GC_RX_FIFO);
    GcFifoResetWait(b, GC_TX_FIFO);
    return ln->init;
}

U32 GwCanTrec(struct GwCanLane *ln)
{
    return (ln->init == GW_CAN_OK) ? GwMcpRd32(&ln->bus, GC_MCP_TREC) : 0U;
}

/* TX 恢复序列：ABAT + 配置模式 → 写 FIFO2 的 FRESET → 回正常模式、等复位完成；
 * 此后 FIFO2 头尾归零，对象从基址起写 */
void GwCanPanicSend(struct GwCanLane *ln, U32 firstId, U32 count, const U8 data[8])
{
    struct GwSpiBus *b = &ln->bus;
    U32 k;

    if (ln->init != GW_CAN_OK || count == 0U || count > GC_CHUNK) {
        return;
    }
    if (b->len != 0U) {
        (void)GwSpiFinish(b);                           /* 异常打断了在途事务：先收尾 */
    }
    /* 异常可能落在 GwSpiStart 改了 CR0 帧宽、还没记 w16 的时候：置无效值，下一笔必重写 */
    b->w16 = 2U;
    GwMcpWr8(b, GW_MCP_CICON + 3U, GC_CICON_B3_ABAT | GC_OPMOD_CONFIG);
    (void)GcWaitOpmod(b, GC_OPMOD_CONFIG);
    GwMcpWr8(b, GW_MCP_FIFOCON(GC_TX_FIFO) + 1U, GC_FRESET);
    GwMcpWr8(b, GW_MCP_CICON + 3U, GC_OPMOD_NORMAL);
    if (GcWaitOpmod(b, GC_OPMOD_NORMAL) != 0) {
        return;
    }
    GcFifoResetWait(b, GC_TX_FIFO);
    for (k = 0; k < count; k++) {
        GcObjMake(ln->obj + k * GW_CAN_STRIDE, firstId + k, data);
    }
    (void)GwMcpWrite(b, GW_MCP_RAM + ln->txBase, ln->obj, count * GW_CAN_STRIDE);
    for (k = 0; k < count; k++) {
        GwMcpWr8(b, GW_MCP_FIFOCON(GC_TX_FIFO) + 1U, (k + 1U == count) ? GC_UINC_TXREQ : GC_UINC);
    }
}
