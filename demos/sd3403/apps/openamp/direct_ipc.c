#include "direct_ipc.h"
#include "canfd_gw_gateway.h"
#include "openarm_control_dual.h"
#include "prt_config.h"
#include "prt_mem.h"
#include "prt_sys.h"
#include "prt_task.h"
#include "resource_table.h"

/* 32 KiB, as the ALGO_CPP tests' task (FILESYSTEM_TEST_STACK_SIZE): the
 * control step builds both pinocchio models on its first run, which
 * overflows 8 KiB. The step runs as soon as both arms report feedback,
 * armed or not. */
#define DIRECT_TASK_STACK_SIZE 0x8000U
#define DIRECT_TASK_PRIO 20U
#define DIRECT_TASK_PERIOD_TICKS ((OS_TICK_PER_SECOND / 1000U) == 0U ? 1U : (OS_TICK_PER_SECOND / 1000U))
#define MICA_CPU_OFF 0x84000002U
#define DIRECT_ARM_COUNT 2U

/* Per-arm state, indexed 0 = left, 1 = right (OpenArmControlStepDual slot
 * order). */
typedef struct {
    struct DirectRobotFeedbackMessage latest;
    bool haveFeedback;
    U32 lastAcceptedSeq;
    U32 validCount;
    U32 gwBoot;
    U32 effMask;
    U32 seq;
    struct DirectJointParams params;   /* the control step's copy, see direct_ipc.h */
    U32 paramsSeq;                     /* configAckSequence that params was taken at */
} DirectArmState;

static struct DirectIpcChannel *const g_directChannel[DIRECT_ARM_COUNT] = {
    (struct DirectIpcChannel *)(uintptr_t)DIRECT_CHANNEL_LEFT_ADDR,
    (struct DirectIpcChannel *)(uintptr_t)DIRECT_CHANNEL_RIGHT_ADDR,
};
static DirectArmState g_directArm[DIRECT_ARM_COUNT];
static bool g_armRequested;
static TskHandle g_directTask;
static U32 g_pauseTargetCycles;
static U32 g_pauseStatusConsumerCycles;
static bool g_corruptTargetCrc;
static bool g_badTargetVersion;
static const struct DirectJointParams g_defaultParams = DIRECT_DEFAULT_JOINT_PARAMS;

extern void (*g_rpmsg_ipi_handler)(void);

static inline void DirectBarrier(void)
{
    OS_EMBED_ASM("DSB SY" ::: "memory");
}

/* Both channels are mapped Device-nGnRnE (bsp/mmu.c) and this file builds
 * -O0, where a struct assignment is the byte-wise __wrap_memcpy: one
 * uncached access per byte. Ring messages therefore move a word at a time
 * (both sizes are multiples of 4, see direct_ipc.h) and are checksummed in
 * normal memory, as the gateway does. */
static void DirectCopyWords(void *dst, const void *src, U32 bytes)
{
    volatile U32 *d = (volatile U32 *)dst;
    const volatile U32 *s = (const volatile U32 *)src;
    U32 i;

    for (i = 0U; i < bytes / sizeof(U32); i++)
        d[i] = s[i];
}

/* CRC-32 (reflected, polynomial 0xEDB88320), table-driven. */
static U32 g_directCrcTable[256];
static bool g_directCrcReady;

static U32 DirectCrc32(const void *data, U32 length)
{
    const U8 *bytes = (const U8 *)data;
    U32 crc;
    U32 i;
    U32 bit;

    if (!g_directCrcReady) {
        for (i = 0U; i < 256U; i++) {
            crc = i;
            for (bit = 0U; bit < 8U; bit++)
                crc = (crc >> 1U) ^ (0xEDB88320U & (U32)-(S32)(crc & 1U));
            g_directCrcTable[i] = crc;
        }
        g_directCrcReady = true;
    }
    crc = 0xFFFFFFFFU;
    for (i = 0U; i < length; i++)
        crc = (crc >> 8U) ^ g_directCrcTable[(crc ^ bytes[i]) & 0xFFU];
    return ~crc;
}

static void DirectSetFault(struct DirectIpcChannel *ch, U32 fault)
{
    ch->ctrlActiveFaultBits |= fault;
    ch->activeFaultBits = ch->ctrlActiveFaultBits | ch->gwActiveFaultBits;
    ch->latchedFaultBits |= fault;
    ch->faultBits = ch->activeFaultBits;
    DirectBarrier();
}

