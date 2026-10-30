/* tsid_test.cc - TSID inverse-dynamics QP controller on UniProton sd3403.
 *
 * Builds the OpenArm model by hand (same constants as openarm_gravity.c),
 * runs an InverseDynamicsFormulationAccFW with a joint-posture task solved
 * by the eiquadprog backend, and cross-checks the resulting joint torques
 * against the C gravity reference at zero velocity/acceleration targets.
 * Timed with CNTPCT_EL0 at -O2.
 */
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <locale.h>
#include <memory>

#include <Eigen/Core>

#include "pinocchio/spatial/se3.hpp"
#include "pinocchio/multibody/model.hpp"
#include "pinocchio/multibody/data.hpp"
#include "pinocchio/multibody/joint/joint-revolute-unbounded-unaligned.hpp"
#include "pinocchio/algorithm/center-of-mass.hpp"
#include "pinocchio/algorithm/joint-configuration.hpp"

#include "tsid/robots/robot-wrapper.hpp"
#include "tsid/formulations/inverse-dynamics-formulation-acc-force.hpp"
#include "tsid/tasks/task-joint-posture.hpp"
#include "tsid/trajectories/trajectory-base.hpp"
#include "tsid/solvers/solver-HQP-eiquadprog.hpp"

#include "openarm_gravity.h"
#include "prt_task.h"
#include "prt_mem.h"

extern "C" {

/* Bare-metal stand-ins (same rationale as the other algo tests). */
void *__dso_handle = (void *)&__dso_handle;
const char *__strftime_fmt_1(char (*s)[100], size_t *l, int f, const struct tm *tm,
    locale_t loc, int pad)
{
    (void)s; (void)l; (void)f; (void)tm; (void)loc; (void)pad;
    return 0;
}

} /* extern "C" */

/* bsp/timer.c: the board counter runs at 24 MHz, but the firmware leaves
 * CNTFRQ_EL0 at 25 MHz, so reading it would make every time 4% short. */
extern "C" unsigned long long GetGenericTimerFreq(void);

static inline uint64_t CntFreq(void)
{
    return GetGenericTimerFreq();
}

static inline uint64_t CntTicks(void)
{
    uint64_t v;
    __asm volatile("dsb sy\nmrs %0, cntpct_el0" : "=r"(v));
    return v;
}

static uint64_t g_cntFreq;

static double TicksToUs(uint64_t ticks)
{
    return (double)ticks * 1000000.0 / (double)g_cntFreq;
}

static uint32_t Lcg(uint32_t *state)
{
    *state = *state * 1664525u + 1013904223u;
    return *state;
}

static double LcgDouble(uint32_t *state)
{
    return ((double)(Lcg(state) >> 8) / 16777216.0) - 0.5;
}

/* URDF-derived constants, identical to openarm_gravity.c (same as pinocchio-test). */
static const double kOrigin[7][3] = {
    {0.0, 0.0, 0.0625}, {-0.0301, 0.0, 0.06}, {0.0301, 0.0, 0.06625},
    {0.0, 0.0315, 0.15375}, {0.0, -0.0315, 0.0955},
    {0.0375, 0.0, 0.1205}, {-0.0375, 0.0, 0.0}
};
static const double kAxis[7][3] = {
    {0.0, 0.0, 1.0}, {-1.0, 0.0, 0.0}, {0.0, 0.0, 1.0},
    {0.0, 1.0, 0.0}, {0.0, 0.0, 1.0}, {1.0, 0.0, 0.0},
    {0.0, 1.0, 0.0}
};
static const double kCom[7][3] = {
    {0.0011467657911800769, 0.00003319987657026362, 0.05395284380736254},
    {0.00839629182351943, -0.000000020145102027597523, 0.03256649300522363},
    {-0.002104752099628911, 0.0005549085042607548, 0.09047470545721961},
    {-0.0029006831074562967, -0.03030575826634669, 0.06339637422196209},
    {-0.003049665024221911, 0.0008866902457326625, 0.043079803024980934},
    {-0.037136587005447405, 0.00033230528343419053, -0.00009498374522309838},
    {0.00006875510271056056, 0.01266175250761268, 0.06951945409987448}
};
static const double kMass[7] = {
    1.1416684646202298, 0.2775092746011571, 1.073863338202347,
    0.6348534566833373, 0.6156588026168502, 0.475202773187987,
    0.4659771327380578
};

static void BuildOpenArm(pinocchio::Model &model)
{
    using pinocchio::Inertia;
    using pinocchio::JointModelRevoluteUnboundedUnaligned;
    typedef Eigen::Matrix<double, 3, 1> V3;

    Eigen::Matrix3d eyeI = Eigen::Matrix3d::Identity() * 1e-4;
    pinocchio::JointIndex parent = 0;
    for (int i = 0; i < 7; i++) {
        JointModelRevoluteUnboundedUnaligned jx(
            (V3() << kAxis[i][0], kAxis[i][1], kAxis[i][2]).finished());
        Eigen::Matrix3d R = Eigen::Matrix3d::Identity();
        /* base-mount roll (link0_joint) folds into j0's placement, joint2's
         * fixed origin roll into j2's -- mirrors openarm_gravity.c, where r
         * starts at the roll and gains a second one after link 1. */
        if (i == 0 || i == 1) {
            R << 1.0, 0.0, 0.0,
                 0.0, 0.0, -1.0,
                 0.0, 1.0, 0.0;
        }
        pinocchio::SE3 placement(R,
            (V3() << kOrigin[i][0], kOrigin[i][1], kOrigin[i][2]).finished());
        Inertia inertia(kMass[i],
            (V3() << kCom[i][0], kCom[i][1], kCom[i][2]).finished(),
            eyeI);
        parent = model.addJoint(parent, jx, placement, "j");
        model.appendBodyToJoint(parent, inertia, pinocchio::SE3::Identity());
    }
    model.gravity.linear(Eigen::Vector3d(0.0, 0.0, -9.81));
}

