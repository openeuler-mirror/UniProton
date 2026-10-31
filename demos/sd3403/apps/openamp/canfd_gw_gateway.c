/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2026. All rights reserved.
 * Description: OpenArm 双臂 CAN FD 网关实现（见 canfd_gw_gateway.h）
 *
 * 每臂一个 GatewayInstance，GatewayStep 每拍走一次该臂的状态机：等通道就绪 → 等控制任务
 * 心跳稳定 → 发现电机 → DISARMED → arm 请求沿使能 → RUNNING，任一致命故障进 FAULT_SAFE
 * 并失能。收发经 canfd_gw_can.c 的软件队列，按拍批量进出 MCP2518FD。
 * 通道区按 Device 属性映射（非缓存），结构体整块进出改用字拷贝（__wrap_memcpy 逐字节），
 * CRC 在本地副本上算好再写入槽位。
 */
#include <math.h>
#include <string.h>
#include "canfd_gw_gateway.h"
#include "canfd_gw_can.h"
#include "openarm_can.h"
#include "direct_ipc.h"
#include "prt_task.h"
#include "prt_sem.h"
#include "prt_exc.h"
#include "prt_tick.h"
#include "cpu_config.h"

extern U32 PRT_Printf(const char *format, ...);

/* 共享内存位置（可在编译时另行定义，便于在主机上测试） */
#ifndef GW_MARK
#define GW_MARK                     ((volatile U32 *)(uintptr_t)GW_STATS_ADDR)
#define DIRECT_CHANNEL_RIGHT_PTR    ((struct DirectIpcChannel *)(uintptr_t)DIRECT_CHANNEL_RIGHT_ADDR)
#define DIRECT_CHANNEL_LEFT_PTR     ((struct DirectIpcChannel *)(uintptr_t)DIRECT_CHANNEL_LEFT_ADDR)
#endif

#define GW_TASK_PRIO            10U
#define GW_TASK_STACK           0x4000U
#define GW_PUBLISH_BEATS        100U
#define GW_TREC_BEATS           1000U
#define GW_LATE_NS              1500000U

/* ---- 网关常量与类型 ---- */
#define DIRECT_TARGET_TIMEOUT_NS 10000000ULL
#define DIRECT_FEEDBACK_TIMEOUT_NS 10000000ULL
#define DIRECT_CTRL_TIMEOUT_NS 100000000ULL
#define DIRECT_DISCOVERY_TIMEOUT_NS 2000000000ULL
#define DIRECT_ARM_TIMEOUT_NS 100000000ULL
#define DIRECT_ARM_GRACE_NS 1000000000ULL
#define DIRECT_CTRL_STABLE_HEARTBEATS 10U
#define DIRECT_FEEDBACK_STABLE_SAMPLES 3U
#define DIRECT_NUM_ARMS 2U
/* 夹爪（槽 7）：每 DIRECT_GRIP_POLL_BEATS 拍才一帧应答，新鲜窗放宽到 20ms
 * （连丢两帧仍在窗内）。指令由网关再钳一道，与算法侧参数无关：位置软限位
 * （0≈全闭、−1.1≈全开，两端各留 0.05 rad），弹簧力 kp×|qTarget−q| ≤ 0.5Nm，
 * 前馈 ≤ 0.1Nm */
#define DIRECT_GRIP_FEEDBACK_TIMEOUT_NS 20000000ULL
/* 失能未经回帧确认时补发 DISABLE 的间隔（拍） */
#define DIRECT_DISABLE_RETRY_BEATS 10U
#define GRIP_Q_MIN (-1.05F)
#define GRIP_Q_MAX (-0.05F)
#define GRIP_KP_MAX 10.0F
#define GRIP_KD_MAX 1.0F
#define GRIP_DQ_MAX 5.0F
#define GRIP_TAU_FF_MAX 0.1F
#define GRIP_SPRING_MAX 0.5F
#define GRIP_BIT (1U << DIRECT_GRIPPER_SLOT)
#define CANFD_OK 0
#define CANFD_ERR_QUEUE_FULL (-1)

typedef struct DirectIpcChannel DirectChannel;
typedef struct DirectRobotTargetMessage DirectTarget;
typedef struct DirectRobotFeedbackMessage DirectFeedback;

typedef enum {
    GATEWAY_BOOT = 0U, GATEWAY_WAIT_IPC, GATEWAY_WAIT_CTRL,
    GATEWAY_CAN_SELF_TEST, GATEWAY_WAIT_FEEDBACK, GATEWAY_DISARMED,
    GATEWAY_ARMED, GATEWAY_RUNNING, GATEWAY_FAULT_SAFE,
} GatewayState;

typedef struct {
    const char *name;
    struct GwCanLane *lane;
    DirectChannel *channel;
    OpenArmMotor motors[OPENARM_NUM_MOTORS];
    uint32_t generation;
    uint32_t last_target;
    uint64_t last_target_ns;
    uint32_t feedback_sequence;
    DirectFeedback feedback;
    uint8_t can_initialized;
    uint64_t joint_last_ns[DIRECT_MOTOR_COUNT];
    DirectTarget latest_target;
    uint8_t have_target;
    uint8_t motors_enabled;
    uint8_t enabled_set;            /* 本次使能沿发过 ENABLE 的电机 */
    uint8_t disable_pending;        /* 应已失能、回帧尚未报失能的电机 */
    uint32_t disable_countdown;
    uint32_t arm_blocked;           /* arm 请求其余条件齐备、仅因 disable_pending 被挡的累计拍数 */
    uint32_t rx_count;
    uint32_t tx_count;
    GatewayState gateway_state;
    uint8_t arm_edge_ready;
    uint8_t discovered_mask;
    uint8_t snapshot_mask;
    uint32_t active_joint_mask;
    uint32_t config_sequence;
    uint8_t ipc_ready;
    uint32_t refresh_countdown;
    uint32_t ctrl_heartbeat;
    uint64_t ctrl_last_ns;
    uint64_t discovery_start_ns;
    uint64_t armed_start_ns;
    uint32_t ctrl_stable_count;
    uint32_t feedback_stable_count;
    uint8_t ctrl_active;
    volatile struct DirectGripperState *grip;   /* 通道 + DIRECT_GRIP_OFFSET */
    uint32_t grip_countdown;
    /* 拍首两路同步收好的帧（GwGatewayBeat），本拍 DrainCan 取用 */
    uint8_t rx_pre;
    uint32_t rx_pre_n;
    struct GwCanFrame rx_pre_frames[GW_CAN_RX_DEPTH];
} GatewayInstance;

/* ---- 承载层 ---- */
static struct GwCanLane g_gwLane[DIRECT_NUM_ARMS] = {
    { .bus = { .base = GW_SPI0_BASE, .crg = GW_SPI_CRG_BASE + 0x480U, .id = 0U }, .init = GW_CAN_ERR_CLOCK,
      .tdco = GW_CAN_TDCO_DEFAULT },
    { .bus = { .base = GW_SPI2_BASE, .crg = GW_SPI_CRG_BASE + 0x490U, .id = 2U }, .init = GW_CAN_ERR_CLOCK,
      .tdco = GW_CAN_TDCO_DEFAULT },
};

static GatewayInstance g_gateway[DIRECT_NUM_ARMS] = {
    { .name = "R", .lane = &g_gwLane[0] },
    { .name = "L", .lane = &g_gwLane[1] },
};
static struct GwCanLane *const g_gwLanes[DIRECT_NUM_ARMS] = { &g_gwLane[0], &g_gwLane[1] };

static volatile struct DirectCanConfig *g_gwCanCfg;
static SemHandle g_gwSem;
static volatile U32 g_gwSemReady;
static volatile U64 g_gwAlgoEnd;
static volatile U32 g_gwAlgoIters;
static volatile U32 g_gwPanicDone;
static volatile U32 g_gwPanicReady;

