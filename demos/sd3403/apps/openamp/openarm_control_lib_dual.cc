/* openarm_control_lib_dual.cc - dual-arm library-backed OpenArm control.
 *
 * OpenArm's two chains hang off one fixed base with no dynamic coupling
 * between them, so dual-arm control is two independent 7-joint admittance
 * controllers, each with its own URDF-verified parameters
 * (openarm_description v1.urdf).
 *
 * The right arm carries the model and control tunables verified on the
 * robot. The left arm differs from the right exactly in (URDF facts):
 *   - base-mount roll and joint2 origin roll are -pi/2 (right: +pi/2)
 *   - joint1/2 limits are mirrored (negated and swapped)
 *   - joint7 axis is [0,-1,0] (right: [0,1,0])
 *   - com y is negated for links 1/2/5/6/7; links 3/4 com are identical
 *   - origins, axes 1..6, masses and all control tunables are shared
 */
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstring>
#include <new>

#include <Eigen/Core>

#include "pinocchio/spatial/se3.hpp"
#include "pinocchio/multibody/model.hpp"
#include "pinocchio/multibody/data.hpp"
#include "pinocchio/multibody/joint/joint-revolute-unbounded-unaligned.hpp"
#include "pinocchio/algorithm/rnea.hpp"
#include "pinocchio/algorithm/joint-configuration.hpp"

#include "openarm_can.h"
#include "openarm_control_dual.h"

using Vec7 = Eigen::Matrix<double, 7, 1>;
using Vec3 = Eigen::Matrix<double, 3, 1>;

/* ---------- constants shared by both arms ----------
 * Plain aggregate tables, not Eigen objects: they need no dynamic
 * initialization, so they are valid whether or not the image runs C++
 * static constructors. The tunable gains arrive per arm as
 * struct DirectJointParams (direct_ipc.h). */
static const double kFc[7] = {0.306, 0.306, 0.40, 0.166, 0.050, 0.083, 0.172};
static const double kFk[7] = {28.417, 28.417, 29.065, 130.038, 151.771, 242.287, 7.888};
static const double kFv[7] = {0.063, 0.063, 0.604, 0.813, 0.029, 0.072, 0.084};
static const double kFo[7] = {0.088, 0.088, 0.008, -0.058, 0.005, 0.009, -0.059};
/* Motor torque range per joint (DM8009, DM4340, DM4310; OpenArmMotorsInit):
 * the gateway rejects a target beyond it, or beyond OPENARM_KD_MAX, as a
 * bad message, so the outputs are clamped to both. */
static const double kTauMax[7] = {54.0, 54.0, 28.0, 28.0, 10.0, 10.0, 10.0};

/* ---------- shared geometry (identical for both arms per v1.urdf) ---------- */

static const double kOrigin[7][3] = {
    {0.0, 0.0, 0.0625}, {-0.0301, 0.0, 0.06}, {0.0301, 0.0, 0.06625},
    {0.0, 0.0315, 0.15375}, {0.0, -0.0315, 0.0955},
    {0.0375, 0.0, 0.1205}, {-0.0375, 0.0, 0.0}
};
static const double kAxis[7][3] = {
    {0.0, 0.0, 1.0}, {-1.0, 0.0, 0.0}, {0.0, 0.0, 1.0},
    {0.0, 1.0, 0.0}, {0.0, 0.0, 1.0}, {1.0, 0.0, 0.0},
    {0.0, 1.0, 0.0} /* joint7: right arm; left arm overrides the sign */
};
static const double kCom[7][3] = {
    {0.0011467657911800769, 0.00003319987657026362, 0.05395284380736254},
    {0.00839629182351943, -0.000000020145102027597523, 0.03256649300522363},
    {-0.002104752099628911, 0.0005549085042607548, 0.09047470545721961},
    {-0.0029006831074562967, -0.03030575826634669, 0.06339637422196209},
    {-0.003049665024221911, 0.0008866902457326625, 0.043079803024980934},
    {-0.037136587005447405, 0.00033230528343419053, -0.00009498374522309838},
    {0.00006875510271106056, 0.01266175250761268, 0.06951945409987448}
};
static const double kMass[7] = {
    1.1416684646202298, 0.2775092746011571, 1.073863338202347,
    0.6348534566833373, 0.6156588026168502, 0.475202773187987,
    0.4659771327380578
};

/* ---------- per-arm URDF facts ---------- */