#define BENCH_REPS 200

static void RunAll(void)
{
    using namespace tsid;

    pinocchio::Model model;
    BuildOpenArm(model);

    robots::RobotWrapper robot(model, robots::RobotWrapper::FIXED_BASE_SYSTEM, false);
    auto tsidCtrl = std::make_shared<InverseDynamicsFormulationAccForce>(
        "tsid-openarm", robot, false);

    /* current state = q0 */
    Eigen::VectorXd q0 = pinocchio::neutral(model);
    Eigen::VectorXd v0 = Eigen::VectorXd::Zero(model.nv);
    double t = 0.0;
    tsidCtrl->computeProblemData(t, q0, v0);

    auto posture = std::make_shared<tasks::TaskJointPosture>("posture", robot);
    /* addMotionTask evaluates the task immediately, so the initial reference
     * must be a valid configuration: a zero-filled sample encodes (cos,sin)=
     * (0,0) on unbounded joints and derails the kinematics. */
    trajectories::TrajectorySample init(model.nq, model.nv);
    init.setValue(pinocchio::neutral(model));
    posture->setReference(init);
    tsidCtrl->addMotionTask(*posture, 1e6, 1, 0.0);

    solvers::SolverHQuadProg solver("qpsolver");

    /* posture target = random pose, zero vel/acc: with the dynamics
     * constraint tau = M*0 + g(q) the resulting torques equal gravity.
     * Revolute-unbounded joints encode q as (cos, sin) pairs, so the
     * reference must be packed that way and the C reference takes the
     * plain angles. */
    uint32_t seed = 20260915u;
    double angRef[7];
    Eigen::VectorXd qRef = pinocchio::neutral(model);
    for (int i = 0; i < 7; i++) {
        angRef[i] = LcgDouble(&seed) * 3.0;
        qRef[2 * i] = cos(angRef[i]);
        qRef[2 * i + 1] = sin(angRef[i]);
    }
    trajectories::TrajectorySample sample(model.nq, model.nv);
    sample.setValue(qRef);
    sample.setDerivative(Eigen::VectorXd::Zero(model.nv));
    sample.setSecondDerivative(Eigen::VectorXd::Zero(model.nv));
    posture->setReference(sample);
    tsidCtrl->computeProblemData(t, qRef, Eigen::VectorXd::Zero(model.nv));

    const solvers::HQPData &hqp = tsidCtrl->computeProblemData(t, qRef,
        Eigen::VectorXd::Zero(model.nv));
    solvers::HQPOutput output = solver.solve(hqp);
    /* SolverHQuadProg leaves HQPOutput::iterations unset; print status only. */
    printf("[tsid] status=%d\n", (int)output.status);

    /* x = [dv, f]: actuator torques are back-computed, not a QP variable. */
    Eigen::VectorXd tau = tsidCtrl->getActuatorForces(output);
    double tauC[7];
    OpenArmGravityRight(angRef, tauC);
    double maxDiff = 0.0;
    for (int i = 0; i < 7; i++) {
        double d = fabs(tau[i] - tauC[i]);
        if (d > maxDiff) { maxDiff = d; }
    }
    printf("[tsid] tau vs C gravity max|diff| = %.6f Nm\n", maxDiff);

    /* timed repeats: full problem build + solve */
    volatile double sink = 0.0;
    uint64_t best = ~0ull;
    for (int round = 0; round < 3; round++) {
        Eigen::VectorXd tr = Eigen::VectorXd::Zero(model.nv);
        uint64_t t0 = CntTicks();
        for (int n = 0; n < BENCH_REPS; n++) {
            const solvers::HQPData &hh = tsidCtrl->computeProblemData(t, qRef,
                Eigen::VectorXd::Zero(model.nv));
            solvers::HQPOutput out = solver.solve(hh);
            tr += tsidCtrl->getActuatorForces(out);
        }
        uint64_t t1 = CntTicks();
        sink += tr[0];
        if (t1 - t0 < best) { best = t1 - t0; }
    }
    printf("[tsid] posture step (problem+solve) : %8.2f us/op\n", TicksToUs(best) / BENCH_REPS);
    printf("[tsid] sink=%f counter=%llu Hz\n", (double)sink, (unsigned long long)g_cntFreq);
}

extern "C" {

/* tsid/pinocchio template call chains need far more stack than the 32 KiB
 * default test task; run the suite in its own 256 KiB-stack task. */
static void AlgoTaskEntry(void)
{
    RunAll();
    printf("tsid test end\n");
}

void Init(uintptr_t param1, uintptr_t param2, uintptr_t param3, uintptr_t param4)
{
    (void)param1; (void)param2; (void)param3; (void)param4;
    g_cntFreq = CntFreq();
    printf("tsid test start: counter=%llu Hz\n", (unsigned long long)g_cntFreq);

    static char algoTaskName[] = "AlgoTest";
    struct TskInitParam param = {0};
    TskHandle algoTask = 0;
    param.stackAddr = (uintptr_t)PRT_MemAllocAlign(0, OS_MEM_DEFAULT_FSC_PT,
        0x40000, MEM_ADDR_ALIGN_016);
    param.taskEntry = (TskEntryFunc)AlgoTaskEntry;
    param.taskPrio = 24;
    param.name = algoTaskName;
    param.stackSize = 0x40000;
    if (PRT_TaskCreate(&algoTask, &param) != 0) {
        printf("tsid task create failed\n");
        return;
    }
    PRT_TaskResume(algoTask);
}

} /* extern "C" */