static struct {
    U64 lastT0;
    U64 workSum;
    U64 totalSum;
    U32 workN;
    U32 totalN;
    U32 beats;
    U32 work;
    U32 workMax;
    U32 workAvg;
    U32 total;
    U32 totalMax;
    U32 totalAvg;
    U32 intvMax;
    U32 late;
    U32 overrun;
    U32 postedIters;
    U32 posted;
    U64 postT;                  /* 末次释放信号量的时刻 */
    U32 hist[8];
    U32 trec[DIRECT_NUM_ARMS];
} g_gwBeat;

/* 算法任务分段（GW_W_ALGO）：0 唤醒、1 镜像、2 算法、3 下发 */
#define GW_ALGO_SEGS 4U
static volatile U64 g_gwAlgoMarkT[GW_ALGO_MARK_STEP + 1U];
static volatile U64 g_gwAlgoWakePost;   /* 起跑时看到的 postT */
static struct {
    U64 prevEnd;
    U64 sum[GW_ALGO_SEGS];
    U32 n[GW_ALGO_SEGS];
    U32 last[GW_ALGO_SEGS];
    U32 avg[GW_ALGO_SEGS];
    U32 max[GW_ALGO_SEGS];
    U32 steps;
} g_gwAlgo;

static const U32 g_gwHistEdgeUs[7] = { 400U, 500U, 600U, 700U, 800U, 900U, 1000U };

static U64 GwNowNs(void)
{
    /* 24MHz 下 ×1000 到 2^64 约 24 年，不会溢出 */
    return GwSpiNow() * 1000ULL / (SYS_COUNTER_FREQ / 1000000ULL);
}

static U32 GwNs(U64 cycles)
{
    return (U32)(cycles * 1000ULL / (SYS_COUNTER_FREQ / 1000000ULL));
}

/* 通道区 Device 映射：按字搬，大小均为 4 的倍数（静态断言见 direct_ipc.h） */
static void GwCopyWords(void *dst, const void *src, U32 bytes)
{
    volatile U32 *d = (volatile U32 *)dst;
    const volatile U32 *s = (const volatile U32 *)src;
    U32 i;

    for (i = 0; i < bytes / 4U; i++) {
        d[i] = s[i];
    }
}

static void GwBarrier(void)
{
    OS_EMBED_ASM("DSB SY" ::: "memory");
}

/* ---- 网关逻辑 ---- */
static const struct DirectJointParams g_gwDefaultParams = DIRECT_DEFAULT_JOINT_PARAMS;

static uint8_t ConfigParametersValid(DirectChannel *channel)
{
    struct DirectJointParams params;
    GwCopyWords(&params, (const void *)&channel->gravityScale[0], sizeof(params));
    return (uint8_t)DirectJointParamsValid(&params);
}

/* CRC-32（反射，多项式 0xEDB88320）查表版：每拍两臂各校验一条 target、生成一条反馈，约 600 字节 */
static uint32_t g_gwCrcTable[256];
static uint8_t g_gwCrcReady;

static uint32_t Crc32(const void *data, uint32_t length)
{
    const uint8_t *bytes = (const uint8_t *)data;
    uint32_t crc, i, bit;
    if (g_gwCrcReady == 0U) {
        for (i = 0U; i < 256U; i++) {
            crc = i;
            for (bit = 0U; bit < 8U; bit++)
                crc = (crc >> 1U) ^ (0xEDB88320U & (uint32_t)-(int32_t)(crc & 1U));
            g_gwCrcTable[i] = crc;
        }
        g_gwCrcReady = 1U;
    }
    crc = 0xFFFFFFFFU;
    for (i = 0U; i < length; i++)
        crc = (crc >> 8U) ^ g_gwCrcTable[(crc ^ bytes[i]) & 0xFFU];
    return ~crc;
}

static void SetFault(GatewayInstance *gw, uint32_t fault)
{
    DirectChannel *channel = gw->channel;
    channel->gwActiveFaultBits |= fault;
    channel->activeFaultBits = channel->ctrlActiveFaultBits |
        channel->gwActiveFaultBits;
    channel->faultBits = channel->activeFaultBits;
    channel->latchedFaultBits |= fault;
}

static int8_t CanTransmit(GatewayInstance *gw, uint32_t id, const uint8_t data[8])
{
    if (GwCanSend(gw->lane, id, data) != 0)
        return CANFD_ERR_QUEUE_FULL;
    gw->tx_count++;
    return CANFD_OK;
}

/* 只发关节（每拍）；夹爪槽 7 由 PollGripper 每 DIRECT_GRIP_POLL_BEATS 拍发、使能沿另发 */
static void SendTarget(GatewayInstance *gw, const DirectTarget *target)
{
    uint32_t i; uint8_t data[8];
    for (i=0U; i<DIRECT_ROBOT_JOINT_COUNT; i++) {
        if ((gw->active_joint_mask & (1U << i)) == 0U) continue;
        OpenArmPackMit(&gw->motors[i],target->kp[i],target->kd[i],
            target->qTarget[i],target->dqTarget[i],target->tauFf[i],data);
        if (CanTransmit(gw,gw->motors[i].send_id,data) != CANFD_OK)
            SetFault(gw,DIRECT_FAULT_CAN);
    }
}

static void SendRefresh(GatewayInstance *gw)
{
    uint32_t i;
    uint8_t data[8];
    for (i = 0U; i < DIRECT_ROBOT_JOINT_COUNT; i++) {
        if ((gw->active_joint_mask & (1U << i)) == 0U) continue;
        OpenArmPackRefresh(&gw->motors[i],data);
        if (CanTransmit(gw,OPENARM_CANID_BROADCAST,data) != CANFD_OK)
            SetFault(gw,DIRECT_FAULT_CAN);
    }
}

static void SendMotorCommand(GatewayInstance *gw, uint8_t command, uint8_t joint_mask)
{
    uint32_t i;
    uint8_t data[8];
    OpenArmPackCommand(command,data);
    for (i = 0U; i < DIRECT_MOTOR_COUNT; i++) {
        if ((joint_mask & (1U << i)) == 0U) continue;
        if (CanTransmit(gw,gw->motors[i].send_id,data) != CANFD_OK)
            SetFault(gw,DIRECT_FAULT_CAN);
    }
}

static float GripClamp(float x, float lo, float hi)
{
    return x < lo ? lo : (x > hi ? hi : x);
}

/* 夹爪 MIT 帧：target 槽 7 先过网关钳位。弹簧窗以最近一帧实测为中心，
 * 越过软限位的实测也只按 ≤ GRIP_SPRING_MAX 往回拉 */
static void SendGripper(GatewayInstance *gw, const DirectTarget *target)
{
    OpenArmMotor *m = &gw->motors[OPENARM_GRIPPER_IDX];
    const uint32_t g = DIRECT_GRIPPER_SLOT;
    float kp = GripClamp(target->kp[g], 0.0F, GRIP_KP_MAX);
    float q = GripClamp(target->qTarget[g], GRIP_Q_MIN, GRIP_Q_MAX);
    uint8_t data[8];
    if (kp > 0.0F)
        q = GripClamp(q, (float)m->q - GRIP_SPRING_MAX / kp, (float)m->q + GRIP_SPRING_MAX / kp);
    OpenArmPackMit(m,kp,GripClamp(target->kd[g],0.0F,GRIP_KD_MAX),q,
        GripClamp(target->dqTarget[g],-GRIP_DQ_MAX,GRIP_DQ_MAX),
        GripClamp(target->tauFf[g],-GRIP_TAU_FF_MAX,GRIP_TAU_FF_MAX),data);
    if (CanTransmit(gw,m->send_id,data) != CANFD_OK)
        SetFault(gw,DIRECT_FAULT_CAN);
    else
        gw->grip->polls++;
}

