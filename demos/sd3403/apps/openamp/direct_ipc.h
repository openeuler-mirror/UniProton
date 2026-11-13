#ifndef DIRECT_IPC_H
#define DIRECT_IPC_H

#include "prt_typedef.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Direct IPC: one channel per arm in Linux-reserved memory, shared by the
 * control task (ctrl), the CAN-FD gateway task (gw) and Linux tools
 * (tools/direct-ctl.py). Each channel holds two SPSC rings: targets from the
 * control task to the gateway and feedback from the gateway to the control
 * task. Every message carries magic, version, the channel generation and a
 * CRC32; the receiver raises a fault bit for anything corrupt and drops
 * messages of an earlier generation unread. Linux reads the header for
 * status and writes the command and configuration words. */
#define DIRECT_CHANNEL_RIGHT_ADDR 0x40100000ULL
#define DIRECT_CHANNEL_LEFT_ADDR 0x40102000ULL
#define DIRECT_CHANNEL_SLOT_SIZE 0x2000U
#define DIRECT_CHANNEL_MAGIC 0x52445452U
#define DIRECT_CHANNEL_VERSION 1U
#define DIRECT_RING_CAP 16U
#define DIRECT_CHANNEL_HEADER_SIZE 364U

/* Each arm has seven joints plus the gripper as motor 8 (slot 7, a DM4310
 * on CAN 0x08/0x18); bit n of a joint mask selects motor n + 1. */
#define DIRECT_ROBOT_JOINT_COUNT 7U
#define DIRECT_MOTOR_COUNT 8U
#define DIRECT_GRIPPER_SLOT 7U

#define DIRECT_STATE_CTRL_READY (1U << 0)
#define DIRECT_STATE_GW_READY (1U << 1)
#define DIRECT_STATE_RUNNING (1U << 2)
#define DIRECT_STATE_FAULT (1U << 3)
#define DIRECT_STATE_SAFE (1U << 4)
#define DIRECT_STATE_RECOVERING (1U << 5)
#define DIRECT_STATE_STOPPING (1U << 6)
#define DIRECT_STATE_ARMED (1U << 7)
#define DIRECT_STATE_DISARMED (1U << 8)
#define DIRECT_STATE_DISCOVERED (1U << 9)

#define DIRECT_FAULT_BAD_MESSAGE (1U << 0)
#define DIRECT_FAULT_SEQUENCE (1U << 1)
#define DIRECT_FAULT_TARGET_TIMEOUT (1U << 2)
#define DIRECT_FAULT_TARGET_OVERFLOW (1U << 3)
#define DIRECT_FAULT_STATUS_OVERFLOW (1U << 4)
#define DIRECT_FAULT_FEEDBACK_TIMEOUT (1U << 5)
#define DIRECT_FAULT_CAN (1U << 6)
#define DIRECT_FAULT_OFFLINE (1U << 7)
#define DIRECT_FAULT_FATAL_MASK (DIRECT_FAULT_BAD_MESSAGE | DIRECT_FAULT_SEQUENCE | \
    DIRECT_FAULT_TARGET_TIMEOUT | DIRECT_FAULT_TARGET_OVERFLOW | \
    DIRECT_FAULT_STATUS_OVERFLOW | DIRECT_FAULT_FEEDBACK_TIMEOUT | \
    DIRECT_FAULT_CAN | DIRECT_FAULT_OFFLINE)

/* Commands Linux posts through testCommand/testArgument/testSequence; the
 * control task acknowledges with testAckSequence. Arm, disarm and reset
 * apply to both arms; reset clears every fault and starts a new generation
 * (default parameters, masks kept). The others inject faults to exercise
 * the checks. */
#define DIRECT_TEST_CORRUPT_TARGET_CRC 0x54450001U
#define DIRECT_TEST_PAUSE_TARGET 0x54450002U
#define DIRECT_TEST_PAUSE_STATUS_CONSUMER 0x54450003U
#define DIRECT_TEST_BAD_TARGET_VERSION 0x54450004U
#define DIRECT_TEST_ARM_MOTORS 0x54450006U
#define DIRECT_TEST_DISARM_MOTORS 0x54450007U
#define DIRECT_TEST_RESET_FAULTS 0x5445FFFFU
#define DIRECT_TEST_RESULT_ACCEPTED 1U

