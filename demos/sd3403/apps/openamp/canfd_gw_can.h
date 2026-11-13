/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2026. All rights reserved.
 * Description: OpenArm 网关的 MCP2518FD CAN FD 收发层
 *
 * 由网关任务按拍直接调用（单任务，不加锁）。
 * 初始化：
 *   RESET → 12.5MHz 资格（不过则退回 6.25MHz）→ 16 位帧资格（不过
 *   则 RESET 后留在 8 位帧）→ ECC 开 → 报文 RAM
 *   清零 → CiCON（复位值去 STEF：ISO CRC、允许 BRS、TXQ 1×8B 在前）→ TX FIFO2 16×8B
 *   TXPRI 1、TXAT 3 → RX FIFO1 32×8B 无时间戳 → 过滤器 0 全收进 FIFO1 → 1M/5M 位时序、
 *   TDC 自动（偏移取 lane 的 tdco，默认 6 = 数据段采样点 1+5 TQ）→ GPIO
 *   脚中断模式与 TX/RX/FIFO1 中断使能（本层按拍轮询、不接中断线）→ 正常 CAN FD 模式。
 * 每拍收发：
 *   收：一笔读 FIFO1 STA+UA，按「FIFOCI=写头、UA=读尾、都相对 FIFO 基址」算占用，
 *     再单读一次 STA、占用取两次的小者（勘误 DS80000789 #6：RX FIFO 的 FIFOCI 偶发读错，
 *     紧接着的再读是对的，勘误建议多读比对；读成偏大的写头会把已处理的旧帧或写了一半的
 *     帧当新帧），
 *     对象整批读（一笔 <=15 帧，跨 FIFO 尾拆笔），再逐帧 UINC。
 *     合法帧：DLC 折算长度 <=8B。
 *   发：GwCanSend 先进软件队列（32 帧，满时返回 -1，网关据此报 CAN 故障）；
 *     下一拍拍首 GwCanFlush 一笔读 FIFO2 STA+UA（同样再读
 *     一次 STA，待发数取两次的大者，免得把未发的对象覆盖掉），按空位整批
 *     写对象（单笔至多 15 帧：16 帧 258B 超出 PL022 FIFO）、逐帧 UINC、末帧带 TXREQ，
 *     发不完的留到下一拍。
 *   两路同步（GwCanFlushAll / GwCanReceiveAll）：上述每一步两路各起一笔、同时移位，
 *     再轮流收齐（GwSpiFinishAll），两路的步数不同时少的一路先歇。16 位帧下 UINC
 *     写 FIFOCON 字节 0、1 两字节（字节 0 原值回写，见 GcUinc），凑偶数长度；长于
 *     GW_SPI_W16_MAX 的笔（积压时的整批对象读写）仍走 8 位帧。
 * 不做软件 bus-off 恢复：芯片自身 bus-off 后自动恢复，发送堵塞由软件队列满上报。
 */
#ifndef CANFD_GW_CAN_H
#define CANFD_GW_CAN_H

#include "prt_typedef.h"
#include "canfd_gw_spi.h"

#define GW_CAN_RX_DEPTH     32U
#define GW_CAN_TX_DEPTH     16U
#define GW_CAN_STRIDE       16U     /* 8B 头 + 8B 载荷（PLSIZE_8，无时间戳） */
#define GW_CAN_TXQ_DEPTH    32U
#define GW_CAN_TDCO_DEFAULT 6U      /* TDC 偏移（SYSCLK 25ns）= 5Mbit/s 数据段采样点 */
#define GW_CAN_TDCO_MAX     63U     /* TDCO 为 7 位有符号字段，本层只用 0..63 */

enum {
    GW_CAN_OK = 0,
    GW_CAN_ERR_CLOCK = -1,          /* SPI 时钟门控或复位中，未碰 PL022 */
    GW_CAN_ERR_CHIP = -2,           /* RESET 后 CiCON 不是复位值 */
    GW_CAN_ERR_MODE = -3,           /* 进不了正常模式 */
    GW_CAN_ERR_LAYOUT = -4,         /* FIFO 基址与预期不符 */
    GW_CAN_ERR_SPI = -5,            /* 配置期间 SPI 事务超时 */
};