/* 夹爪每 DIRECT_GRIP_POLL_BEATS 拍一帧：已使能且在掩码内发 MIT（同关节，入队失败
 * 判 CAN 故障）；否则发回读请求，电机只回状态、不使能——入队失败不置故障，队满时
 * 关节帧会判 */
static void PollGripper(GatewayInstance *gw)
{
    uint8_t data[8];
    if (gw->grip_countdown != 0U) {
        gw->grip_countdown--;
        return;
    }
    gw->grip_countdown=DIRECT_GRIP_POLL_BEATS-1U;
    if (gw->motors_enabled != 0U && (gw->active_joint_mask & GRIP_BIT) != 0U) {
        SendGripper(gw,&gw->latest_target);
        return;
    }
    OpenArmPackRefresh(&gw->motors[OPENARM_GRIPPER_IDX],data);
    if (CanTransmit(gw,OPENARM_CANID_BROADCAST,data) == CANFD_OK)
        gw->grip->polls++;
}

/* 遥测块（direct_ipc.h DirectGripperState）：DrainCan 已按槽 7 解码过 */
static void PublishGripper(GatewayInstance *gw, const uint8_t data[8])
{
    volatile struct DirectGripperState *g = gw->grip;
    OpenArmMotor *m = &gw->motors[OPENARM_GRIPPER_IDX];
    g->seq++;
    g->replies++;
    g->timestampUs=(uint32_t)(GwNowNs()/1000ULL);
    g->q=(float)m->q;
    g->dq=(float)m->dq;
    g->tau=(float)m->tau;
    g->temps=(uint32_t)(uint8_t)m->t_mos | ((uint32_t)(uint8_t)m->t_rotor << 8);
    g->raw[0]=(uint32_t)data[0] | ((uint32_t)data[1] << 8) | ((uint32_t)data[2] << 16) |
        ((uint32_t)data[3] << 24);
    g->raw[1]=(uint32_t)data[4] | ((uint32_t)data[5] << 8) | ((uint32_t)data[6] << 16) |
        ((uint32_t)data[7] << 24);
    g->seq++;
}

/* 失能以电机回帧报失能为准（总线断开时软件队列可能已满，唯一一次 DISABLE
 * 会被丢；电机 TIMEOUT=0 不会自行失能，总线恢复后按队列里的旧 MIT 帧继续
 * 保持）：先丢掉软件队列里未下发的旧帧，再发 DISABLE，没报失能的
 * 电机由 ServiceDisable 补发 */
static void DisableMotors(GatewayInstance *gw)
{
    if (gw->motors_enabled != 0U) {
        GwCanDropQueued(gw->lane);
        SendMotorCommand(gw,OPENARM_CMD_DISABLE,DIRECT_ALL_JOINTS_MASK);
        gw->disable_pending |= gw->enabled_set;
        gw->disable_countdown = DIRECT_DISABLE_RETRY_BEATS;
    }
    gw->motors_enabled = 0U;
}

/* 每 DIRECT_DISABLE_RETRY_BEATS 拍给仍未报失能的电机补发 DISABLE；软件队列
 * 非空（总线不通时上一批还没写进 FIFO2）就不再加，不顶满队列 */
static void ServiceDisable(GatewayInstance *gw)
{
    uint32_t i;
    uint8_t data[8];
    if (gw->disable_pending == 0U)
        return;
    if (gw->disable_countdown != 0U) {
        gw->disable_countdown--;
        return;
    }
    if (gw->lane->txqCount != 0U)
        return;
    gw->disable_countdown = DIRECT_DISABLE_RETRY_BEATS;
    OpenArmPackCommand(OPENARM_CMD_DISABLE,data);
    for (i = 0U; i < DIRECT_MOTOR_COUNT; i++)
        if ((gw->disable_pending & (1U << i)) != 0U)
            (void)CanTransmit(gw,gw->motors[i].send_id,data);
}

static void DrainCan(GatewayInstance *gw)
{
    static struct GwCanFrame own[GW_CAN_RX_DEPTH];
    const struct GwCanFrame *frames = gw->rx_pre_frames;
    DirectFeedback *feedback = &gw->feedback;
    uint32_t n = gw->rx_pre_n;
    uint32_t k;
    if (gw->rx_pre != 0U) {
        gw->rx_pre = 0U;
    } else {
        frames = own;
        n = GwCanReceive(gw->lane, own);
    }
    for (k = 0U; k < n; k++) {
        const struct GwCanFrame *message = &frames[k];
        uint32_t i;
        if (message->dlc < 8U) continue;    /* 收层已滤掉 DLC>8 */
        for (i=0U;i<DIRECT_MOTOR_COUNT;i++)    /* 槽 7 = 夹爪 0x18，同关节进反馈 */
            if (gw->motors[i].recv_id == message->id) break;
        if (i == DIRECT_MOTOR_COUNT) continue;
        if (!OpenArmParseState(&gw->motors[i],message->data)) continue;
        /* 未使能时电机应报失能；报使能的（失能帧丢了、或上次会话留下的）补发 */
        if (gw->motors_enabled == 0U) {
            if (gw->motors[i].state == OPENARM_STATE_ENABLED)
                gw->disable_pending |= (uint8_t)(1U << i);
            else
                gw->disable_pending &= (uint8_t)~(1U << i);
        }
        if (i == DIRECT_GRIPPER_SLOT)
            PublishGripper(gw,message->data);
        feedback->q[i]=(float)gw->motors[i].q;
        feedback->dq[i]=(float)gw->motors[i].dq;
        feedback->tau[i]=(float)gw->motors[i].tau;
        feedback->tMos[i]=(uint8_t)gw->motors[i].t_mos;
        feedback->tRotor[i]=(uint8_t)gw->motors[i].t_rotor;
        feedback->onlineMask|=(uint8_t)(1U<<i);
        feedback->validMask|=(uint8_t)(1U<<i);
        gw->discovered_mask|=(uint8_t)(1U<<i);
        gw->snapshot_mask|=(uint8_t)(1U<<i);
        gw->joint_last_ns[i]=GwNowNs();
        gw->rx_count++;
    }
}

static int PopNewestTarget(GatewayInstance *gw, DirectTarget *target)
{
    DirectChannel *channel = gw->channel;
    int found=0; uint32_t current=channel->targetConsumer;
    while (current != channel->targetProducer) {
        DirectTarget copy; uint32_t crc;
        uint32_t i; uint8_t invalid=0U;
        GwCopyWords(&copy,&channel->target[current],sizeof(copy)); crc=copy.crc32;
        channel->targetConsumer=(current+1U)%DIRECT_RING_CAP;
        copy.crc32=0U;
        if (copy.magic != DIRECT_PROTOCOL_MAGIC || copy.version != DIRECT_PROTOCOL_VERSION ||
            copy.type != DIRECT_MSG_JOINT_TARGET || crc != Crc32(&copy,sizeof(copy))) {
            SetFault(gw,DIRECT_FAULT_BAD_MESSAGE); current=channel->targetConsumer; continue;
        }
        if (copy.generation != gw->generation) {    /* 上一会话入环的，丢弃 */
            current=channel->targetConsumer; continue;
        }
        for (i=0U; i<DIRECT_MOTOR_COUNT; i++) {
            if (!isfinite(copy.qTarget[i]) || !isfinite(copy.dqTarget[i]) ||
                !isfinite(copy.tauFf[i]) || !isfinite(copy.kp[i]) ||
                !isfinite(copy.kd[i]) || copy.kp[i] < 0.0F ||
                copy.kp[i] > (float)OPENARM_KP_MAX || copy.kd[i] < 0.0F ||
                copy.kd[i] > (float)OPENARM_KD_MAX ||
                copy.qTarget[i] < (float)-gw->motors[i].p_max ||
                copy.qTarget[i] > (float)gw->motors[i].p_max ||
                copy.dqTarget[i] < (float)-gw->motors[i].v_max ||
                copy.dqTarget[i] > (float)gw->motors[i].v_max ||
                copy.tauFf[i] < (float)-gw->motors[i].t_max ||
                copy.tauFf[i] > (float)gw->motors[i].t_max) {
                SetFault(gw,DIRECT_FAULT_BAD_MESSAGE);
                invalid=1U;
                break;
            }
        }
        if (invalid != 0U) { current=channel->targetConsumer; continue; }
        if ((copy.flags & ~DIRECT_TARGET_FLAG_ARM_REQUEST) != 0U) {
            SetFault(gw,DIRECT_FAULT_BAD_MESSAGE); current=channel->targetConsumer; continue;
        }
        if ((int32_t)(copy.sequence - gw->last_target) <= 0 && gw->last_target != 0U)
            SetFault(gw,DIRECT_FAULT_SEQUENCE);
        *target=copy; found=1; current=channel->targetConsumer;
    }
    return found;
}

