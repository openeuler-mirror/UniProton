/* proxsuite_test.cc - proxsuite dense QP solver on UniProton sd3403.
 *
 * Solves the same 7-variable QP as the eiquadprog test with a known optimum
 * (one active box constraint on joint 0), cross-checks both solvers against
 * the analytic solution, then times proxsuite with CNTPCT_EL0 at -O2.
 */
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <locale.h>

#include <Eigen/Core>
#include <proxsuite/proxqp/dense/dense.hpp>
#include <eiquadprog/eiquadprog-fast.hpp>

typedef Eigen::Matrix<double, 7, 1> Vec7;
typedef Eigen::Matrix<double, 7, 7> Mat7;
typedef Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic> MatX;
typedef Eigen::Matrix<double, Eigen::Dynamic, 1> VecX;

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

#define BENCH_REPS 1000

static void SolveBench(void)
{
    uint32_t seed = 20260915u;
    Mat7 a = Mat7::Zero();
    for (int r = 0; r < 7; r++) {
        for (int c = 0; c < 7; c++) {
            a(r, c) = LcgDouble(&seed);
        }
    }
    Mat7 H = a * a.transpose() + Mat7::Identity(); /* spd */
    Vec7 xStar = Vec7::Zero();
    for (int i = 0; i < 7; i++) {
        xStar[i] = LcgDouble(&seed) * 2.0;
    }
    VecX g = (-H * xStar).eval(); /* unconstrained optimum = xStar */

    /* inequality i: x[i] <= bound_i (joint 0 active at xStar[0]-0.1). */
    const int nIneq = 7;
    VecX bound(nIneq);
    for (int i = 0; i < nIneq; i++) {
        bound[i] = (i == 0) ? (xStar[0] - 0.1) : (xStar[i] + 1.0);
    }
    /* proxsuite form l <= C x <= u with C = -I: -x >= -bound. */
    MatX C = -MatX::Identity(nIneq, 7);
    VecX l = -bound;
    VecX u = VecX::Constant(nIneq, 1e10);
    MatX A = MatX::Zero(0, 7);
    VecX b = VecX::Zero(0);

    /* Only joint 0 is active: x = xStar - 0.1 * h / h[0], h = H^-1 e0. */
    Vec7 h = H.llt().solve(Vec7::Unit(0));
    Vec7 xExpect = xStar - (0.1 / h[0]) * h;

    /* --- proxsuite dense QP --- */
    proxsuite::proxqp::dense::QP<double> qp(7, 0, nIneq);
    qp.init(H, g, A, b, C, l, u);
    qp.solve();
    double maxErrProx = 0.0;
    for (int i = 0; i < 7; i++) {
        double d = fabs(qp.results.x[i] - xExpect[i]);
        if (d > maxErrProx) { maxErrProx = d; }
    }
    printf("[prox] proxsuite iters=%d max|x-x*| = %.3e\n",
        (int)qp.results.info.iter, maxErrProx);

    /* --- eiquadprog on the same problem (CI x + ci0 >= 0 form: bound - x >= 0) --- */
    MatX CI = C;
    VecX ci0 = bound;
    eiquadprog::solvers::EiquadprogFast eq;
    eq.reset(7, 0, nIneq);
    VecX x = VecX::Zero(7);
    (void)eq.solve_quadprog(H, g, A, b, CI, ci0, x);
    double maxErrEq = 0.0;
    double maxDiff = 0.0;
    for (int i = 0; i < 7; i++) {
        double d = fabs(x[i] - xExpect[i]);
        if (d > maxErrEq) { maxErrEq = d; }
        d = fabs(qp.results.x[i] - x[i]);
        if (d > maxDiff) { maxDiff = d; }
    }
    printf("[prox] eiquadprog  max|x-x*| = %.3e\n", maxErrEq);
    printf("[prox] proxsuite vs eiquadprog max|dx| = %.3e\n", maxDiff);
    /* proxsuite stops at its default eps_abs = 1e-5 residual; H >= I bounds
     * the solution error by the residual, so 1e-4 leaves margin. */
    printf("[prox] same-problem check: %s\n",
        (maxErrProx < 1e-4 && maxErrEq < 1e-9) ? "PASS" : "FAIL");

    /* --- timed repeats --- */
    volatile double sink = 0.0;
    uint64_t best = ~0ull;
    for (int round = 0; round < 3; round++) {
        uint64_t t0 = CntTicks();
        for (int n = 0; n < BENCH_REPS; n++) {
            qp.init(H, g, A, b, C, l, u);
            qp.solve();
        }
        uint64_t t1 = CntTicks();
        sink += qp.results.x[0];
        if (t1 - t0 < best) { best = t1 - t0; }
    }
    printf("[prox] proxsuite solve : %8.2f us/op (best of 3 x %d)\n",
        TicksToUs(best) / BENCH_REPS, BENCH_REPS);
    printf("[prox] sink=%f counter=%llu Hz\n", (double)sink, (unsigned long long)g_cntFreq);
}

extern "C" {

void Init(uintptr_t param1, uintptr_t param2, uintptr_t param3, uintptr_t param4)
{
    (void)param1; (void)param2; (void)param3; (void)param4;
    g_cntFreq = CntFreq();
    printf("proxsuite test start: counter=%llu Hz\n", (unsigned long long)g_cntFreq);
    SolveBench();
    printf("proxsuite test end\n");
}

} /* extern "C" */