struct ArmParams {
    double rollSign;      /* +1: mount/joint2 roll = Rx(+pi/2), right arm */
    bool comYNeg[7];      /* negate com.y per link */
    double qMin[7], qMax[7];   /* POD: no dynamic init on this -nostdlib image */
    double axisJ7[3];
};

static const ArmParams kArmLeft = {
    -1.0,
    {true, true, false, false, true, true, true},
    {-3.490659, -3.316125, -1.570796, 0.0, -1.570796, -0.785398, -1.570796},
    {1.396263, 0.174533, 1.570796, 2.443461, 1.570796, 0.785398, 1.570796},
    {0.0, -1.0, 0.0}
};
static const ArmParams kArmRight = {
    +1.0,
    {false, false, false, false, false, false, false},
    {-1.396263, -0.174533, -1.570796, 0.0, -1.570796, -0.785398, -1.570796},
    {3.490659, 3.316125, 1.570796, 2.443461, 1.570796, 0.785398, 1.570796},
    {0.0, 1.0, 0.0}
};

/* ---------- gripper (Direct IPC slot 7, DM4310) ----------
 * Same virtual clutch as the joints, no gravity or friction feedforward.
 * q is the motor angle: about 0 = closed, about -1.1 = open on both arms
 * (official v1 gripper; ends measured at -0.05/-0.06 and -1.12/-1.15), so
 * the soft limits keep 0.05 rad off each end. Within them the holding
 * spring is <= kGripYield (0.15 Nm: with kd damping, 0.3 felt stiff at the
 * fingertips); past them it grows with the overshoot, and the gateway
 * clamps it again on its own (GRIP_SPRING_MAX). */
static const double kGripKp = 4.0;
static const double kGripKd = 0.1;
static const double kGripYield = 0.15;
static const double kGripMin = -1.05;
static const double kGripMax = -0.05;

/* ---------- controller state ---------- */

struct ArmController {
    pinocchio::Model model;
    pinocchio::Data data;
    Eigen::VectorXd qPin;      /* (cos, sin) per joint, sized with the model */
    bool modelReady;
    Vec7 qRef;
    bool initialized;
    double gripRef;
    bool gripInitialized;
};

/* [0]=left, [1]=right. Constructed on first use in raw storage, so the
 * result does not depend on whether the image runs C++ static constructors
 * (the sd3403 image links -nostdlib). ArmController has no base class,
 * virtuals or destructor side effects, and is never destroyed. */
alignas(ArmController) static unsigned char g_armStorage[2][sizeof(ArmController)];
static ArmController *g_arm[2];

static ArmController &Arm(int slot)
{
    if (g_arm[slot] == nullptr) {
        g_arm[slot] = new (g_armStorage[slot]) ArmController();
    }
    return *g_arm[slot];
}

static inline Vec7 Clamp(const Vec7 &x, const Vec7 &lo, const Vec7 &hi)
{
    return x.cwiseMax(lo).cwiseMin(hi);
}

static Eigen::Matrix3d RollX90(double sign)
{
    Eigen::Matrix3d R = Eigen::Matrix3d::Identity();
    if (sign > 0.0) {
        R << 1.0, 0.0, 0.0,
             0.0, 0.0, -1.0,
             0.0, 1.0, 0.0;
    } else {
        R << 1.0, 0.0, 0.0,
             0.0, 0.0, 1.0,
             0.0, -1.0, 0.0;
    }
    return R;
}

static void BuildArmModel(ArmController &a, const ArmParams &p)
{
    using pinocchio::Inertia;
    using pinocchio::JointModelRevoluteUnboundedUnaligned;

    const Eigen::Matrix3d roll = RollX90(p.rollSign);
    pinocchio::JointIndex parent = 0;
    for (int i = 0; i < 7; i++) {
        const Vec3 axis = (i == 6)
            ? Eigen::Map<const Vec3>(p.axisJ7)
            : (Vec3() << kAxis[i][0], kAxis[i][1], kAxis[i][2]).finished();
        /* base-mount roll (link0_joint) on j0, joint2's fixed origin roll
         * on j1 -- both use the arm's roll sign. */
        Eigen::Matrix3d R = Eigen::Matrix3d::Identity();
        if (i == 0 || i == 1) {
            R = roll;
        }
        pinocchio::SE3 placement(R,
            (Vec3() << kOrigin[i][0], kOrigin[i][1], kOrigin[i][2]).finished());
        Vec3 com(kCom[i][0], p.comYNeg[i] ? -kCom[i][1] : kCom[i][1], kCom[i][2]);
        Inertia inertia(kMass[i], com, Eigen::Matrix3d::Identity() * 1e-4);
        parent = a.model.addJoint(parent,
            JointModelRevoluteUnboundedUnaligned(axis), placement, "j");
        a.model.appendBodyToJoint(parent, inertia, pinocchio::SE3::Identity());
    }
    /* Explicit: a new Model copies its gravity from a dynamically initialized
     * static (pinocchio::Model::gravity981). */
    a.model.gravity.linear(Eigen::Vector3d(0.0, 0.0, -9.81));
    a.data = pinocchio::Data(a.model);
    a.qPin = pinocchio::neutral(a.model);
    a.modelReady = true;
}