static void PublishFeedback(GatewayInstance *gw, uint16_t state, uint32_t accepted)
{
    DirectChannel *channel = gw->channel;
    uint32_t current=channel->statusProducer, next=(current+1U)%DIRECT_RING_CAP;
    DirectFeedback *slot;
    if (next == channel->statusConsumer) {
        channel->statusOverflowCount++;
        SetFault(gw,DIRECT_FAULT_STATUS_OVERFLOW);
        return;
    }
    slot=&channel->status[current];
    gw->feedback.magic=DIRECT_PROTOCOL_MAGIC; gw->feedback.version=DIRECT_PROTOCOL_VERSION;
    gw->feedback.state=state; gw->feedback.sampleSequence=++gw->feedback_sequence;
    gw->feedback.acceptedSequence=accepted; gw->feedback.generation=gw->generation;
    gw->feedback.faultBits=channel->faultBits;
    gw->feedback.timestampUs=(uint32_t)(GwNowNs()/1000ULL);
    gw->feedback.crc32=0U; gw->feedback.crc32=Crc32(&gw->feedback,sizeof(gw->feedback));
    GwCopyWords(slot,&gw->feedback,sizeof(*slot));
    GwBarrier(); channel->statusProducer=next;
    channel->gwTxCount++;
    channel->lastStatusSequence=gw->feedback.sampleSequence;
}

/* 对外报 SAFE：没有电机在使能，也没有失能待确认 */
static uint32_t GwSafeBit(const GatewayInstance *gw)
{
    return (gw->motors_enabled == 0U && gw->disable_pending == 0U) ? DIRECT_STATE_SAFE : 0U;
}

static void ResetSession(GatewayInstance *gw, uint32_t generation)
{
    DirectChannel *channel = gw->channel;
    DisableMotors(gw);
    gw->generation=generation; gw->last_target=0U; gw->last_target_ns=0U;
    gw->feedback_sequence=0U; gw->have_target=0U;
    gw->gateway_state=GATEWAY_WAIT_CTRL;
    gw->arm_edge_ready=0U;
    gw->discovered_mask=0U;
    gw->snapshot_mask=0U;
    if (channel->requestedJointMask != 0U &&
        (channel->requestedJointMask & ~DIRECT_ALL_JOINTS_MASK) == 0U)
        gw->active_joint_mask=channel->requestedJointMask;
    else
        gw->active_joint_mask=DIRECT_DEFAULT_JOINT_MASK;
    gw->config_sequence=channel->configSequence;
    channel->effectiveJointMask=gw->active_joint_mask;
    channel->configAckSequence=gw->config_sequence;
    GwCopyWords((void *)&channel->gravityScale[0],&g_gwDefaultParams,sizeof(g_gwDefaultParams));
    (void)memset(&gw->latest_target,0,sizeof(gw->latest_target));
    (void)memset(&gw->feedback,0,sizeof(gw->feedback));
    (void)memset(gw->joint_last_ns,0,sizeof(gw->joint_last_ns));
    channel->gwSeenGeneration=gw->generation;
    channel->gwState=GwSafeBit(gw);
}

/* 硬件初始化已在网关任务启动时完成（GwCanInit，一次约 0.1s，放在拍循环里会让
 * 另一臂停拍）；这里只登记电机表、置 can_initialized */
static int8_t OpenArmCanInit(GatewayInstance *gw)
{
    OpenArmMotorsInit(gw->motors);
    if (gw->lane->init != GW_CAN_OK) {
        return (int8_t)gw->lane->init;
    }
    gw->can_initialized=1U;
    return CANFD_OK;
}

