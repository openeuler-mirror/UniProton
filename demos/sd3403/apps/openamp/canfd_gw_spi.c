/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2026. All rights reserved.
 * Description: 网关 PL022 SPI 与 MCP2518FD 访问原语实现（见 canfd_gw_spi.h）
 */
#include "canfd_gw_spi.h"
#include "cpu_config.h"

extern U64 PRT_ClkGetCycleCount64(void);

/* 最长事务 256B@6.25MHz 线时 0.33ms，10ms 未收齐即判失败 */
#define GW_SPI_TIMEOUT_CYCLES   (SYS_COUNTER_FREQ / 100U)

U64 GwSpiNow(void)
{
    return PRT_ClkGetCycleCount64();
}

void GwSpiDelayUs(U32 us)
{
    U64 t0 = GwSpiNow();
    U64 n = (U64)us * SYS_COUNTER_FREQ / 1000000U;

    while (GwSpiNow() - t0 < n) {
    }
}

static void GwSpiFlushRx(struct GwSpiBus *bus)
{
    U32 i;

    for (i = 0; i < GW_SPI_FIFO_DEPTH * 2U &&
        (GW_REG_RD(bus->base, GW_SSP_SR) & GW_SR_RNE) != 0U; i++) {
        (void)GW_REG_RD(bus->base, GW_SSP_DR);
    }
}

/* SCLK = 100MHz / (CPSDVSR × (SCR+1))，取不超过 target 的最高档 */
U32 GwSpiSetClock(struct GwSpiBus *bus, U32 target)
{
    U32 best = 0;
    U32 bestC = 0;
    U32 bestS = 0;
    U32 c;
    U32 s;

    for (c = 2; c <= 254U; c += 2U) {
        for (s = 0; s <= 255U; s++) {
            U32 hz = GW_SPI_SRC_HZ / (c * (s + 1U));
            if (hz <= target) {
                if (hz > best) {
                    best = hz;
                    bestC = c;
                    bestS = s;
                }
                break;
            }
        }
    }
    if (bestC == 0U) {
        return 0;
    }
    GW_REG_WR(bus->base, GW_SSP_CR1, GW_CR1_ALT);
    GW_REG_WR(bus->base, GW_SSP_CR0, (GW_REG_RD(bus->base, GW_SSP_CR0) & ~0xFF00U) | (bestS << 8));
    GW_REG_WR(bus->base, GW_SSP_CPSR, bestC);
    bus->hz = best;
    return best;
}

/* 模式 0、8 位、主机、ALT 片选，不用 DMA */
U32 GwSpiInit(struct GwSpiBus *bus, U32 hz)
{
    U32 crg = GW_REG_RD(bus->crg, 0U);

    bus->hz = 0;
    bus->len = 0;
    bus->wide = 0;
    bus->w16 = 0;
    /* CRG 只读不写：Linux 启动后两路时钟均已开，
     * 门控或复位中访问 PL022 可能挂总线，宁可放弃本路 */
    if ((crg & GW_SPI_CRG_CLK_EN) == 0U || (crg & GW_SPI_CRG_RESET) != 0U) {
        return 0;
    }
    GW_REG_WR(bus->base, GW_SSP_CR1, GW_CR1_ALT);
    GW_REG_WR(bus->base, GW_SSP_CR0, 0x7U);
    GW_REG_WR(bus->base, GW_SSP_IMSC, 0U);
    GW_REG_WR(bus->base, GW_SSP_DMACR, 0U);
    GW_REG_WR(bus->base, GW_SSP_ICR, 3U);
    (void)GwSpiSetClock(bus, hz);
    GW_REG_WR(bus->base, GW_SSP_CR1, GW_CR1_ALT | GW_CR1_SSE);
    GwSpiFlushRx(bus);
    GW_REG_WR(bus->base, GW_SSP_CR1, GW_CR1_ALT);
    return bus->hz;
}

/* 收发前半：置 SSE（拉低片选、开始移位）后整笔盲写，len <= 256 不会写满 FIFO。
 * 16 位帧只用于偶数且 <= GW_SPI_W16_MAX 的笔。帧宽在 SSE=0 时改：上一笔已清 SSE */
void GwSpiStart(struct GwSpiBus *bus, U32 len)
{
    U32 w16 = (bus->wide != 0U && (len & 1U) == 0U && len <= GW_SPI_W16_MAX) ? 1U : 0U;
    U32 i;

    GwSpiFlushRx(bus);
    if (w16 != bus->w16) {
        GW_REG_WR(bus->base, GW_SSP_CR0, (GW_REG_RD(bus->base, GW_SSP_CR0) & ~0xFU) |
            ((w16 != 0U) ? 15U : 7U));
        bus->w16 = w16;
    }
    bus->len = len;
    bus->words = (w16 != 0U) ? len / 2U : len;
    bus->ri = 0;
    bus->t0 = GwSpiNow();
    GW_REG_WR(bus->base, GW_SSP_CR1, GW_CR1_ALT | GW_CR1_SSE);
    if (w16 != 0U) {
        for (i = 0; i < bus->words; i++) {
            GW_REG_WR(bus->base, GW_SSP_DR, ((U32)bus->tx[2U * i] << 8) | bus->tx[2U * i + 1U]);
        }
    } else {
        for (i = 0; i < len; i++) {
            GW_REG_WR(bus->base, GW_SSP_DR, bus->tx[i]);
        }
    }
}

