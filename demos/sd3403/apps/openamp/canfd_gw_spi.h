/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2026. All rights reserved.
 * Description: 网关 PL022 SPI 与 MCP2518FD 访问原语
 *
 * 要点：
 *   - 纯 PIO，不用 DMAC：PL022 收发 FIFO 各 256B，常规一拍的事务（2B 命令头
 *     + 8 帧×16B = 130B）一次装得下；积压时按最多 15 个 CAN 对象（242B）分块。
 *   - 收发：先置 SSE（海思 ALT 模式下 SSE 即片选，FIFO 打空也不松），
 *     整笔盲写进 TX FIFO，按 RNE 收齐，等 !BSY 后清 SSE。
 *   - GwSpiStart/GwSpiFinish 拆开后可两条总线各起一笔再分别收，线时重叠；
 *     GwSpiFinishAll 轮流收几路，读 RX 时另一路仍在移位。
 *   - 16 位帧（bus->wide，资格通过后才置位）：偶数字节的事务一项 FIFO 装两字节、
 *     高字节先上线，MMIO 次数减半；奇数字节的事务仍用 8 位帧，按需改 CR0.DSS。
 *     8 位帧时每字节 MMIO（写 0.14 + 读 SR/DR 0.66µs）慢于线时 0.64µs，两路同时
 *     移位也快不了；16 位帧后 CPU 快于线时，两路才能真重叠。
 * MCP2518FD：RESET/READ/WRITE 指令，命令头 (ins<<4 | addr[11:8], addr[7:0])；
 * 报文 RAM 在 0x400 起 2KB，只能整字写。
 * 寄存器访问统一经 GW_REG_RD/GW_REG_WR（可在编译时另行定义，便于在主机上测试）。
 */
#ifndef CANFD_GW_SPI_H
#define CANFD_GW_SPI_H

#include "prt_typedef.h"

#ifndef GW_REG_RD
#define GW_REG_RD(base, off)        (*(volatile U32 *)((base) + (off)))
#define GW_REG_WR(base, off, val)   (*(volatile U32 *)((base) + (off)) = (val))
#endif

/* 地址：CRG 与 SPI 控制器都在 0x11000000 起的 2MB 内，须映射为 Device。 */
#define GW_SPI0_BASE        0x11070000UL    /* 默认右臂 */
#define GW_SPI2_BASE        0x11073000UL    /* 默认左臂 */
#define GW_SPI_CRG_BASE     0x11014000UL    /* SPI0 CRG @+0x480，SPI2 @+0x490 */
#define GW_SPI_CRG_CLK_EN   (1U << 4)
#define GW_SPI_CRG_RESET    (1U << 0)

/* PL022 寄存器 */
#define GW_SSP_CR0          0x000U
#define GW_SSP_CR1          0x004U
#define GW_SSP_DR           0x008U
#define GW_SSP_SR           0x00CU
#define GW_SSP_CPSR         0x010U
#define GW_SSP_IMSC         0x014U
#define GW_SSP_ICR          0x020U
#define GW_SSP_DMACR        0x024U
#define GW_CR1_SSE          (1U << 1)
#define GW_CR1_ALT          (1U << 6)       /* 海思扩展：SSE 驱动片选 */
#define GW_SR_RNE           (1U << 2)
#define GW_SR_BSY           (1U << 4)
#define GW_SPI_SRC_HZ       100000000U
#define GW_SPI_SAFE_HZ      6250000U        /* 引脚驱动档 0 时唯一无误码档 */
#define GW_SPI_FAST_HZ      12500000U       /* 需引脚驱动档 >=3 */
#define GW_SPI_FIFO_DEPTH   256U
#define GW_SPI_W16_MAX      130U            /* 16 位帧单笔上限（字节）= 16 位资格核对的长度；更长退回 8 位帧。
                                               16 位帧 <=144B 无误码、256B 每笔收不齐
                                               （FIFO 8/16 位都是 256 项，机理未明） */