static void GatewayStep(GatewayInstance *gw)
{
    DirectTarget target;
    uint16_t state; uint32_t i; uint64_t now=GwNowNs();
    uint8_t fresh_mask=0U;
    uint8_t arm_request;
    if (gw->ipc_ready == 0U) {
        if (gw->channel->magic != DIRECT_CHANNEL_MAGIC ||
            gw->channel->version != DIRECT_CHANNEL_VERSION ||
            gw->channel->headerSize != DIRECT_CHANNEL_HEADER_SIZE) {
            return;
        }
        gw->channel->gwBootCount++;
        ResetSession(gw,gw->channel->generation);
        gw->ctrl_heartbeat=gw->channel->ctrlHeartbeat;
        gw->discovery_start_ns=GwNowNs();
        gw->ipc_ready=1U;
        now=GwNowNs();
    }
    gw->channel->gwHeartbeat++;
    if (gw->generation != gw->channel->generation) {
        DisableMotors(gw);
        ResetSession(gw,gw->channel->generation);
        gw->ctrl_active=0U;
        gw->ctrl_stable_count=0U;
        gw->feedback_stable_count=0U;
        gw->discovery_start_ns=now;
        gw->ctrl_heartbeat=gw->channel->ctrlHeartbeat;
    }
    if ((gw->gateway_state == GATEWAY_DISARMED ||
        gw->gateway_state == GATEWAY_FAULT_SAFE) &&
        gw->channel->configSequence != gw->config_sequence) {
        uint32_t requested=gw->channel->requestedJointMask;
        if (requested != 0U && (requested & ~DIRECT_ALL_JOINTS_MASK) == 0U &&
            ConfigParametersValid(gw->channel) != 0U) {
            uint32_t removed=gw->active_joint_mask & ~requested;
            if (gw->can_initialized != 0U && removed != 0U)
                SendMotorCommand(gw,OPENARM_CMD_DISABLE,(uint8_t)removed);
            gw->active_joint_mask=requested;
            gw->channel->effectiveJointMask=requested;
            gw->config_sequence=gw->channel->configSequence;
            gw->channel->configAckSequence=gw->config_sequence;
            gw->snapshot_mask=0U;
        }
    }
    if (gw->channel->ctrlHeartbeat != gw->ctrl_heartbeat &&
        gw->channel->ctrlSeenGeneration == gw->generation) {
        gw->ctrl_active=1U;
        gw->ctrl_heartbeat=gw->channel->ctrlHeartbeat;
        gw->ctrl_last_ns=now;
        if (gw->ctrl_stable_count < DIRECT_CTRL_STABLE_HEARTBEATS)
            gw->ctrl_stable_count++;
    }
    if (gw->ctrl_active == 0U && now-gw->discovery_start_ns > DIRECT_DISCOVERY_TIMEOUT_NS) {
        SetFault(gw,DIRECT_FAULT_FEEDBACK_TIMEOUT);
        gw->gateway_state=GATEWAY_FAULT_SAFE;
    }
    if (gw->ctrl_active != 0U && now-gw->ctrl_last_ns > DIRECT_CTRL_TIMEOUT_NS) {
        SetFault(gw,DIRECT_FAULT_FEEDBACK_TIMEOUT);
        gw->gateway_state=GATEWAY_FAULT_SAFE;
    }
    if (gw->gateway_state == GATEWAY_WAIT_CTRL &&
        gw->ctrl_stable_count >= DIRECT_CTRL_STABLE_HEARTBEATS) {
        gw->gateway_state=GATEWAY_CAN_SELF_TEST;
        if (gw->can_initialized == 0U && OpenArmCanInit(gw) != CANFD_OK) {
            SetFault(gw,DIRECT_FAULT_CAN);
            gw->gateway_state=GATEWAY_FAULT_SAFE;
        } else {
            gw->gateway_state=GATEWAY_WAIT_FEEDBACK;
            gw->discovery_start_ns=now;
        }
    }
    if (gw->can_initialized != 0U)
        DrainCan(gw);
    /* DrainCan timestamps each response. Use a post-drain time base. */
    now=GwNowNs();
    if (PopNewestTarget(gw,&target)) {
        gw->latest_target=target; gw->have_target=1U;
        gw->last_target=target.sequence; gw->last_target_ns=now;
    }
    gw->feedback.onlineMask=gw->discovered_mask & gw->active_joint_mask;
    for (i=0U; i<DIRECT_MOTOR_COUNT; i++)
        if (now-gw->joint_last_ns[i] <= (i == DIRECT_GRIPPER_SLOT ?
            DIRECT_GRIP_FEEDBACK_TIMEOUT_NS : DIRECT_FEEDBACK_TIMEOUT_NS))
            fresh_mask|=(uint8_t)(1U<<i);
    if ((gw->gateway_state == GATEWAY_ARMED || gw->gateway_state == GATEWAY_RUNNING) &&
        now-gw->armed_start_ns > DIRECT_ARM_GRACE_NS)
        gw->feedback.onlineMask=fresh_mask & gw->active_joint_mask;
    gw->feedback.validMask=gw->feedback.onlineMask;
    if (gw->can_initialized != 0U && gw->motors_enabled == 0U &&
        gw->gateway_state != GATEWAY_FAULT_SAFE) {
        if (gw->refresh_countdown == 0U) {
            SendRefresh(gw);
            /* 未使能时每 21 拍一轮回读：发现电机、跟踪位姿，只占少量总线 */
            gw->refresh_countdown=20U;
        } else {
            gw->refresh_countdown--;
        }
    }
    if (gw->can_initialized != 0U &&
        gw->feedback.onlineMask == gw->active_joint_mask) {
        if (gw->feedback_stable_count < DIRECT_FEEDBACK_STABLE_SAMPLES)
            gw->feedback_stable_count++;
        if (gw->gateway_state == GATEWAY_WAIT_FEEDBACK &&
            gw->feedback_stable_count >= DIRECT_FEEDBACK_STABLE_SAMPLES)
            gw->gateway_state=GATEWAY_DISARMED;
    } else {
        gw->feedback_stable_count=0U;
        if (gw->motors_enabled != 0U)
            SetFault(gw,DIRECT_FAULT_OFFLINE);
    }
    if (gw->gateway_state == GATEWAY_WAIT_FEEDBACK &&
        now-gw->discovery_start_ns > DIRECT_DISCOVERY_TIMEOUT_NS)
        SetFault(gw,DIRECT_FAULT_FEEDBACK_TIMEOUT);
    if (gw->gateway_state == GATEWAY_ARMED &&
        now-gw->armed_start_ns > DIRECT_ARM_TIMEOUT_NS)
        SetFault(gw,DIRECT_FAULT_FEEDBACK_TIMEOUT);
    if ((gw->gateway_state == GATEWAY_ARMED || gw->gateway_state == GATEWAY_RUNNING) &&
        gw->have_target != 0U &&
        now-gw->last_target_ns > DIRECT_TARGET_TIMEOUT_NS)
        SetFault(gw,DIRECT_FAULT_TARGET_TIMEOUT);
    /* arm 请求以新鲜 target 为准：控制任务停发（故障）后留下的旧请求不算 */
    arm_request=(uint8_t)((gw->latest_target.flags & DIRECT_TARGET_FLAG_ARM_REQUEST) != 0U &&
        gw->have_target != 0U && now-gw->last_target_ns <= DIRECT_TARGET_TIMEOUT_NS);
    if (gw->channel->activeFaultBits & DIRECT_FAULT_FATAL_MASK) {
        gw->gateway_state=GATEWAY_FAULT_SAFE;
        DisableMotors(gw);
    } else if (gw->gateway_state == GATEWAY_DISARMED &&
        (gw->latest_target.flags & DIRECT_TARGET_FLAG_ARM_REQUEST) == 0U) {
        gw->arm_edge_ready=1U;
    } else if (gw->gateway_state == GATEWAY_DISARMED && gw->arm_edge_ready != 0U &&
        arm_request != 0U &&
        (fresh_mask & gw->active_joint_mask) == gw->active_joint_mask &&
        gw->disable_pending == 0U) {
        /* 先排一帧当前 target 再发 ENABLE：电机一使能就按这一帧保持，不用它存着的旧指令 */
        SendTarget(gw,&gw->latest_target);
        if ((gw->active_joint_mask & GRIP_BIT) != 0U)
            SendGripper(gw,&gw->latest_target);
        if ((gw->channel->activeFaultBits & DIRECT_FAULT_FATAL_MASK) != 0U)
            return;
        gw->snapshot_mask=0U;
        SendMotorCommand(gw,OPENARM_CMD_ENABLE,(uint8_t)gw->active_joint_mask);
        gw->enabled_set=(uint8_t)gw->active_joint_mask;
        gw->motors_enabled=1U;
        gw->armed_start_ns=now;
        gw->gateway_state=GATEWAY_ARMED;
    }
    /* arm 被「失能待确认」挡住时不报故障：累计拍数写进统计页（GW_W_ARMBLOCK） */
    if (gw->gateway_state == GATEWAY_DISARMED && gw->arm_edge_ready != 0U &&
        arm_request != 0U &&
        (fresh_mask & gw->active_joint_mask) == gw->active_joint_mask &&
        gw->disable_pending != 0U)
        gw->arm_blocked++;
    if (gw->gateway_state == GATEWAY_ARMED && gw->motors_enabled != 0U &&
        (gw->snapshot_mask & gw->active_joint_mask) == gw->active_joint_mask)
        gw->gateway_state=GATEWAY_RUNNING;
    if ((gw->gateway_state == GATEWAY_ARMED || gw->gateway_state == GATEWAY_RUNNING) &&
        (gw->latest_target.flags & DIRECT_TARGET_FLAG_ARM_REQUEST) == 0U) {
        DisableMotors(gw);
        gw->gateway_state=GATEWAY_DISARMED;
        gw->arm_edge_ready=1U;
    }
    if ((gw->gateway_state == GATEWAY_ARMED || gw->gateway_state == GATEWAY_RUNNING) &&
        gw->motors_enabled != 0U)
        SendTarget(gw,&gw->latest_target);
    if (gw->gateway_state == GATEWAY_FAULT_SAFE)
        DisableMotors(gw);
    if (gw->can_initialized != 0U)
        ServiceDisable(gw);
    /* 排在本拍关节帧之后：TX FIFO 按序发，0x7FF / 0x18 仲裁又输给关节回帧，
     * 夹爪收发整体落在关节之后，不推迟关节指令与回帧 */
    if (gw->can_initialized != 0U && gw->gateway_state != GATEWAY_FAULT_SAFE)
        PollGripper(gw);
    if (gw->gateway_state == GATEWAY_WAIT_FEEDBACK)
        gw->channel->gwState=DIRECT_STATE_DISCOVERED|GwSafeBit(gw);
    else if (gw->gateway_state == GATEWAY_DISARMED)
        gw->channel->gwState=DIRECT_STATE_GW_READY|DIRECT_STATE_DISCOVERED|
            DIRECT_STATE_DISARMED|GwSafeBit(gw);
    else if (gw->gateway_state == GATEWAY_ARMED)
        gw->channel->gwState=DIRECT_STATE_GW_READY|DIRECT_STATE_DISCOVERED|
            DIRECT_STATE_ARMED;
    else if (gw->gateway_state == GATEWAY_RUNNING)
        gw->channel->gwState=DIRECT_STATE_GW_READY|DIRECT_STATE_DISCOVERED|
            DIRECT_STATE_ARMED|DIRECT_STATE_RUNNING;
    else if (gw->gateway_state == GATEWAY_FAULT_SAFE)
        gw->channel->gwState=DIRECT_STATE_FAULT|GwSafeBit(gw);
    else
        gw->channel->gwState=GwSafeBit(gw);
    gw->channel->gwStage=(uint32_t)gw->gateway_state;
    state=gw->channel->gwState;
    if (gw->ctrl_stable_count >= DIRECT_CTRL_STABLE_HEARTBEATS) {
        if (gw->gateway_state == GATEWAY_RUNNING) {
            gw->feedback.onlineMask=(uint8_t)gw->active_joint_mask;
            gw->feedback.validMask=(uint8_t)gw->active_joint_mask;
        }
        PublishFeedback(gw,state,gw->last_target);
        if (gw->gateway_state == GATEWAY_RUNNING)
            gw->snapshot_mask=0U;
    }
}

