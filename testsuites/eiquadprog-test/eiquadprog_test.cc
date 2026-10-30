/* eiquadprog_test.cc - eiquadprog QP solver benchmark on UniProton sd3403.
 *
 * Solves a 7-variable QP with a known solution:
 *   min. x' H x + 2 g0' x   (=  (x-x*)' H (x-x*) + const)
 *   s.t. CI x + ci0 >= 0     (one active box constraint on joint 0)
 * Compiled with -O2, timed with CNTPCT_EL0.
 */
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <locale.h>

#include <Eigen/Core>
#include <eiquadprog/eiquadprog-fast.hpp>

typedef Eigen::Matrix<double, 7, 1> Vec7;
typedef Eigen::Matrix<double, 7, 7> Mat7;
typedef Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic> MatX;
typedef Eigen::Matrix<double, Eigen::Dynamic, 1> VecX;

extern "C" {

/* Bare-metal stand-in for the crtbegin-provided DSO handle. */
void *__dso_handle = (void *)&__dso_handle;

/* musl's wcsftime.o calls this helper, which musl's strftime.c defines; sd3403
 * builds the litelibc strftime instead (no CONFIG_OS_OPTION_LOCALE). NULL is
 * the helper's own result for an unsupported conversion: __wcsftime_l() then
 * stops and returns 0, so a wcsftime() call fails cleanly. The test never
 * formats times. */
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

#define BENCH_REPS 2000

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
    VecX g0 = (-H * xStar).eval(); /* unconstrained optimum = xStar */

    /* inequality i: -x[i] + bound_i >= 0. joint 0 bound is active (xStar[0]-0.1),
     * the rest are loose upper bounds (xStar[i]+1). */
    const int nIneq = 7;
    MatX CI = MatX::Zero(nIneq, 7);
    VecX ci0 = VecX::Zero(nIneq);
    for (int i = 0; i < nIneq; i++) {
        CI(i, i) = -1.0;
        ci0[i] = (i == 0) ? (xStar[0] - 0.1) : (xStar[i] + 1.0);
    }
    MatX CE = MatX::Zero(0, 7);
    VecX ce0 = VecX::Zero(0);

    eiquadprog::solvers::EiquadprogFast qp;
    qp.reset(7, 0, nIneq);
    VecX x = VecX::Zero(7);

    eiquadprog::solvers::EiquadprogFast_status st = qp.solve_quadprog(H, g0, CE, ce0, CI, ci0, x);
    printf("[eq] status=%d obj=%.6f iters=%d activeSet=%d\n",
        (int)st, qp.getObjValue(), qp.getIteratios(), (int)qp.getActiveSetSize());

    /* Only joint 0 is active, so KKT gives H (x - xStar) = -lambda e0 with
     * x[0] = xStar[0] - 0.1: x = xStar - 0.1 * h / h[0], h = H^-1 e0. H is not
     * diagonal, so the other joints move too. */
    Vec7 h = H.llt().solve(Vec7::Unit(0));
    Vec7 xExpect = xStar - (0.1 / h[0]) * h;
    double maxErr = 0.0;
    for (int i = 0; i < 7; i++) {
        double d = fabs(x[i] - xExpect[i]);
        if (d > maxErr) {
            maxErr = d;
        }
    }
    bool pass = (st == eiquadprog::solvers::EIQUADPROG_FAST_OPTIMAL) && (maxErr < 1e-9);
    printf("[eq] known-solution check: %s\n", pass ? "PASS" : "FAIL");
    printf("[eq] known-solution max|x-x*| = %.3e\n", maxErr);

    /* timed repeats on the same problem */
    volatile double sink = 0.0;
    uint64_t best = ~0ull;
    for (int round = 0; round < 3; round++) {
        VecX xr = VecX::Zero(7);
        uint64_t t0 = CntTicks();
        for (int n = 0; n < BENCH_REPS; n++) {
            qp.reset(7, 0, nIneq);
            (void)qp.solve_quadprog(H, g0, CE, ce0, CI, ci0, xr);
            sink += xr[0];
        }
        uint64_t t1 = CntTicks();
        if (t1 - t0 < best) {
            best = t1 - t0;
        }
    }
    printf("[eq] solve 7-var QP : %8.2f us/op (best of 3 x %d)\n",
        TicksToUs(best) / BENCH_REPS, BENCH_REPS);
    printf("[eq] sink=%f\n", (double)sink);
}

extern "C" {

void Init(uintptr_t param1, uintptr_t param2, uintptr_t param3, uintptr_t param4)
{
    (void)param1; (void)param2; (void)param3; (void)param4;
    g_cntFreq = CntFreq();
    printf("eiquadprog test start: counter=%llu Hz\n", (unsigned long long)g_cntFreq);
    SolveBench();
    printf("eiquadprog test end\n");
}

} /* extern "C" */