/* MCP2518FD */
#define GW_MCP_INS_RESET    0x0U
#define GW_MCP_INS_WRITE    0x2U
#define GW_MCP_INS_READ     0x3U
#define GW_MCP_HDR          2U
#define GW_MCP_MAX_DATA     (GW_SPI_FIFO_DEPTH - GW_MCP_HDR)
#define GW_MCP_CICON        0x000U
#define GW_MCP_NBTCFG       0x004U
#define GW_MCP_DBTCFG       0x008U
#define GW_MCP_TDC          0x00CU
#define GW_MCP_FIFOCON(m)   (0x050U + 12U * (m))
#define GW_MCP_FIFOSTA(m)   (0x054U + 12U * (m))
#define GW_MCP_FIFOUA(m)    (0x058U + 12U * (m))
#define GW_MCP_FLTCON0      0x1D0U
#define GW_MCP_FLTOBJ0      0x1F0U
#define GW_MCP_MASK0        0x1F4U
#define GW_MCP_OSC          0xE00U
#define GW_MCP_DEVID        0xE14U
#define GW_MCP_RAM          0x400U
#define GW_MCP_RAM_SIZE     2048U
#define GW_MCP_CICON_RESET  0x04980760U     /* 复位后：配置模式、TXQEN=1、STEF=1 */

struct GwSpiBus {
    uintptr_t base;         /* PL022 寄存器基址 */
    uintptr_t crg;          /* 本路 SPI 的 CRG 时钟/复位寄存器 */
    U32 id;                 /* 0=SPI0，2=SPI2 */
    U32 hz;                 /* 当前 SCLK，0=未初始化 */
    U32 wide;               /* 允许 16 位帧 */
    U32 w16;                /* CR0 当前为 16 位帧 */
    U32 len;                /* GwSpiStart 起的在途字节数 */
    U32 words;              /* 在途 FIFO 项数（16 位帧为 len/2） */
    U32 ri;                 /* 已收项数 */
    U32 timeouts;           /* 收不齐、BSY 不落或 RX 溢出的事务数 */
    U64 t0;
    U8 tx[GW_SPI_FIFO_DEPTH];
    U8 rx[GW_SPI_FIFO_DEPTH];
};

/* MCP2518FD 寄存器与报文对象均为小端 */
static inline U32 GwLe32Get(const U8 *p)
{
    return (U32)p[0] | ((U32)p[1] << 8) | ((U32)p[2] << 16) | ((U32)p[3] << 24);
}

static inline void GwLe32Put(U8 *p, U32 v)
{
    p[0] = (U8)v;
    p[1] = (U8)(v >> 8);
    p[2] = (U8)(v >> 16);
    p[3] = (U8)(v >> 24);
}

U64 GwSpiNow(void);                          /* CNTPCT，SYS_COUNTER_FREQ 计数 */
void GwSpiDelayUs(U32 us);
U32 GwSpiInit(struct GwSpiBus *bus, U32 hz); /* 返回实际 SCLK；CRG 门控或复位中返回 0 且不碰 SPI */
U32 GwSpiSetClock(struct GwSpiBus *bus, U32 target);
void GwSpiStart(struct GwSpiBus *bus, U32 len);   /* 发 bus->tx[0..len)，len <= 256 */
S32 GwSpiFinish(struct GwSpiBus *bus);            /* 收进 bus->rx[0..len)，0=成功 */
/* 已 GwSpiStart 的 n 路轮流收齐，ret[k] 同 GwSpiFinish */
void GwSpiFinishAll(struct GwSpiBus *const *bus, U32 n, S32 *ret);
S32 GwSpiXfer(struct GwSpiBus *bus, U32 len);

U32 GwMcpPrepare(struct GwSpiBus *bus, U32 ins, U32 addr, const U8 *data, U32 len);
S32 GwMcpRead(struct GwSpiBus *bus, U32 addr, U8 *data, U32 len);
S32 GwMcpWrite(struct GwSpiBus *bus, U32 addr, const U8 *data, U32 len);
U32 GwMcpRd32(struct GwSpiBus *bus, U32 addr);
void GwMcpWr32(struct GwSpiBus *bus, U32 addr, U32 val);
void GwMcpWr8(struct GwSpiBus *bus, U32 addr, U8 val);
void GwMcpReset(struct GwSpiBus *bus);

#endif /* CANFD_GW_SPI_H */