/* ---- 节拍、CAN 配置、异常兜底 ---- */
static void GwPublishStats(void)
{
    U32 i;
    U32 k;

    GW_MARK[GW_W_WORKMAX] = g_gwBeat.workMax;
    GW_MARK[GW_W_WORKAVG] = g_gwBeat.workAvg;
    GW_MARK[GW_W_TOTAL] = g_gwBeat.total;
    GW_MARK[GW_W_TOTALMAX] = g_gwBeat.totalMax;
    GW_MARK[GW_W_TOTALAVG] = g_gwBeat.totalAvg;
    GW_MARK[GW_W_INTVMAX] = g_gwBeat.intvMax;
    GW_MARK[GW_W_LATE] = g_gwBeat.late;
    GW_MARK[GW_W_OVERRUN] = g_gwBeat.overrun;
    for (k = 0; k < 8U; k++) {
        GW_MARK[GW_W_HIST + k] = g_gwBeat.hist[k];
    }
    GW_MARK[GW_W_TICKS] = (U32)PRT_TickGetCount();
    GW_MARK[GW_W_CNTPCT] = (U32)GwSpiNow();
    GW_MARK[GW_W_ALGOITER] = g_gwAlgoIters;
    for (k = 0; k < GW_ALGO_SEGS; k++) {
        GW_MARK[GW_W_ALGO + 3U * k] = g_gwAlgo.last[k];
        GW_MARK[GW_W_ALGO + 3U * k + 1U] = g_gwAlgo.avg[k];
        GW_MARK[GW_W_ALGO + 3U * k + 2U] = g_gwAlgo.max[k];
    }
    GW_MARK[GW_W_ALGOSTEPS] = g_gwAlgo.steps;
    for (i = 0; i < DIRECT_NUM_ARMS; i++) {
        const struct GwCanLane *ln = &g_gwLane[i];
        volatile U32 *w = &GW_MARK[GW_W_LANE(i)];
        w[0] = ln->txFrames;
        w[1] = ln->rxFrames;
        w[2] = ln->rxDropped;
        w[3] = ln->rxOverflow;
        w[4] = ln->txQueueFull;
        w[5] = ln->txFifoBusy;
        w[6] = ln->layoutErr;
        w[7] = ln->bus.timeouts;
        w[8] = ln->maxRxOcc;
        w[9] = g_gwBeat.trec[i];
        GW_MARK[GW_W_STAGLITCH + i] = ln->rxStaGlitch;
        GW_MARK[GW_W_STAGLITCH + 2U + i] = ln->txStaGlitch;
        GW_MARK[GW_W_ARMBLOCK(i)] = g_gateway[i].arm_blocked;
    }
    {
        U32 pend = 0;
        for (i = 0U; i < DIRECT_NUM_ARMS && i < 4U; i++)
            pend |= (U32)g_gateway[i].disable_pending << (8U * i);
        GW_MARK[GW_W_DISABLEPEND] = pend;
    }
}

/* 拍起点记账：拍间隔、上一拍的「拍起点 → 算法本轮结束」总耗时与算法超拍 */
static void GwBeatOpen(U64 t0)
{
    U32 k;

    if (g_gwBeat.lastT0 != 0U) {
        U32 intv = GwNs(t0 - g_gwBeat.lastT0);
        U64 algoEnd = g_gwAlgoEnd;
        if (intv > g_gwBeat.intvMax) {
            g_gwBeat.intvMax = intv;
        }
        if (intv > GW_LATE_NS) {
            g_gwBeat.late++;
        }
        if (g_gwBeat.posted != 0U) {
            if (g_gwAlgoIters == g_gwBeat.postedIters) {
                g_gwBeat.overrun++;
            } else if (algoEnd > g_gwBeat.lastT0) {
                g_gwBeat.total = GwNs(algoEnd - g_gwBeat.lastT0);
                if (g_gwBeat.total > g_gwBeat.totalMax) {
                    g_gwBeat.totalMax = g_gwBeat.total;
                }
                for (k = 0; k < 7U && g_gwBeat.total >= g_gwHistEdgeUs[k] * 1000U; k++) {
                }
                g_gwBeat.hist[k]++;
                g_gwBeat.totalSum += g_gwBeat.total;
                if (++g_gwBeat.totalN == 1000U) {
                    g_gwBeat.totalAvg = (U32)(g_gwBeat.totalSum / 1000U);
                    g_gwBeat.totalSum = 0;
                    g_gwBeat.totalN = 0;
                }
            }
        }
    }
    g_gwBeat.lastT0 = t0;
}

/* 两路同步收：本拍单步里必走 DrainCan 的臂（通道已就绪且 CAN 已初始化，后者置位后
 * 不再清零）先收好；其余臂照旧在单步里自己收 */
static void GwReceiveBoth(void)
{
    struct GwCanLane *lane[DIRECT_NUM_ARMS];
    struct GwCanFrame *out[DIRECT_NUM_ARMS];
    GatewayInstance *gw[DIRECT_NUM_ARMS];
    U32 got[DIRECT_NUM_ARMS];
    U32 n = 0;
    U32 arm;

    for (arm = 0U; arm < DIRECT_NUM_ARMS; arm++) {
        g_gateway[arm].rx_pre = 0U;
        if (g_gateway[arm].ipc_ready != 0U && g_gateway[arm].can_initialized != 0U) {
            gw[n] = &g_gateway[arm];
            lane[n] = g_gateway[arm].lane;
            out[n] = g_gateway[arm].rx_pre_frames;
            n++;
        }
    }
    GwCanReceiveAll(lane, n, out, got);
    for (arm = 0U; arm < n; arm++) {
        gw[arm]->rx_pre_n = got[arm];
        gw[arm]->rx_pre = 1U;
    }
}