static void DirectClearFault(struct DirectIpcChannel *ch, U32 fault)
{
    ch->ctrlActiveFaultBits &= ~fault;
    ch->activeFaultBits = ch->ctrlActiveFaultBits | ch->gwActiveFaultBits;
    ch->faultBits = ch->activeFaultBits;
    DirectBarrier();
}

static void DirectClearChannel(struct DirectIpcChannel *ch)
{
    volatile U32 *word = (volatile U32 *)ch;
    U32 i;
    for (i = 0U; i < sizeof(*ch) / sizeof(U32); i++)
        word[i] = 0U;
}

static bool DirectPushTarget(struct DirectIpcChannel *ch,
    struct DirectRobotTargetMessage *source, U32 sequence)
{
    U32 current = ch->targetProducer;
    U32 next = (current + 1U) % DIRECT_RING_CAP;

    if (next == ch->targetConsumer) {
        ch->targetOverflowCount++;
        DirectSetFault(ch, DIRECT_FAULT_TARGET_OVERFLOW);
        return false;
    }
    /* finish the message in the caller's buffer, then copy the slot out */
    source->sequence = sequence;
    source->crc32 = 0U;
    source->crc32 = DirectCrc32(source, sizeof(*source));
    if (g_corruptTargetCrc) {
        source->crc32 ^= 1U;
        g_corruptTargetCrc = false;
    }
    if (g_badTargetVersion) {
        source->version++;
        g_badTargetVersion = false;
    }
    DirectCopyWords(&ch->target[current], source, sizeof(*source));
    DirectBarrier();
    ch->targetProducer = next;
    ch->lastTargetSequence = sequence;
    ch->ctrlTxCount++;
    DirectBarrier();
    return true;
}

static bool DirectPopStatus(struct DirectIpcChannel *ch, DirectArmState *st)
{
    U32 current = ch->statusConsumer;
    struct DirectRobotFeedbackMessage slot;
    U32 crc;

    if (current == ch->statusProducer)
        return false;
    DirectBarrier();
    DirectCopyWords(&slot, &ch->status[current], sizeof(slot));
    ch->statusConsumer = (current + 1U) % DIRECT_RING_CAP;
    DirectBarrier();

    crc = slot.crc32;
    slot.crc32 = 0U;
    if (slot.magic != DIRECT_PROTOCOL_MAGIC ||
        slot.version != DIRECT_PROTOCOL_VERSION ||
        crc != DirectCrc32(&slot, sizeof(slot))) {
        ch->badMessageCount++;
        st->validCount = 0U;   /* cleared only after three valid ones in a row */
        DirectSetFault(ch, DIRECT_FAULT_BAD_MESSAGE);
        return true;
    }
    /* Published before the gateway saw the current generation: drop it. */
    if (slot.generation != ch->generation)
        return true;
    if ((S32)(slot.sampleSequence - st->lastAcceptedSeq) <= 0 && st->lastAcceptedSeq != 0U) {
        ch->sequenceFaultCount++;
        DirectSetFault(ch, DIRECT_FAULT_SEQUENCE);
        return true;
    }
    if ((slot.validMask & st->effMask) == st->effMask &&
        (slot.onlineMask & st->effMask) == st->effMask) {
        st->latest = slot;
        st->haveFeedback = true;
    }
    if ((ch->gwActiveFaultBits & DIRECT_FAULT_BAD_MESSAGE) != 0U) {
        st->validCount = 0U;
    } else {
        if (st->validCount < 3U)
            st->validCount++;
        if (st->validCount >= 3U)
            DirectClearFault(ch, DIRECT_FAULT_BAD_MESSAGE);
    }
    st->lastAcceptedSeq = slot.sampleSequence;
    return true;
}

/* New generation: restore the default parameters; the gateway follows the
 * generation and resets its side. The generation goes out first, so a
 * gateway beat that preempts the rest of this function already works in
 * the new session. Each side keeps its own ring indices: messages still in
 * the rings carry the old generation and both ends drop them unread. */