#define DIRECT_TARGET_FLAG_ARM_REQUEST (1U << 0)

enum DirectRobotMessageType {
    DIRECT_MSG_HELLO = 1U,
    DIRECT_MSG_HEARTBEAT = 2U,
    DIRECT_MSG_JOINT_TARGET = 3U,
    DIRECT_MSG_STOP = 4U,
};

#define DIRECT_PROTOCOL_MAGIC 0x4F41524DU
#define DIRECT_PROTOCOL_VERSION 1U
#define DIRECT_ALL_JOINTS_MASK 0xFFU
#define DIRECT_DEFAULT_JOINT_MASK 0xFFU             /* power-up mask: all eight motors */
#define DIRECT_PARAM_SCALE 1000000

struct DirectRobotTargetMessage {
    U32 magic;
    U16 version;
    U16 type;
    U32 sequence;
    U32 generation;
    U32 flags;
    float qTarget[DIRECT_MOTOR_COUNT];
    float dqTarget[DIRECT_MOTOR_COUNT];
    float tauFf[DIRECT_MOTOR_COUNT];
    float kp[DIRECT_MOTOR_COUNT];
    float kd[DIRECT_MOTOR_COUNT];
    U32 crc32;
};

struct DirectRobotFeedbackMessage {
    U32 magic;
    U16 version;
    U16 state;
    U32 sampleSequence;
    U32 acceptedSequence;
    U32 generation;
    U32 faultBits;
    U32 timestampUs;
    float q[DIRECT_MOTOR_COUNT];
    float dq[DIRECT_MOTOR_COUNT];
    float tau[DIRECT_MOTOR_COUNT];
    U8 tMos[DIRECT_MOTOR_COUNT];
    U8 tRotor[DIRECT_MOTOR_COUNT];
    U8 onlineMask;
    U8 validMask;
    U8 reserved[2];                            /* explicit padding: the CRC never covers compiler padding */
    U32 crc32;
};

/* Joint parameters of one arm, J1-J7, in DIRECT_PARAM_SCALE units; the
 * channel holds the same rows. Linux writes the rows (and the requested
 * mask), then increments configSequence. While the arm is DISARMED or in
 * fault, the gateway checks the request (DirectJointParamsValid) and
 * acknowledges it with configAckSequence; the control task then takes its
 * own copy of the rows and checks it again, so rows rewritten without a new
 * sequence are never used unchecked. An invalid request is never
 * acknowledged. Every new generation restores the defaults. */
struct DirectJointParams {
    S32 gravityScale[DIRECT_ROBOT_JOINT_COUNT];    /* x gravity torque, 0..3 */
    S32 gravityOffset[DIRECT_ROBOT_JOINT_COUNT];   /* Nm, -20..20 */
    S32 jointKp[DIRECT_ROBOT_JOINT_COUNT];         /* Nm/rad, (0, 100] */
    S32 jointKd[DIRECT_ROBOT_JOINT_COUNT];         /* Nm*s/rad, 0..5 (OPENARM_KD_MAX) */
    S32 yieldTorque[DIRECT_ROBOT_JOINT_COUNT];     /* clutch spring limit, Nm, (0, 10] */
    S32 inertiaDamping[DIRECT_ROBOT_JOINT_COUNT];  /* extra kd per rad/s, 0..10 */
    S32 inertiaCapture[DIRECT_ROBOT_JOINT_COUNT];  /* clutch shift per rad/s, s, 0..2 */
};

#define DIRECT_DEFAULT_JOINT_PARAMS { \
    { 1200000, 1100000, 1100000, 1100000, 1100000, 1100000, 1100000 }, \
    { 0, 0, 0, 0, 0, 0, 0 }, \
    { 32000000, 32000000, 26000000, 26000000, 6000000, 6000000, 14000000 }, \
    { 1900000, 1900000, 1550000, 1550000, 420000, 420000, 950000 }, \
    { 750000, 750000, 500000, 500000, 180000, 180000, 250000 }, \
    { 0, 0, 0, 0, 0, 0, 0 }, \
    { 0, 0, 0, 0, 0, 0, 0 }, \
}