/* Gripper slot of an already-built target: the joint path never touches
 * slot 7 (its loops stop at 7). Inactive or non-finite -> zero gain at the
 * measured angle, like an inactive joint; a non-finite joint frame (see
 * StepArm) drops the gripper to zero gain too. */
static void StepGripper(ArmController &a, const struct DirectRobotFeedbackMessage *f,
    struct DirectRobotTargetMessage *t, U32 activeJointMask)
{
    const U32 g = DIRECT_GRIPPER_SLOT;
    const double q = (double)f->q[g];
    const double dq = (double)f->dq[g];

    t->dqTarget[g] = 0.0F;
    t->tauFf[g] = 0.0F;
    if ((activeJointMask & (1U << g)) == 0U || !std::isfinite(q) || !std::isfinite(dq) ||
        !std::isfinite((double)f->tau[g])) {
        a.gripInitialized = false;
        t->qTarget[g] = std::isfinite(q) ? f->q[g] : 0.0F;
        t->kp[g] = 0.0F;
        t->kd[g] = 0.0F;
        return;
    }
    if (!a.gripInitialized) {
        a.gripRef = q;
        a.gripInitialized = true;
    }
    const double window = kGripYield / kGripKp;
    a.gripRef = std::min(std::max(a.gripRef, q - window), q + window);
    a.gripRef = std::min(std::max(a.gripRef, kGripMin), kGripMax);
    t->qTarget[g] = (float)a.gripRef;
    t->kp[g] = (float)kGripKp;
    t->kd[g] = (float)kGripKd;
}