static void DirectBeginSession(struct DirectIpcChannel *ch, DirectArmState *st)
{
    U32 generation = ch->generation + 1U;

    ch->generation = generation;
    ch->ctrlState = DIRECT_STATE_RECOVERING;
    ch->state = DIRECT_STATE_RECOVERING;
    DirectBarrier();
    st->effMask = ch->effectiveJointMask;
    if (st->effMask == 0U)
        st->effMask = DIRECT_DEFAULT_JOINT_MASK;
    ch->effectiveJointMask = st->effMask;
    DirectCopyWords((void *)&ch->gravityScale[0], &g_defaultParams, sizeof(g_defaultParams));
    st->params = g_defaultParams;
    st->paramsSeq = 1U;
    DirectBarrier();
    ch->configSequence = 1U;
    ch->configAckSequence = 1U;
    ch->lastTargetSequence = 0U;
    st->lastAcceptedSeq = 0U;
    st->haveFeedback = false;
    DirectBarrier();
    ch->ctrlSeenGeneration = generation;
    ch->ctrlState = DIRECT_STATE_CTRL_READY;
    ch->state = DIRECT_STATE_CTRL_READY;
    DirectBarrier();
}

/* Take the parameter rows once the gateway has acknowledged a new request;
 * what Linux writes afterwards waits for the next acknowledgement. */
static void DirectTakeParams(struct DirectIpcChannel *ch, DirectArmState *st)
{
    struct DirectJointParams params;
    U32 ack = ch->configAckSequence;

    if (ack == st->paramsSeq || ack != ch->configSequence)
        return;
    DirectBarrier();
    DirectCopyWords(&params, (const void *)&ch->gravityScale[0], sizeof(params));
    if (DirectJointParamsValid(&params))
        st->params = params;
    st->paramsSeq = ack;
}

/* Commands are global: one operator, two arms. A command posted on either
 * channel acts on both, and is acknowledged on the channel it came from. */
static void DirectHandleTestCommand(struct DirectIpcChannel *ch)
{
    U32 sequence = ch->testSequence;
    U32 argument;
    U32 arm;

    if (sequence == ch->testAckSequence)
        return;
    DirectBarrier();
    argument = ch->testArgument;
    switch (ch->testCommand) {
        case DIRECT_TEST_CORRUPT_TARGET_CRC:
            g_corruptTargetCrc = true;
            break;
        case DIRECT_TEST_PAUSE_TARGET:
            g_pauseTargetCycles = (argument == 0U) ? 1U : argument;
            break;
        case DIRECT_TEST_PAUSE_STATUS_CONSUMER:
            g_pauseStatusConsumerCycles = (argument == 0U) ? 1U : argument;
            break;
        case DIRECT_TEST_BAD_TARGET_VERSION:
            g_badTargetVersion = true;
            break;
        case DIRECT_TEST_ARM_MOTORS:
            g_armRequested = true;
            break;
        case DIRECT_TEST_DISARM_MOTORS:
            g_armRequested = false;
            break;
        case DIRECT_TEST_RESET_FAULTS:
            for (arm = 0U; arm < DIRECT_ARM_COUNT; arm++) {
                struct DirectIpcChannel *c = g_directChannel[arm];
                c->ctrlActiveFaultBits = 0U;
                c->gwActiveFaultBits = 0U;
                c->activeFaultBits = 0U;
                c->latchedFaultBits = 0U;
                c->faultBits = 0U;
            }
            g_pauseTargetCycles = 0U;
            g_pauseStatusConsumerCycles = 0U;
            g_corruptTargetCrc = false;
            g_badTargetVersion = false;
            g_armRequested = false;
            for (arm = 0U; arm < DIRECT_ARM_COUNT; arm++)
                DirectBeginSession(g_directChannel[arm], &g_directArm[arm]);
            break;
        default:
            return;
    }
    ch->testResult = DIRECT_TEST_RESULT_ACCEPTED;
    DirectBarrier();
    ch->testAckSequence = sequence;
    DirectBarrier();
}

static void DirectMirrorChannelState(U32 arm)
{
    struct DirectIpcChannel *ch = g_directChannel[arm];
    DirectArmState *st = &g_directArm[arm];

    st->effMask = ch->effectiveJointMask;
    if (st->effMask == 0U)
        st->effMask = DIRECT_DEFAULT_JOINT_MASK;
    ch->ctrlHeartbeat++;
    DirectHandleTestCommand(ch);
    if (st->gwBoot != ch->gwBootCount) {
        st->gwBoot = ch->gwBootCount;
        st->seq = 0U;
        DirectBeginSession(ch, st);
    }
    if ((ch->gwState & DIRECT_STATE_GW_READY) != 0U &&
        ch->gwSeenGeneration == ch->generation) {
        if ((ch->gwState & DIRECT_STATE_RUNNING) != 0U)
            ch->ctrlState = DIRECT_STATE_RUNNING;
        else
            ch->ctrlState = DIRECT_STATE_CTRL_READY;
        ch->state = DIRECT_STATE_CTRL_READY | ch->gwState;
    }
    if ((ch->gwState & DIRECT_STATE_FAULT) != 0U)
        ch->state |= DIRECT_STATE_FAULT;
    DirectTakeParams(ch, st);

    if (g_pauseStatusConsumerCycles != 0U) {
        g_pauseStatusConsumerCycles--;
    } else {
        while (DirectPopStatus(ch, st)) {
        }
    }
}