/* 收一项（若有），返回 1 表示收到 */
static U32 GwSpiPull(struct GwSpiBus *bus)
{
    U32 v;

    if ((GW_REG_RD(bus->base, GW_SSP_SR) & GW_SR_RNE) == 0U) {
        return 0;
    }
    v = GW_REG_RD(bus->base, GW_SSP_DR);
    if (bus->w16 != 0U) {
        bus->rx[2U * bus->ri] = (U8)(v >> 8);
        bus->rx[2U * bus->ri + 1U] = (U8)v;
    } else {
        bus->rx[bus->ri] = (U8)v;
    }
    bus->ri++;
    return 1;
}

/* 收发后半：几路轮流按 RNE 收齐，再逐路等 !BSY、清 SSE。RX FIFO 同为 256 项，单笔不会溢出 */
void GwSpiFinishAll(struct GwSpiBus *const *bus, U32 n, S32 *ret)
{
    U32 left;
    U32 k;

    for (k = 0; k < n; k++) {
        ret[k] = 0;
    }
    do {
        left = 0;
        for (k = 0; k < n; k++) {
            if (ret[k] != 0 || bus[k]->ri >= bus[k]->words) {
                continue;
            }
            if (GwSpiPull(bus[k]) == 0U && GwSpiNow() - bus[k]->t0 > GW_SPI_TIMEOUT_CYCLES) {
                ret[k] = -1;
                continue;
            }
            left += (bus[k]->ri < bus[k]->words) ? 1U : 0U;
        }
    } while (left != 0U);
    for (k = 0; k < n; k++) {
        while ((GW_REG_RD(bus[k]->base, GW_SSP_SR) & GW_SR_BSY) != 0U) {
            if (GwSpiNow() - bus[k]->t0 > GW_SPI_TIMEOUT_CYCLES) {
                ret[k] = -1;
                break;
            }
        }
        GW_REG_WR(bus[k]->base, GW_SSP_CR1, GW_CR1_ALT);
        if (ret[k] != 0) {
            bus[k]->timeouts++;
        }
        bus[k]->len = 0;
    }
}

S32 GwSpiFinish(struct GwSpiBus *bus)
{
    S32 ret;

    GwSpiFinishAll(&bus, 1U, &ret);
    return ret;
}

S32 GwSpiXfer(struct GwSpiBus *bus, U32 len)
{
    GwSpiStart(bus, len);
    return GwSpiFinish(bus);
}

/* 在 bus->tx 组好一条 MCP2518FD 指令，返回总字节数；data 为 NULL 时补 0 */
U32 GwMcpPrepare(struct GwSpiBus *bus, U32 ins, U32 addr, const U8 *data, U32 len)
{
    U32 i;

    bus->tx[0] = (U8)((ins << 4) | ((addr >> 8) & 0xFU));
    bus->tx[1] = (U8)(addr & 0xFFU);
    for (i = 0; i < len; i++) {
        bus->tx[GW_MCP_HDR + i] = (data != NULL) ? data[i] : 0U;
    }
    return GW_MCP_HDR + len;
}

S32 GwMcpRead(struct GwSpiBus *bus, U32 addr, U8 *data, U32 len)
{
    U32 i;
    S32 ret;

    if (len > GW_MCP_MAX_DATA) {
        return -3;
    }
    ret = GwSpiXfer(bus, GwMcpPrepare(bus, GW_MCP_INS_READ, addr, NULL, len));
    for (i = 0; i < len; i++) {
        data[i] = bus->rx[GW_MCP_HDR + i];
    }
    return ret;
}

S32 GwMcpWrite(struct GwSpiBus *bus, U32 addr, const U8 *data, U32 len)
{
    if (len > GW_MCP_MAX_DATA) {
        return -3;
    }
    return GwSpiXfer(bus, GwMcpPrepare(bus, GW_MCP_INS_WRITE, addr, data, len));
}

U32 GwMcpRd32(struct GwSpiBus *bus, U32 addr)
{
    U8 d[4];

    (void)GwMcpRead(bus, addr, d, 4U);
    return GwLe32Get(d);
}

void GwMcpWr32(struct GwSpiBus *bus, U32 addr, U32 val)
{
    U8 d[4];

    GwLe32Put(d, val);
    (void)GwMcpWrite(bus, addr, d, 4U);
}

void GwMcpWr8(struct GwSpiBus *bus, U32 addr, U8 val)
{
    (void)GwMcpWrite(bus, addr, &val, 1U);
}

/* RESET 指令（0x00 0x00）后芯片重启振荡器，等 3ms 再访问 */
void GwMcpReset(struct GwSpiBus *bus)
{
    (void)GwSpiXfer(bus, GwMcpPrepare(bus, GW_MCP_INS_RESET, 0U, NULL, 0U));
    GwSpiDelayUs(3000U);
}