static void StepArm(ArmController &a, const ArmParams &p,
    const struct DirectRobotFeedbackMessage *f,
    struct DirectRobotTargetMessage *t, U32 activeJointMask,
    const struct DirectJointParams &params)
{
    (void)memset(t, 0, sizeof(*t));
    if (!a.modelReady) {
        BuildArmModel(a, p);
    }

    const Vec7 q = Eigen::Map<const Eigen::Matrix<float, 7, 1>>(f->q).cast<double>();
    const Vec7 dq = Eigen::Map<const Eigen::Matrix<float, 7, 1>>(f->dq).cast<double>();
    const Vec7 tau = Eigen::Map<const Eigen::Matrix<float, 7, 1>>(f->tau).cast<double>();
    if (!q.allFinite() || !dq.allFinite() || !tau.allFinite()) {
        /* Defensive: the gateway decodes feedback from fixed-point fields,
         * so it is always finite. Should a non-finite frame still arrive,
         * send a valid zero-gain target (the motors go limp for this beat)
         * rather than the zeroed header: a target without
         * DIRECT_PROTOCOL_MAGIC is a BAD_MESSAGE, which is fatal and
         * disables both arms. */
        a.initialized = false;
        a.qRef.setZero();
        t->magic = DIRECT_PROTOCOL_MAGIC;
        t->version = DIRECT_PROTOCOL_VERSION;
        t->type = DIRECT_MSG_JOINT_TARGET;
        t->sequence = f->sampleSequence;
        t->generation = f->generation;
        StepGripper(a, f, t, activeJointMask & ~(1U << DIRECT_GRIPPER_SLOT));
        return;
    }
    if (!a.initialized) {
        a.qRef = q;
        a.initialized = true;
    }

    /* gravity via pinocchio; unbounded joints encode q as (cos, sin). */
    for (int i = 0; i < 7; i++) {
        a.qPin[2 * i] = std::cos(q[i]);
        a.qPin[2 * i + 1] = std::sin(q[i]);
    }
    const Vec7 gravity = pinocchio::computeGeneralizedGravity(a.model, a.data, a.qPin);

    t->magic = DIRECT_PROTOCOL_MAGIC;
    t->version = DIRECT_PROTOCOL_VERSION;
    t->type = DIRECT_MSG_JOINT_TARGET;
    t->sequence = f->sampleSequence;
    t->generation = f->generation;

    Vec7 scale, offset, jointKp, jointKd, yield, inertiaDamping, inertiaCapture;
    for (int i = 0; i < 7; i++) {
        scale[i] = (double)params.gravityScale[i] / DIRECT_PARAM_SCALE;
        offset[i] = (double)params.gravityOffset[i] / DIRECT_PARAM_SCALE;
        jointKp[i] = (double)params.jointKp[i] / DIRECT_PARAM_SCALE;
        jointKd[i] = (double)params.jointKd[i] / DIRECT_PARAM_SCALE;
        yield[i] = (double)params.yieldTorque[i] / DIRECT_PARAM_SCALE;
        inertiaDamping[i] = (double)params.inertiaDamping[i] / DIRECT_PARAM_SCALE;
        inertiaCapture[i] = (double)params.inertiaCapture[i] / DIRECT_PARAM_SCALE;
    }

    /* virtual clutch: clamp to the window around q first, then (only for
     * fast-moving joints) shift the window by the inertia capture; the
     * order matters when q_ref sits on the window edge. */
    const Vec7 window = yield.cwiseQuotient(jointKp);
    const Vec7 dqAbs = dq.cwiseAbs();
    a.qRef = Clamp(a.qRef, q - window, q + window);
    const Vec7 capture = (dqAbs.array() > 0.05)
        .select((dq.cwiseProduct(inertiaCapture)).cwiseMax(-window).cwiseMin(window),
                Vec7::Zero());
    a.qRef = Clamp(a.qRef, (q - window) + capture, (q + window) + capture);
    a.qRef = Clamp(a.qRef,
        Eigen::Map<const Vec7>(p.qMin), Eigen::Map<const Vec7>(p.qMax));

    /* friction feedforward: Coulomb (tanh of rate) + viscous + offset. */
    const Vec7 friction = 0.3 * (Eigen::Map<const Vec7>(kFc).cwiseProduct(
        (0.1 * Eigen::Map<const Vec7>(kFk).cwiseProduct(dq)).array().tanh().matrix())
        + Eigen::Map<const Vec7>(kFv).cwiseProduct(dq) + Eigen::Map<const Vec7>(kFo));
    const Vec7 tauMax = Eigen::Map<const Vec7>(kTauMax);
    const Vec7 tauFf = (scale.cwiseProduct(gravity) + offset + friction)
        .cwiseMax(-tauMax).cwiseMin(tauMax);
    const Vec7 kdEff = (jointKd + inertiaDamping.cwiseProduct(dqAbs))
        .cwiseMin(OPENARM_KD_MAX);

    for (int i = 0; i < 7; i++) {
        if ((activeJointMask & (1U << i)) == 0U) {
            t->qTarget[i] = f->q[i];
            t->dqTarget[i] = 0.0F;
            t->tauFf[i] = 0.0F;
            t->kp[i] = 0.0F;
            t->kd[i] = 0.0F;
            continue;
        }
        t->qTarget[i] = (float)a.qRef[i];
        t->dqTarget[i] = 0.0F;
        t->tauFf[i] = (float)tauFf[i];
        t->kp[i] = (float)jointKp[i];
        t->kd[i] = (float)kdEff[i];
    }
    StepGripper(a, f, t, activeJointMask);
}

extern "C" void OpenArmControlDualReset(void)
{
    for (int s = 0; s < 2; s++) {
        if (g_arm[s] == nullptr) {
            continue;
        }
        g_arm[s]->initialized = false;
        g_arm[s]->qRef.setZero();
        g_arm[s]->gripInitialized = false;
        g_arm[s]->gripRef = 0.0;
    }
}

extern "C" void OpenArmControlStepDual(const struct DirectRobotFeedbackMessage *fLeft,
    const struct DirectRobotFeedbackMessage *fRight,
    struct DirectRobotTargetMessage *tLeft,
    struct DirectRobotTargetMessage *tRight,
    U32 activeJointMaskLeft, U32 activeJointMaskRight,
    const struct DirectJointParams *paramsLeft,
    const struct DirectJointParams *paramsRight)
{
    StepArm(Arm(0), kArmLeft, fLeft, tLeft, activeJointMaskLeft, *paramsLeft);
    StepArm(Arm(1), kArmRight, fRight, tRight, activeJointMaskRight, *paramsRight);
}
