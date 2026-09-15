/* pinocchio_test.cc - pinocchio 2.6.21 rigid-body dynamics on UniProton sd3403.
 *
 * The OpenArm right arm is rebuilt by hand with the pinocchio Model API using
 * the same URDF-derived constants as demos/sd3403/apps/openamp/openarm_gravity.c
 * (link origins, joint axes incl. the fixed Rx(+90 deg) rotations of the base
 * mount and joint 2, masses and coms). computeGeneralizedGravity and rnea(q,0,0) are cross-checked against
 * the C reference, then timed with CNTPCT_EL0. Compiled with -O2.
 */
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <locale.h>

#include <Eigen/Core>

#include "pinocchio/spatial/se3.hpp"
#include "pinocchio/multibody/model.hpp"
#include "pinocchio/multibody/data.hpp"
#include "pinocchio/algorithm/joint-configuration.hpp"
#include "pinocchio/multibody/joint/joint-revolute-unbounded-unaligned.hpp"
#include "pinocchio/algorithm/center-of-mass.hpp"
#include "pinocchio/algorithm/rnea.hpp"

#include "openarm_gravity.h"

extern "C" {

/* Bare-metal stand-ins (same rationale as the eigen/eiquadprog tests). */
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

/* URDF-derived constants, identical to openarm_gravity.c. */
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

    /* Same chain as OpenArmGravityRight() in openarm_gravity.c, the C
     * reference this test is checked against. There r starts at Rx(+90 deg)
     * (the right arm's base mount, URDF rpy 1.5708 0 0), p += r * origin[i],
     * and for i == 1 r = r * Rx(+90 deg) (joint 2's fixed origin rotation).
     * Here the placements of joints i = 0 and i = 1 carry Rx(+90 deg), all
     * others are SE3(I, origin[i]). The base rotation then acts after joint
     * 0's translation instead of before it, which only shifts the whole arm
     * and leaves the gravity torques unchanged.
     */
    Eigen::Matrix3d eyeI = Eigen::Matrix3d::Identity() * 1e-4; /* rotational inertia is irrelevant for gravity */
    pinocchio::JointIndex parent = 0; /* universe */
    for (int i = 0; i < 7; i++) {
        JointModelRevoluteUnboundedUnaligned jx((V3() << kAxis[i][0], kAxis[i][1], kAxis[i][2]).finished());
        Eigen::Matrix3d R = Eigen::Matrix3d::Identity();
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

#define BENCH_REPS 1000

static void RunAll(void)
{
    pinocchio::Model model;
    BuildOpenArm(model);
    pinocchio::Data data(model);

    printf("[pin] model: nq=%d nv=%d njoints=%d\n", (int)model.nq, (int)model.nv, (int)model.njoints);

    uint32_t seed = 20260915u;
    Eigen::VectorXd q = pinocchio::neutral(model);
    double ang[7];
    double maxDiffG = 0.0;
    double maxDiffR = 0.0;
    double tauC[7];

    /* Revolute-unbounded joints encode q as interleaved (cos, sin) pairs;
     * the C reference consumes plain angles. */
    for (int pose = 0; pose < 5; pose++) {
        for (int i = 0; i < 7; i++) {
            ang[i] = LcgDouble(&seed) * 3.0;
            q[2 * i] = cos(ang[i]);
            q[2 * i + 1] = sin(ang[i]);
        }
        Eigen::VectorXd g = pinocchio::computeGeneralizedGravity(model, data, q);
        OpenArmGravityRight(ang, tauC);
        Eigen::VectorXd r = pinocchio::rnea(model, data, q,
            Eigen::VectorXd::Zero(model.nv), Eigen::VectorXd::Zero(model.nv));
        for (int i = 0; i < 7; i++) {
            double dg = fabs(g[i] - tauC[i]);
            double dr = fabs(r[i] - tauC[i]);
            if (dg > maxDiffG) { maxDiffG = dg; }
            if (dr > maxDiffR) { maxDiffR = dr; }
        }
    }
    printf("[pin] gravity vs C max|diff| = %.3e Nm (5 poses)\n", maxDiffG);
    printf("[pin] rnea(0,0) vs C max|diff| = %.3e Nm (5 poses)\n", maxDiffR);

    for (int i = 0; i < 7; i++) {
        ang[i] = LcgDouble(&seed) * 3.0;
        q[2 * i] = cos(ang[i]);
        q[2 * i + 1] = sin(ang[i]);
    }

    volatile double sink = 0.0;
    uint64_t best = ~0ull;
    for (int round = 0; round < 3; round++) {
        Eigen::VectorXd g = Eigen::VectorXd::Zero(model.nv);
        uint64_t t0 = CntTicks();
        for (int n = 0; n < BENCH_REPS; n++) {
            g += pinocchio::computeGeneralizedGravity(model, data, q);
        }
        uint64_t t1 = CntTicks();
        sink += g[0];
        if (t1 - t0 < best) { best = t1 - t0; }
    }
    printf("[pin] gravity  : %8.2f us/op\n", TicksToUs(best) / BENCH_REPS);

    best = ~0ull;
    for (int round = 0; round < 3; round++) {
        Eigen::VectorXd r = Eigen::VectorXd::Zero(model.nv);
        Eigen::VectorXd v = Eigen::VectorXd::Zero(model.nv);
        Eigen::VectorXd a = Eigen::VectorXd::Constant(model.nv, 1.0);
        uint64_t t0 = CntTicks();
        for (int n = 0; n < BENCH_REPS; n++) {
            r += pinocchio::rnea(model, data, q, v, a);
        }
        uint64_t t1 = CntTicks();
        sink += r[0];
        if (t1 - t0 < best) { best = t1 - t0; }
    }
    printf("[pin] rnea(v=0,a=1): %8.2f us/op\n", TicksToUs(best) / BENCH_REPS);
    printf("[pin] sink=%f counter=%llu Hz\n", (double)sink, (unsigned long long)g_cntFreq);
}

extern "C" {

void Init(uintptr_t param1, uintptr_t param2, uintptr_t param3, uintptr_t param4)
{
    (void)param1; (void)param2; (void)param3; (void)param4;
    g_cntFreq = CntFreq();
    printf("pinocchio test start: counter=%llu Hz\n", (unsigned long long)g_cntFreq);
    RunAll();
    printf("pinocchio test end\n");
}

} /* extern "C" */
