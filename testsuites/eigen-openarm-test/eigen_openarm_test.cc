/* eigen_openarm_test.cc - Eigen benchmark and gravity-model cross-check on UniProton sd3403.
 *
 * Benchmarks (CNTPCT_EL0 ns timing, compiled with -O2):
 *   1) 7x7 * 7x7 matrix multiply
 *   2) 7x7 Cholesky (LLT) decomposition
 *   3) 7x7 inverse (LU)
 *   4) Eigen re-implementation of OpenArmGravityRight, cross-checked
 *      against the C reference in demos/sd3403/apps/openamp/openarm_gravity.c.
 */
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <math.h>

#include <Eigen/Dense>

#include "openarm_gravity.h"

extern "C" {

/* Bare-metal stand-in for the crtbegin-provided DSO handle. libstdc++'s
 * eh_globals static init registers destructors via __cxa_atexit(.., &__dso_handle). */
void *__dso_handle = (void *)&__dso_handle;

} /* extern "C" */

typedef Eigen::Matrix<double, 7, 7> Mat7;
typedef Eigen::Matrix<double, 7, 1> Vec7;

/* ---------- timing via ARMv8 generic timer ---------- */

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

/* ---------- deterministic pseudo random (reproducible runs) ---------- */

static uint32_t Lcg(uint32_t *state)
{
    *state = *state * 1664525u + 1013904223u;
    return *state;
}

static double LcgDouble(uint32_t *state)
{
    return ((double)(Lcg(state) >> 8) / 16777216.0) - 0.5;
}

/* ---------- Eigen gravity model (mirrors openarm_gravity.c) ---------- */

typedef Eigen::Matrix<double, 3, 1> Vec3;
typedef Eigen::Matrix<double, 3, 3> Mat3;

/* Plain arrays keep the table constant-initialized: Eigen members here would
 * need a dynamic initializer from .init_array before Init() runs. */
struct Link {
    double origin[3];
    double axis[3];
    double com[3];
    double mass;
};

typedef Eigen::Map<const Vec3> CVec3;

static const Link kLinks[7] = {
    {{0.0, 0.0, 0.0625}, {0.0, 0.0, 1.0}, {0.0011467657911800769, 0.00003319987657026362, 0.05395284380736254}, 1.1416684646202298},
    {{-0.0301, 0.0, 0.06}, {-1.0, 0.0, 0.0}, {0.00839629182351943, -0.000000020145102027597523, 0.03256649300522363}, 0.2775092746011571},
    {{0.0301, 0.0, 0.06625}, {0.0, 0.0, 1.0}, {-0.002104752099628911, 0.0005549085042607548, 0.09047470545721961}, 1.073863338202347},
    {{0.0, 0.0315, 0.15375}, {0.0, 1.0, 0.0}, {-0.0029006831074562967, -0.03030575826634669, 0.06339637422196209}, 0.6348534566833373},
    {{0.0, -0.0315, 0.0955}, {0.0, 0.0, 1.0}, {-0.003049665024221911, 0.0008866902457326625, 0.043079803024980934}, 0.6156588026168502},
    {{0.0375, 0.0, 0.1205}, {1.0, 0.0, 0.0}, {-0.037136587005447405, 0.00033230528343419053, -0.00009498374522309838}, 0.475202773187987},
    {{-0.0375, 0.0, 0.0}, {0.0, 1.0, 0.0}, {0.00006875510271056056, 0.01266175250761268, 0.06951945409987448}, 0.4659771327380578},
};

static Mat3 Joint2Roll(void)
{
    /* Built at call time: a file-scope Eigen matrix would also need a
     * dynamic initializer. */
    Mat3 r;
    r << 1.0, 0.0, 0.0,
         0.0, 0.0, -1.0,
         0.0, 1.0, 0.0;
    return r;
}

static Vec7 GravityEigen(const Vec7 &q)
{
    /* The right arm's base mount (URDF rpy 1.5708 0 0) is Rx(+90 deg), the
     * same rotation as joint2's origin; like OpenArmGravityRight in the C
     * reference, start from it and apply it again at joint2. */
    const Mat3 joint2Roll = Joint2Roll();
    Mat3 r = joint2Roll;
    Vec3 p = Vec3::Zero();
    Vec3 jp[7];
    Vec3 ja[7];
    Vec3 cp[7];
    Vec7 tau;

    for (int i = 0; i < 7; i++) {
        p += r * CVec3(kLinks[i].origin);
        if (i == 1) {
            r = r * joint2Roll;
        }
        jp[i] = p;
        ja[i] = r * CVec3(kLinks[i].axis);
        r = r * Eigen::AngleAxisd(q[i], CVec3(kLinks[i].axis)).matrix();
        cp[i] = p + r * CVec3(kLinks[i].com);
    }
    for (int i = 0; i < 7; i++) {
        tau[i] = 0.0;
        for (int l = i; l < 7; l++) {
            tau[i] += ja[i].dot((cp[l] - jp[i]).cross(Vec3(0.0, 0.0, kLinks[l].mass * 9.81)));
        }
    }
    return tau;
}