/* MICA CPU_OFF (mica stop): disable the motors, mark both channels as
 * stopping and power the core off right here instead of waiting for the
 * rpmsg listen task to do it. Runs after the stock rpmsg hook, which
 * invalidates the D-cache before it reads reserved[0] and handles
 * SYSTEM_RESET (virtqueue reset) on its own. */
static void DirectLifecycleHandler(void)
{
    void *resource;
    int length;
    struct fw_resource_table *table;
    U32 arm;

    rsc_table_get(&resource, &length);
    (void)length;
    table = (struct fw_resource_table *)resource;
    DirectBarrier();
    if (table->reserved[0] == MICA_CPU_OFF) {
        GwGatewayPowerOffDisable();
        for (arm = 0U; arm < DIRECT_ARM_COUNT; arm++) {
            g_directChannel[arm]->ctrlState = DIRECT_STATE_STOPPING;
            g_directChannel[arm]->state = DIRECT_STATE_STOPPING | DIRECT_STATE_SAFE;
        }
        DirectBarrier();
        PRT_SysPowerOff();
    }
}

/* The mcs notification (SGI 7) reaches the rpmsg backend through
 * RpmsgHwiInit()->IrqHandler()->g_rpmsg_ipi_handler. We chain onto that
 * hook instead of replacing it: the rpmsg backend first (tty console
 * included), then the CPU_OFF check. */
static void (*g_nextIpiHandler)(void);

static void DirectIpiHandler(void)
{
    if (g_nextIpiHandler != NULL) {
        g_nextIpiHandler();
    }
    DirectLifecycleHandler();
}

static void DirectIpcTask(void)
{
    struct DirectRobotTargetMessage targetLeft;
    struct DirectRobotTargetMessage targetRight;
    U32 arm;
    U32 lastMaskLeft = 0U;
    U32 lastMaskRight = 0U;

    for (arm = 0U; arm < DIRECT_ARM_COUNT; arm++) {
        struct DirectIpcChannel *ch = g_directChannel[arm];
        ch->ctrlStage = 3U;
        ch->ctrlState = DIRECT_STATE_CTRL_READY;
        ch->ctrlSeenGeneration = ch->generation;
        DirectBeginSession(ch, &g_directArm[arm]);
    }
    for (;;) {
        GwGatewayAlgoMark(GW_ALGO_MARK_WAKE);
        for (arm = 0U; arm < DIRECT_ARM_COUNT; arm++)
            DirectMirrorChannelState(arm);
        GwGatewayAlgoMark(GW_ALGO_MARK_MIRROR);

        if (g_directArm[0].effMask != lastMaskLeft || g_directArm[1].effMask != lastMaskRight) {
            lastMaskLeft = g_directArm[0].effMask;
            lastMaskRight = g_directArm[1].effMask;
            g_directArm[0].haveFeedback = false;
            g_directArm[1].haveFeedback = false;
            OpenArmControlDualReset();
        }
        if (g_directArm[0].haveFeedback && g_directArm[1].haveFeedback &&
            g_pauseTargetCycles == 0U) {
            struct DirectIpcChannel *chLeft = g_directChannel[0];
            struct DirectIpcChannel *chRight = g_directChannel[1];
            bool fatal = (chLeft->activeFaultBits & DIRECT_FAULT_FATAL_MASK) != 0U ||
                (chRight->activeFaultBits & DIRECT_FAULT_FATAL_MASK) != 0U;
            g_directArm[0].haveFeedback = false;
            g_directArm[1].haveFeedback = false;
            if (!fatal) {
                if (!g_armRequested)
                    OpenArmControlDualReset();
                if (chLeft->configAckSequence == chLeft->configSequence &&
                    chRight->configAckSequence == chRight->configSequence) {
                    OpenArmControlStepDual(&g_directArm[0].latest, &g_directArm[1].latest,
                        &targetLeft, &targetRight, g_directArm[0].effMask, g_directArm[1].effMask,
                        &g_directArm[0].params, &g_directArm[1].params);
                    GwGatewayAlgoMark(GW_ALGO_MARK_STEP);
                    if (targetLeft.magic != DIRECT_PROTOCOL_MAGIC ||
                        targetRight.magic != DIRECT_PROTOCOL_MAGIC) {
                        /* Charge each arm's own channel, so a bad target
                         * from one arm does not fault the other. */
                        if (targetLeft.magic != DIRECT_PROTOCOL_MAGIC)
                            DirectSetFault(chLeft, DIRECT_FAULT_BAD_MESSAGE);
                        if (targetRight.magic != DIRECT_PROTOCOL_MAGIC)
                            DirectSetFault(chRight, DIRECT_FAULT_BAD_MESSAGE);
                    } else {
                        if (g_armRequested) {
                            targetLeft.flags |= DIRECT_TARGET_FLAG_ARM_REQUEST;
                            targetRight.flags |= DIRECT_TARGET_FLAG_ARM_REQUEST;
                        }
                        targetLeft.generation = chLeft->generation;
                        targetRight.generation = chRight->generation;
                        g_directArm[0].seq++;
                        g_directArm[1].seq++;
                        (void)DirectPushTarget(chLeft, &targetLeft, g_directArm[0].seq);
                        (void)DirectPushTarget(chRight, &targetRight, g_directArm[1].seq);
                    }
                }
            }
        } else if (g_pauseTargetCycles != 0U) {
            g_pauseTargetCycles--;
        }
        /* The gateway posts once per 1 kHz beat after publishing feedback.
         * The two-tick timeout only matters if the gateway stops posting;
         * one tick would expire at the tick that starts the beat, before
         * its post. */
        GwGatewayAlgoWait(2U * DIRECT_TASK_PERIOD_TICKS);
    }
}

