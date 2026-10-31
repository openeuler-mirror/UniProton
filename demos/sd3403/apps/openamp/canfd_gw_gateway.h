/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2026. All rights reserved.
 * Description: OpenArm 双臂 CAN FD 网关——与控制任务同在 CPU3
 *
 * 每臂一个 Direct IPC 通道（direct_ipc.h，8 槽：7 关节 + 夹爪）。右臂通道 0x40100000
 * 默认走 SPI0，左臂 0x40102000 默认走 SPI2（可经 DirectCanConfig 互换），每路 SPI 接一片
 * MCP2518FD。
 * 一个网关任务（优先级 10，高于控制任务 20）每 tick 一拍：先两路批量下发上一拍排队的帧、
 * 两路同步收帧，再两臂依次单步（状态机→新帧入队→发布反馈），然后释放信号量唤醒控制任务；
 * 控制任务本拍算出的 target 下一拍取用入队、再下一拍拍首发出。
 * Linux 侧观测：统计页 GW_STATS_ADDR 的下列字。下列「右/左」按默认对应；按总线计的字
 * （FLAGS、INIT、LANE、STAGLITCH）换线后仍是 SPI0/SPI2。
 */
#ifndef CANFD_GW_GATEWAY_H
#define CANFD_GW_GATEWAY_H

#include "prt_typedef.h"

#define GW_STATS_ADDR   0x4010F000ULL
#define GW_W_MAGIC      0U
#define GW_STATS_MAGIC  0x54535747U     /* "GWST" */
#define GW_W_LAYOUT     1U
#define GW_STATS_LAYOUT 1U
#define GW_W_STAGE      2U              /* 1 CAN 初始化，2 运行 */
#define GW_W_FLAGS      3U              /* bit0/1 右/左 CAN 就绪，bit2/3 右/左跑 12.5MHz，
                                           bit4/5 右/左 SPI 16 位帧，
                                           bit8 已补发 DISABLE（异常钩子或 CPU_OFF），bit9 收到 CPU_OFF */
#define GW_W_INIT       4U              /* GwCanInit 返回值：右 | 左<<8（各取低 8 位，0=就绪） */
#define GW_W_BEATS      5U
#define GW_W_WORK       6U              /* 末拍网关耗时 ns（两臂单步 + 两路下发） */
#define GW_W_WORKMAX    7U
#define GW_W_WORKAVG    8U              /* 最近一个 1000 拍窗口的均值 */
#define GW_W_TOTAL      9U              /* 末拍「拍起点 → 控制任务本轮结束」ns：同核每拍总耗时 */
#define GW_W_TOTALMAX   10U
#define GW_W_TOTALAVG   11U
#define GW_W_INTVMAX    12U             /* 拍起点间隔最大 ns */
#define GW_W_LATE       13U             /* 间隔 >1.5ms 的拍数（丢 tick） */
#define GW_W_OVERRUN    14U             /* 下一拍开始时控制任务本轮仍未结束的次数 */
#define GW_W_HIST       15U             /* 15-22：总耗时 <400/500/600/700/800/900/1000/>=1000µs */
#define GW_W_TICKS      23U             /* PRT_TickGetCount 低 32 位（每 100 拍），与下一字同刻 */
#define GW_W_CNTPCT     24U             /* CNTPCT 低 32 位 */
#define GW_W_LANE(i)    (25U + 10U * (i))   /* 每路 10 字（0=右 SPI0，1=左 SPI2）：发、收、
                                               DLC 丢弃、RX 溢出、软件队列满、FIFO 空位不足拍、
                                               UA 越界、SPI 事务失败、RX 最大积压、CiTREC */
#define GW_W_EXC        45U             /* 异常钩子：0xE0000000 | excCause */
#define GW_W_ALGOITER   46U             /* 控制任务轮次 */
#define GW_W_ALGO       47U             /* 47-58：控制任务分段 ns，每段 3 字（末次、1000 次窗口均值、最大）：
                                           0 唤醒（网关释放信号量 → 控制任务起跑，只计真在等的轮）、
                                           1 镜像（两臂通道状态 + 收反馈）、2 算法（OpenArmControlStepDual）、
                                           3 下发（两臂 target 入环到本轮结束）；2、3 不计首次执行
                                           （那一轮建两臂 pinocchio 模型） */
#define GW_W_ALGOSTEPS  59U             /* 算法实际执行轮次（两臂都有新反馈才执行） */
#define GW_W_STAGLITCH  60U             /* 60-63：两次 STA 判出的读错（勘误 DS80000789E #6）：
                                           右 RX、左 RX、右 TX、左 TX */
#define GW_W_ARMBLOCK(i) (64U + (i))    /* 64-65：arm 请求其余条件齐备、仅因失能待确认
                                           （disable_pending）被挡的拍数（0=右，1=左） */
#define GW_W_DISABLEPEND 66U            /* 失能待确认位图：每臂 8 位（右在低 8 位），1=该电机还没报失能 */
#define GW_W_LAST       (GW_W_DISABLEPEND)

U32 GwGatewayInit(void);                /* 在 DirectIpcInit 之后调用 */
/* 网关任务的两段：两路 CAN 初始化、一拍 */
void GwGatewaySetup(void);
void GwGatewayBeat(void);
/* 控制任务每轮末尾调用，取代 PRT_TaskDelay：等网关本拍发布（最多 timeoutTicks） */
void GwGatewayAlgoWait(U32 timeoutTicks);
/* 控制任务本轮打点（GW_W_ALGO 分段）：起跑、镜像完、算法完各一次，GwGatewayAlgoWait 结算 */
#define GW_ALGO_MARK_WAKE   0U
#define GW_ALGO_MARK_MIRROR 1U
#define GW_ALGO_MARK_STEP   2U
void GwGatewayAlgoMark(U32 mark);
/* MICA CPU_OFF 时在 IPI 中断里调用：核下电前给两臂补发 DISABLE（纯 PIO） */
void GwGatewayPowerOffDisable(void);

#endif /* CANFD_GW_GATEWAY_H */