/* 可以改 CAN 配置：没有电机在使能或失能待确认，也没有新鲜的 arm 请求（判法同 GatewayStep） */
static uint8_t GwArmQuiet(const GatewayInstance *gw)
{
    uint8_t arm_request = (uint8_t)((gw->latest_target.flags & DIRECT_TARGET_FLAG_ARM_REQUEST) != 0U &&
        gw->have_target != 0U && GwNowNs() - gw->last_target_ns <= DIRECT_TARGET_TIMEOUT_NS);
    return (uint8_t)(gw->motors_enabled == 0U && gw->disable_pending == 0U &&
        gw->gateway_state != GATEWAY_ARMED && gw->gateway_state != GATEWAY_RUNNING &&
        arm_request == 0U);
}

/* 按 CAN 配置块重绑两臂：swap 时互换两臂所用总线，TDCO 变了的总线进配置模式改写
 * （GwCanSetTdco，FIFO 清空）。之后已初始化的臂重走 OpenArmCanInit（重登记电机表，
 * 清掉旧实测），DISARMED 退回发现；电机凑齐前反馈不报在线，算法拿不到换线前那一路
 * 的位姿 */
static void GwRebindLanes(uint8_t swap, U32 tdco)
{
    struct GwCanLane *lane = g_gateway[0].lane;
    U32 arm;

    if (swap != 0U) {
        g_gateway[0].lane = g_gateway[1].lane;
        g_gateway[1].lane = lane;
    }
    for (arm = 0U; arm < DIRECT_NUM_ARMS; arm++) {
        if (g_gwLane[arm].tdco != tdco) {
            (void)GwCanSetTdco(&g_gwLane[arm], tdco);
        }
    }
    for (arm = 0U; arm < DIRECT_NUM_ARMS; arm++) {
        GatewayInstance *gw = &g_gateway[arm];
        GwCanDropQueued(gw->lane);
        gw->discovered_mask = 0U;
        gw->snapshot_mask = 0U;
        gw->enabled_set = 0U;
        gw->feedback_stable_count = 0U;
        gw->refresh_countdown = 0U;
        gw->arm_edge_ready = 0U;
        (void)memset(&gw->feedback, 0, sizeof(gw->feedback));
        (void)memset(gw->joint_last_ns, 0, sizeof(gw->joint_last_ns));
        if (gw->can_initialized != 0U) {
            gw->can_initialized = 0U;
            if (OpenArmCanInit(gw) != CANFD_OK) {
                SetFault(gw, DIRECT_FAULT_CAN);
                gw->gateway_state = GATEWAY_FAULT_SAFE;
            }
        }
        if (gw->gateway_state == GATEWAY_DISARMED) {
            gw->gateway_state = GATEWAY_WAIT_FEEDBACK;
        }
        if (gw->gateway_state == GATEWAY_WAIT_FEEDBACK) {
            gw->discovery_start_ns = GwNowNs();
        }
    }
}

/* CAN 配置请求（direct_ipc.h DirectCanConfig），拍首、两路收发之前处理：只在两臂
 * 都满足 GwArmQuiet 时生效，否则回 BUSY */
static void GwServiceCanConfig(void)
{
    volatile struct DirectCanConfig *cfg = g_gwCanCfg;
    U32 sequence = cfg->sequence;
    U32 map;
    U32 tdco;

    if (sequence == cfg->ackSequence) {
        return;
    }
    GwBarrier();
    map = cfg->map;
    tdco = cfg->tdco;
    if ((map != DIRECT_CANMAP_DEFAULT && map != DIRECT_CANMAP_SWAPPED) ||
        tdco > DIRECT_CANCFG_TDCO_MAX) {
        cfg->result = DIRECT_CANCFG_RESULT_INVALID;
    } else if (map == cfg->appliedMap && tdco == cfg->appliedTdco) {
        cfg->result = DIRECT_CANCFG_RESULT_ACCEPTED;
    } else if (GwArmQuiet(&g_gateway[0]) == 0U || GwArmQuiet(&g_gateway[1]) == 0U) {
        cfg->result = DIRECT_CANCFG_RESULT_BUSY;
    } else {
        GwRebindLanes((uint8_t)(map != cfg->appliedMap), tdco);
        cfg->appliedMap = map;
        cfg->appliedTdco = tdco;
        cfg->changes++;
        cfg->result = DIRECT_CANCFG_RESULT_ACCEPTED;
    }
    GwBarrier();
    cfg->ackSequence = sequence;
    GwBarrier();
}

void GwGatewayBeat(void)
{
    U64 t0 = GwSpiNow();
    U32 arm;
    U32 cnt = 1U;

    GwBeatOpen(t0);
    GwServiceCanConfig();
    /* 先发上一拍排队的帧，再收：指令 ID 0x01-0x07 仲裁优先于回帧 0x11-0x17，一批 7 帧
     * 收发约占总线 0.75ms。拍末才发时，发送时刻随本拍收帧多少漂移，回帧会错过下一拍
     * 的收帧窗口；拍首发则发送时刻固定，回帧在下一拍收之前到齐，每拍收发量均匀。
     * 收发都两路同步：每笔 SPI 事务两路同时移位（canfd_gw_can.h）。 */
    GwCanFlushAll(g_gwLanes, DIRECT_NUM_ARMS);
    GwReceiveBoth();
    for (arm = 0U; arm < DIRECT_NUM_ARMS; arm++) {
        GatewayStep(&g_gateway[arm]);
    }
    /* 算法跑不完时信号量不累积：只在计数为 0 时释放，超拍由 GwBeatOpen 记账 */
    g_gwBeat.posted = 0U;
    if (g_gwSemReady != 0U && PRT_SemGetCount(g_gwSem, &cnt) == OS_OK && cnt == 0U) {
        g_gwBeat.postedIters = g_gwAlgoIters;
        g_gwBeat.posted = 1U;
        g_gwBeat.postT = GwSpiNow();
        (void)PRT_SemPost(g_gwSem);
    }
    g_gwBeat.work = GwNs(GwSpiNow() - t0);
    g_gwBeat.beats++;
    if (g_gwBeat.work > g_gwBeat.workMax) {
        g_gwBeat.workMax = g_gwBeat.work;
    }
    g_gwBeat.workSum += g_gwBeat.work;
    if (++g_gwBeat.workN == 1000U) {
        g_gwBeat.workAvg = (U32)(g_gwBeat.workSum / 1000U);
        g_gwBeat.workSum = 0;
        g_gwBeat.workN = 0;
    }
    GW_MARK[GW_W_BEATS] = g_gwBeat.beats;
    GW_MARK[GW_W_WORK] = g_gwBeat.work;
    if ((g_gwBeat.beats % GW_TREC_BEATS) == 0U) {
        for (arm = 0U; arm < DIRECT_NUM_ARMS; arm++) {
            g_gwBeat.trec[arm] = GwCanTrec(&g_gwLane[arm]);
        }
    }
    if ((g_gwBeat.beats % GW_PUBLISH_BEATS) == 0U) {
        GwPublishStats();
    }
}

void GwGatewayAlgoMark(U32 mark)
{
    if (mark == GW_ALGO_MARK_WAKE) {
        g_gwAlgoWakePost = g_gwBeat.postT;
    }
    if (mark <= GW_ALGO_MARK_STEP) {
        g_gwAlgoMarkT[mark] = GwSpiNow();
    }
}

static void GwAlgoSample(U32 seg, U64 cycles)
{
    U32 ns = GwNs(cycles);

    g_gwAlgo.last[seg] = ns;
    if (ns > g_gwAlgo.max[seg]) {
        g_gwAlgo.max[seg] = ns;
    }
    g_gwAlgo.sum[seg] += ns;
    if (++g_gwAlgo.n[seg] == 1000U) {
        g_gwAlgo.avg[seg] = (U32)(g_gwAlgo.sum[seg] / 1000U);
        g_gwAlgo.sum[seg] = 0;
        g_gwAlgo.n[seg] = 0;
    }
}