U32 DirectIpcInit(void)
{
    U32 ret;
    U32 arm;
    struct TskInitParam param = {0};

    for (arm = 0U; arm < DIRECT_ARM_COUNT; arm++) {
        struct DirectIpcChannel *ch = g_directChannel[arm];
        U32 generation = 1U;
        U32 ctrlBootCount = 1U;
        if (ch->magic == DIRECT_CHANNEL_MAGIC &&
            ch->version == DIRECT_CHANNEL_VERSION) {
            generation = ch->generation + 1U;
            ctrlBootCount = ch->ctrlBootCount + 1U;
            g_directArm[arm].gwBoot = ch->gwBootCount;
        }
        DirectClearChannel(ch);
        ch->version = DIRECT_CHANNEL_VERSION;
        ch->headerSize = (U32)((uintptr_t)&ch->target[0] - (uintptr_t)ch);
        ch->generation = generation;
        ch->ctrlBootCount = ctrlBootCount;
        ch->gwBootCount = g_directArm[arm].gwBoot;
        ch->requestedJointMask = DIRECT_DEFAULT_JOINT_MASK;
        ch->effectiveJointMask = DIRECT_DEFAULT_JOINT_MASK;
        ch->configSequence = 1U;
        ch->ctrlStage = 1U;
        DirectBarrier();
        ch->magic = DIRECT_CHANNEL_MAGIC;
        ch->state = DIRECT_STATE_CTRL_READY;
        DirectBarrier();
    }

    /* chain onto the stock rpmsg IPI hook instead of replacing it */
    g_nextIpiHandler = g_rpmsg_ipi_handler;
    g_rpmsg_ipi_handler = DirectIpiHandler;

    for (arm = 0U; arm < DIRECT_ARM_COUNT; arm++)
        g_directChannel[arm]->ctrlStage = 2U;
    param.stackAddr = (uintptr_t)PRT_MemAllocAlign(0, OS_MEM_DEFAULT_FSC_PT,
        DIRECT_TASK_STACK_SIZE, MEM_ADDR_ALIGN_016);
    if (param.stackAddr == 0U) {
        g_directChannel[1]->errorCode = 0x58000000U;
        return OS_ERROR;
    }
    param.taskEntry = (TskEntryFunc)DirectIpcTask;
    param.taskPrio = DIRECT_TASK_PRIO;
    param.name = "DirectIpc";
    param.stackSize = DIRECT_TASK_STACK_SIZE;
    ret = PRT_TaskCreate(&g_directTask, &param);
    if (ret == OS_OK)
        ret = PRT_TaskResume(g_directTask);
    if (ret != OS_OK)
        g_directChannel[1]->errorCode = 0x59000000U | ret;
    return ret;
}