struct GwCanFrame {
    U32 id;
    U8 dlc;
    U8 ide;
    U8 rtr;
    U8 fdf;
    U8 data[8];
};

struct GwCanTxEntry {
    U32 id;
    U8 data[8];
};

struct GwCanLane {
    struct GwSpiBus bus;
    S32 init;                       /* GwCanInit 结果，GW_CAN_OK 即就绪 */
    U32 tdco;                       /* GwCanInit / GwCanSetTdco 写入的 TDC 偏移 */
    U32 rxBase;
    U32 txBase;
    U32 txqHead;
    U32 txqCount;
    struct GwCanTxEntry txq[GW_CAN_TXQ_DEPTH];
    U8 obj[GW_CAN_RX_DEPTH * GW_CAN_STRIDE];
    /* 统计（只增） */
    U32 txFrames;
    U32 rxFrames;
    U32 rxDropped;                  /* DLC 超出 8B 负载，已出队不交付 */
    U32 rxOverflow;
    U32 txQueueFull;
    U32 txFifoBusy;                 /* 本拍没发完（FIFO2 空位不够或超单笔 15 帧）的拍数 */
    U32 layoutErr;                  /* UA 越界或未对齐 */
    U32 maxRxOcc;
    U32 rxStaGlitch;                /* FIFO1 两次 STA：首读占用大于再读（必是读错） */
    U32 txStaGlitch;                /* FIFO2 两次 STA：再读待发数大于首读（必是读错） */
    U32 qualSfrErr;                 /* 12.5MHz 资格：SFR 读错 /150 */
    U32 qualRamErr;                 /* RAM 错字节，0xFFFFFFFF=未跑 */
    U32 qualWideErr;                /* 16 位帧资格：RAM 错字节 */
    U8 rxCon0;                      /* FIFO1 / FIFO2 的 FIFOCON 字节 0（初始化末读回） */
    U8 txCon0;
};

S32 GwCanInit(struct GwCanLane *ln);
/* 改 TDC 偏移（0..GW_CAN_TDCO_MAX）：ABAT + 配置模式 → 写 TDC、复位两个 FIFO、清软件队列
 * → 回正常模式。在途帧与未读帧全部丢弃，只在两臂都未使能时调用；进出模式失败时 init 置
 * GW_CAN_ERR_MODE */
S32 GwCanSetTdco(struct GwCanLane *ln, U32 tdco);
/* CAN FD + BRS、DLC 8 标准帧入软件队列；队满返回 -1 */
S32 GwCanSend(struct GwCanLane *ln, U32 id, const U8 data[8]);
/* 把 RX FIFO1 现有帧整批读出，合法帧写进 out（容量须 >= GW_CAN_RX_DEPTH），返回帧数 */
U32 GwCanReceive(struct GwCanLane *ln, struct GwCanFrame *out);
void GwCanFlush(struct GwCanLane *ln);
/* 丢弃软件队列里尚未写进 FIFO2 的帧（已在 FIFO2 的照发） */
void GwCanDropQueued(struct GwCanLane *ln);
/* 多路同步版（n <= GW_CAN_LANES）：got[k] 为 ln[k] 收到的帧数 */
#define GW_CAN_LANES        2U
void GwCanReceiveAll(struct GwCanLane *const *ln, U32 n, struct GwCanFrame *const *out, U32 *got);
void GwCanFlushAll(struct GwCanLane *const *ln, U32 n);
U32 GwCanTrec(struct GwCanLane *ln);    /* CiTREC 原值（TEC/REC 与错误状态位） */
/* 异常钩子与 CPU_OFF 用：不依赖中断与调度，先收尾被打断的在途事务，再中止并清空
 * TX FIFO2，发 count 帧（<= 15）data，ID 依次为 firstId 起 */
void GwCanPanicSend(struct GwCanLane *ln, U32 firstId, U32 count, const U8 data[8]);

#endif /* CANFD_GW_CAN_H */