/* 结算刚结束的一轮：边界 = 释放信号量、起跑、镜像完、算法完、本轮结束，相邻两个都到了
 * 才计该段；释放早于上一轮结束（算法没在等）不计唤醒段 */
static void GwAlgoAccount(U64 end)
{
    U64 b[GW_ALGO_SEGS + 1U];
    U32 k;

    b[0] = g_gwAlgoWakePost;
    for (k = 0; k <= GW_ALGO_MARK_STEP; k++) {
        b[k + 1U] = g_gwAlgoMarkT[k];
        g_gwAlgoMarkT[k] = 0;
    }
    b[GW_ALGO_SEGS] = (b[GW_ALGO_MARK_STEP + 1U] != 0U) ? end : 0U;
    if (b[0] < g_gwAlgo.prevEnd) {
        b[0] = 0;
    }
    g_gwAlgo.prevEnd = end;
    if (b[GW_ALGO_MARK_STEP + 1U] != 0U) {
        g_gwAlgo.steps++;
    }
    for (k = 0; k < GW_ALGO_SEGS; k++) {
        if (b[k] == 0U || b[k + 1U] < b[k] || (k >= 2U && g_gwAlgo.steps < 2U)) {
            continue;
        }
        GwAlgoSample(k, b[k + 1U] - b[k]);
    }
}

void GwGatewayAlgoWait(U32 timeoutTicks)
{
    U64 end = GwSpiNow();

    g_gwAlgoEnd = end;
    g_gwAlgoIters++;
    GwAlgoAccount(end);
    if (g_gwSemReady != 0U) {
        (void)PRT_SemPend(g_gwSem, timeoutTicks);
    } else {
        PRT_TaskDelay(timeoutTicks);
    }
}

/* 纯 PIO、不依赖中断与调度：中止并清空两路 TX FIFO，补发 7 关节 + 夹爪 DISABLE。
 * 只发一次；两路 CAN 初始化完成后才生效 */
static void GwPanicDisable(void)
{
    U8 data[8];
    U32 arm;

    if (g_gwPanicReady == 0U || g_gwPanicDone != 0U) {
        return;
    }
    g_gwPanicDone = 1U;
    OpenArmPackCommand(OPENARM_CMD_DISABLE, data);
    for (arm = 0U; arm < DIRECT_NUM_ARMS; arm++) {
        GwCanPanicSend(&g_gwLane[arm], 1U, DIRECT_MOTOR_COUNT, data);   /* send ID 0x01~0x08 */
    }
    GW_MARK[GW_W_FLAGS] |= 0x100U;
}

/* 实例崩溃兜底：异常处理先打印调用栈（串口，几十 ms）再调本钩子，随后核下电 */
static U32 GwExcHook(struct ExcInfo *excInfo)
{
    GW_MARK[GW_W_EXC] = 0xE0000000U | (excInfo->excCause & 0xFFFFU);
    GwPanicDisable();
    return OS_OK;
}

/* MICA CPU_OFF（mica stop）：IPI 中断里、核下电之前调用。不发的话，
 * TIMEOUT=0 的达妙电机会一直保持最后一条命令 */
void GwGatewayPowerOffDisable(void)
{
    if (g_gwPanicReady != 0U) {
        GW_MARK[GW_W_FLAGS] |= 0x200U;
    }
    GwPanicDisable();
}

/* 两路 CAN 硬件初始化（一次约 0.1s），之后进入拍循环 */
void GwGatewaySetup(void)
{
    U32 arm;
    U32 w;
    U32 flags = 0;

    for (w = GW_W_MAGIC; w <= GW_W_LAST; w++) {
        GW_MARK[w] = 0;
    }
    GW_MARK[GW_W_LAYOUT] = GW_STATS_LAYOUT;
    GW_MARK[GW_W_MAGIC] = GW_STATS_MAGIC;
    GW_MARK[GW_W_STAGE] = 1U;
    for (arm = 0U; arm < DIRECT_NUM_ARMS; arm++) {
        GatewayInstance *gw = &g_gateway[arm];
        volatile struct DirectGripperState *g =
            (volatile struct DirectGripperState *)((uintptr_t)gw->channel + DIRECT_GRIP_OFFSET);
        g->seq = 0;
        g->polls = 0;
        g->replies = 0;
        g->timestampUs = 0;
        g->q = 0.0F;
        g->dq = 0.0F;
        g->tau = 0.0F;
        g->temps = 0;
        g->raw[0] = 0;
        g->raw[1] = 0;
        g->magic = DIRECT_GRIP_MAGIC;
        gw->grip = g;
        gw->grip_countdown = arm * (DIRECT_GRIP_POLL_BEATS / 2U);   /* 两臂错开半个周期 */
        (void)GwCanInit(gw->lane);
        if (gw->lane->init == GW_CAN_OK) {
            flags |= 1U << arm;
        }
        if (gw->lane->bus.hz == GW_SPI_FAST_HZ) {
            flags |= 4U << arm;
        }
        if (gw->lane->bus.wide != 0U) {
            flags |= 0x10U << arm;
        }
    }
    GW_MARK[GW_W_INIT] = ((U32)g_gwLane[0].init & 0xFFU) | (((U32)g_gwLane[1].init & 0xFFU) << 8);
    GW_MARK[GW_W_FLAGS] = flags;
    g_gwCanCfg = (volatile struct DirectCanConfig *)((uintptr_t)g_gateway[0].channel + DIRECT_CANCFG_OFFSET);
    g_gwCanCfg->magic = 0U;
    g_gwCanCfg->map = DIRECT_CANMAP_DEFAULT;
    g_gwCanCfg->tdco = GW_CAN_TDCO_DEFAULT;
    g_gwCanCfg->appliedMap = DIRECT_CANMAP_DEFAULT;
    g_gwCanCfg->appliedTdco = GW_CAN_TDCO_DEFAULT;
    g_gwCanCfg->result = 0U;
    g_gwCanCfg->changes = 0U;
    g_gwCanCfg->ackSequence = g_gwCanCfg->sequence;
    GwBarrier();
    g_gwCanCfg->magic = DIRECT_CANCFG_MAGIC;
    g_gwPanicReady = 1U;
    (void)PRT_ExcRegHook(GwExcHook);
    GW_MARK[GW_W_STAGE] = 2U;
}

static void GwGatewayTaskEntry(void)
{
    GwGatewaySetup();
    for (;;) {
        GwGatewayBeat();
        PRT_TaskDelay(1U);
    }
}

U32 GwGatewayInit(void)
{
    U32 ret;
    U32 arm;
    TskHandle handle;
    struct TskInitParam param = {0};

    g_gateway[0].channel = DIRECT_CHANNEL_RIGHT_PTR;
    g_gateway[1].channel = DIRECT_CHANNEL_LEFT_PTR;
    for (arm = 0U; arm < DIRECT_NUM_ARMS; arm++) {
        g_gateway[arm].gateway_state = GATEWAY_WAIT_IPC;
    }
    ret = PRT_SemCreate(0U, &g_gwSem);
    if (ret != OS_OK) {
        PRT_Printf("[gateway] semaphore create failed, ret=0x%x\n", ret);
        return ret;
    }
    g_gwSemReady = 1U;
    param.taskEntry = (TskEntryFunc)GwGatewayTaskEntry;
    param.taskPrio = GW_TASK_PRIO;
    param.stackSize = GW_TASK_STACK;
    param.name = "gateway";
    ret = PRT_TaskCreate(&handle, &param);
    if (ret != OS_OK) {
        PRT_Printf("[gateway] gateway task create failed, ret=0x%x\n", ret);
        return ret;
    }
    PRT_Printf("[gateway] 1 kHz, right arm SPI0 @0x40100000, left arm SPI2 @0x40102000\n");
    return PRT_TaskResume(handle);
}