static inline int DirectJointParamsValid(const struct DirectJointParams *p)
{
    U32 i;

    for (i = 0U; i < DIRECT_ROBOT_JOINT_COUNT; i++) {
        if (p->gravityScale[i] < 0 || p->gravityScale[i] > 3000000 ||
            p->gravityOffset[i] < -20000000 || p->gravityOffset[i] > 20000000 ||
            p->jointKp[i] <= 0 || p->jointKp[i] > 100000000 ||
            p->jointKd[i] < 0 || p->jointKd[i] > 5000000 ||
            p->yieldTorque[i] <= 0 || p->yieldTorque[i] > 10000000 ||
            p->inertiaDamping[i] < 0 || p->inertiaDamping[i] > 10000000 ||
            p->inertiaCapture[i] < 0 || p->inertiaCapture[i] > 2000000)
            return 0;
    }
    return 1;
}

struct DirectIpcChannel {
    volatile U32 magic;
    volatile U32 version;
    volatile U32 headerSize;
    volatile U32 generation;
    volatile U32 state;
    volatile U32 ctrlState;
    volatile U32 gwState;
    volatile U32 ctrlBootCount;
    volatile U32 gwBootCount;
    volatile U32 ctrlHeartbeat;
    volatile U32 gwHeartbeat;
    volatile U32 ctrlSeenGeneration;
    volatile U32 gwSeenGeneration;
    volatile U32 faultBits;
    volatile U32 targetProducer;
    volatile U32 targetConsumer;
    volatile U32 statusProducer;
    volatile U32 statusConsumer;
    volatile U32 targetOverflowCount;
    volatile U32 statusOverflowCount;
    volatile U32 badMessageCount;
    volatile U32 sequenceFaultCount;
    volatile U32 lastTargetSequence;
    volatile U32 lastStatusSequence;
    volatile U32 ctrlStage;
    volatile U32 gwStage;
    volatile U32 ctrlTxCount;
    volatile U32 gwTxCount;
    volatile U32 errorCode;
    volatile U32 activeFaultBits;
    volatile U32 ctrlActiveFaultBits;
    volatile U32 gwActiveFaultBits;
    volatile U32 latchedFaultBits;
    volatile U32 testCommand;
    volatile U32 testArgument;
    volatile U32 testSequence;
    volatile U32 testAckSequence;
    volatile U32 testResult;
    volatile U32 requestedJointMask;
    volatile U32 effectiveJointMask;
    volatile U32 configSequence;
    volatile U32 configAckSequence;
    /* struct DirectJointParams, row by row */
    volatile S32 gravityScale[DIRECT_ROBOT_JOINT_COUNT];
    volatile S32 gravityOffset[DIRECT_ROBOT_JOINT_COUNT];
    volatile S32 jointKp[DIRECT_ROBOT_JOINT_COUNT];
    volatile S32 jointKd[DIRECT_ROBOT_JOINT_COUNT];
    volatile S32 yieldTorque[DIRECT_ROBOT_JOINT_COUNT];
    volatile S32 inertiaDamping[DIRECT_ROBOT_JOINT_COUNT];
    volatile S32 inertiaCapture[DIRECT_ROBOT_JOINT_COUNT];
    struct DirectRobotTargetMessage target[DIRECT_RING_CAP];
    struct DirectRobotFeedbackMessage status[DIRECT_RING_CAP];
};

/* Gripper telemetry, after the channel body in each arm's 8 KiB slot. The
 * gateway sends the gripper one frame every DIRECT_GRIP_POLL_BEATS beats:
 * a read-back request while it is not enabled (the motor replies with its
 * state and stays disabled), an MIT frame once it is; the two arms are half
 * a period apart. Each 0x18 reply is decoded as a DM4310 and written here.
 * seq is a sequence lock: +1 before and after the reply fields, so a reader
 * that sees it odd or changed reads again. */
#define DIRECT_GRIP_OFFSET 0x1800U
#define DIRECT_GRIP_MAGIC 0x31505247U          /* "GRP1" */
#define DIRECT_GRIP_POLL_BEATS 4U