/* ---------- benches ---------- */

#define BENCH_REPS 2000

static void BenchAll(void)
{
    uint32_t seed = 20260915u;
    Mat7 a = Mat7::Zero();
    for (int r = 0; r < 7; r++) {
        for (int c = 0; c < 7; c++) {
            a(r, c) = LcgDouble(&seed);
        }
    }
    Mat7 spd = a * a.transpose() + Mat7::Identity(); /* symmetric positive definite for LLT */
    Vec7 q = Vec7::Zero();
    for (int i = 0; i < 7; i++) {
        q[i] = LcgDouble(&seed) * 3.0;
    }

    volatile double sink = 0.0;
    uint64_t t0;
    uint64_t t1;
    uint64_t best;

    /* 1) 7x7 matrix multiply */
    best = ~0ull;
    for (int round = 0; round < 3; round++) {
        Mat7 b = Mat7::Zero();
        t0 = CntTicks();
        for (int n = 0; n < BENCH_REPS; n++) {
            b = a * b + spd; /* result depends on inputs, prevents dead-code removal */
        }
        t1 = CntTicks();
        sink += b(0, 0);
        if (t1 - t0 < best) {
            best = t1 - t0;
        }
    }
    printf("[eigen] mat7 mul      : %8.2f us/op (best of 3 x %d)\n",
        TicksToUs(best) / BENCH_REPS, BENCH_REPS);

    /* 2) 7x7 Cholesky (LLT) */
    best = ~0ull;
    for (int round = 0; round < 3; round++) {
        Mat7 l = Mat7::Zero();
        t0 = CntTicks();
        for (int n = 0; n < BENCH_REPS; n++) {
            l = spd.llt().matrixL();
        }
        t1 = CntTicks();
        sink += l(6, 6);
        if (t1 - t0 < best) {
            best = t1 - t0;
        }
    }
    printf("[eigen] mat7 llt      : %8.2f us/op\n", TicksToUs(best) / BENCH_REPS);

    /* 3) 7x7 inverse (LU) */
    best = ~0ull;
    for (int round = 0; round < 3; round++) {
        Mat7 inv = Mat7::Zero();
        t0 = CntTicks();
        for (int n = 0; n < BENCH_REPS; n++) {
            inv = spd.inverse();
        }
        t1 = CntTicks();
        sink += inv(0, 6);
        if (t1 - t0 < best) {
            best = t1 - t0;
        }
    }
    printf("[eigen] mat7 inverse  : %8.2f us/op\n", TicksToUs(best) / BENCH_REPS);

    /* 4) Eigen gravity model vs C gravity model */
    double tauC[7];
    best = ~0ull;
    for (int round = 0; round < 3; round++) {
        Vec7 gE = Vec7::Zero();
        t0 = CntTicks();
        for (int n = 0; n < BENCH_REPS; n++) {
            gE += GravityEigen(q); /* accumulate to keep the call alive */
        }
        t1 = CntTicks();
        sink += gE[0];
        if (t1 - t0 < best) {
            best = t1 - t0;
        }
    }
    printf("[eigen] gravity eigen : %8.2f us/op\n", TicksToUs(best) / BENCH_REPS);

    best = ~0ull;
    for (int round = 0; round < 3; round++) {
        double sum = 0.0;
        t0 = CntTicks();
        for (int n = 0; n < BENCH_REPS; n++) {
            OpenArmGravityRight(q.data(), tauC);
            sum += tauC[0];
        }
        t1 = CntTicks();
        sink += sum;
        if (t1 - t0 < best) {
            best = t1 - t0;
        }
    }
    printf("[eigen] gravity C ref : %8.2f us/op\n", TicksToUs(best) / BENCH_REPS);

    /* cross check on several poses */
    double maxDiff = 0.0;
    for (int pose = 0; pose < 5; pose++) {
        for (int i = 0; i < 7; i++) {
            q[i] = LcgDouble(&seed) * 3.0;
        }
        OpenArmGravityRight(q.data(), tauC);
        Vec7 tauE = GravityEigen(q);
        for (int i = 0; i < 7; i++) {
            double d = fabs(tauE[i] - tauC[i]);
            if (d > maxDiff) {
                maxDiff = d;
            }
        }
    }
    printf("[eigen] gravity cross-check: %s\n", (maxDiff < 1e-9) ? "PASS" : "FAIL");
    printf("[eigen] gravity cross-check max|eigen-C| = %.3e Nm (5 poses)\n", maxDiff);
    printf("[eigen] sink=%f counter=%llu Hz\n", (double)sink, (unsigned long long)g_cntFreq);
}

extern "C" {

void Init(uintptr_t param1, uintptr_t param2, uintptr_t param3, uintptr_t param4)
{
    (void)param1;
    (void)param2;
    (void)param3;
    (void)param4;

    g_cntFreq = CntFreq();
    printf("eigen test start: counter=%llu Hz\n", (unsigned long long)g_cntFreq);
    BenchAll();
    printf("eigen test end\n");
}

} /* extern "C" */