struct DirectGripperState {
    volatile U32 magic;
    volatile U32 seq;
    volatile U32 polls;                        /* gripper frames queued (read-back or MIT) */
    volatile U32 replies;                      /* 0x18 frames received */
    volatile U32 timestampUs;                  /* gateway time of the latest reply */
    volatile float q;                          /* rad */
    volatile float dq;                         /* rad/s */
    volatile float tau;                        /* Nm */
    volatile U32 temps;                        /* tMos | tRotor << 8 (degC, signed bytes) */
    volatile U32 raw[2];                       /* latest 8 data bytes, little-endian words */
};

/* CAN configuration, after the gripper block in the right arm's slot: which
 * bus serves each arm and the TDC offset both buses use. Every boot restores
 * the defaults (right arm SPI0, left arm SPI2, TDCO 6). Linux writes map and
 * tdco, then increments sequence; the gateway applies the request only while
 * no motor of either arm is enabled or awaiting a confirmed disable and no
 * live arm request is pending, otherwise it answers BUSY. Either way it then
 * sets ackSequence to sequence. Once applied, an arm that is DISARMED or
 * discovering rediscovers its motors; an arm in fault stays there until
 * reset. Channels, masks and parameters stay with the arm; only the bus
 * under it changes. */
#define DIRECT_CANCFG_OFFSET 0x1840U
#define DIRECT_CANCFG_MAGIC 0x314E4143U        /* "CAN1" */
#define DIRECT_CANMAP_DEFAULT 0U               /* right arm SPI0, left arm SPI2 */
#define DIRECT_CANMAP_SWAPPED 1U               /* right arm SPI2, left arm SPI0 */
#define DIRECT_CANCFG_TDCO_MAX 63U
#define DIRECT_CANCFG_RESULT_ACCEPTED 1U
#define DIRECT_CANCFG_RESULT_BUSY 2U
#define DIRECT_CANCFG_RESULT_INVALID 3U

struct DirectCanConfig {
    volatile U32 magic;
    volatile U32 map;                          /* Linux: DIRECT_CANMAP_DEFAULT / _SWAPPED */
    volatile U32 tdco;                         /* Linux: TDC offset in SYSCLK periods (25 ns), 0..63 */
    volatile U32 sequence;                     /* Linux: +1 per request */
    volatile U32 ackSequence;                  /* gateway: last sequence handled */
    volatile U32 result;                       /* gateway: result of that request */
    volatile U32 appliedMap;                   /* gateway: current mapping */
    volatile U32 appliedTdco;                  /* gateway: current TDCO */
    volatile U32 changes;                      /* gateway: requests applied since boot */
};

typedef char DirectTargetMessageSizeCheck[(sizeof(struct DirectRobotTargetMessage) == 184U) ? 1 : -1];
typedef char DirectFeedbackMessageSizeCheck[(sizeof(struct DirectRobotFeedbackMessage) == 148U) ? 1 : -1];
typedef char DirectHeaderSizeCheck[
    (offsetof(struct DirectIpcChannel, target) == DIRECT_CHANNEL_HEADER_SIZE) ? 1 : -1];
typedef char DirectParamRowsCheck[(offsetof(struct DirectIpcChannel, target) -
    offsetof(struct DirectIpcChannel, gravityScale) == sizeof(struct DirectJointParams)) ? 1 : -1];
typedef char DirectGripperFitCheck[(DIRECT_GRIP_OFFSET >= sizeof(struct DirectIpcChannel) &&
    DIRECT_GRIP_OFFSET + sizeof(struct DirectGripperState) <= DIRECT_CHANNEL_SLOT_SIZE) ? 1 : -1];
typedef char DirectCanConfigFitCheck[(DIRECT_CANCFG_OFFSET >=
    DIRECT_GRIP_OFFSET + sizeof(struct DirectGripperState) &&
    DIRECT_CANCFG_OFFSET + sizeof(struct DirectCanConfig) <= DIRECT_CHANNEL_SLOT_SIZE) ? 1 : -1];

/* Set up both channels and start the control task; call after TestOpenamp()
 * so the MICA lifecycle hook chains onto the rpmsg IPI handler. */
U32 DirectIpcInit(void);

#ifdef __cplusplus
}
#endif

#endif
