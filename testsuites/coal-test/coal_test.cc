/* coal_test.cc - coal (hpp-fcl) collision/distance primitives on UniProton sd3403.
 *
 * Sphere-box collision + distance queries with CNTPCT timing at -O2.
 * The full BVH/mesh path needs a filesystem; primitive shapes cover the
 * bare-metal smoke check.
 */
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <locale.h>

#include <hpp/fcl/collision.h>
#include <hpp/fcl/distance.h>
#include <hpp/fcl/shape/geometric_shapes.h>

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

#define BENCH_REPS 2000

static void RunAll(void)
{
    using namespace hpp::fcl;

    Sphere sphere(0.05);
    Box box(Eigen::Vector3d(0.1, 0.2, 0.3));
    Transform3f tfBox;
    tfBox.setTranslation(Eigen::Vector3d(0.0, 0.0, 0.1)); /* overlapping */
    CollisionRequest req;
    CollisionResult res;
    bool hit = collide(&sphere, Transform3f(), &box, tfBox, req, res);
    printf("[coal] collide overlap: %d numContacts=%d\n", (int)hit, (int)res.numContacts());

    Transform3f tfFar;
    tfFar.setTranslation(Eigen::Vector3d(0.0, 0.0, 0.5)); /* box z 0.35..0.65: 0.30 clear */
    CollisionResult res2;
    bool hit2 = collide(&sphere, Transform3f(), &box, tfFar, req, res2);
    DistanceRequest dreq;
    DistanceResult dres;
    double dist = distance(&sphere, Transform3f(), &box, tfFar, dreq, dres);
    printf("[coal] collide separated: %d distance=%.6f\n", (int)hit2, dist);

    volatile double sink = 0.0;
    uint64_t best = ~0ull;
    for (int round = 0; round < 3; round++) {
        double acc = 0.0;
        uint64_t t0 = CntTicks();
        for (int n = 0; n < BENCH_REPS; n++) {
            CollisionResult r;
            collide(&sphere, Transform3f(), &box, tfBox, req, r);
            acc += r.numContacts();
        }
        uint64_t t1 = CntTicks();
        sink += acc;
        if (t1 - t0 < best) { best = t1 - t0; }
    }
    printf("[coal] sphere-box collide : %8.2f us/op (best of 3 x %d)\n",
        TicksToUs(best) / BENCH_REPS, BENCH_REPS);

    best = ~0ull;
    for (int round = 0; round < 3; round++) {
        double acc = 0.0;
        uint64_t t0 = CntTicks();
        for (int n = 0; n < BENCH_REPS; n++) {
            DistanceResult dr;
            acc += distance(&sphere, Transform3f(), &box, tfFar, dreq, dr);
        }
        uint64_t t1 = CntTicks();
        sink += acc;
        if (t1 - t0 < best) { best = t1 - t0; }
    }
    printf("[coal] sphere-box distance: %8.2f us/op\n", TicksToUs(best) / BENCH_REPS);
    printf("[coal] sink=%f counter=%llu Hz\n", (double)sink, (unsigned long long)g_cntFreq);
}

extern "C" {

void Init(uintptr_t param1, uintptr_t param2, uintptr_t param3, uintptr_t param4)
{
    (void)param1; (void)param2; (void)param3; (void)param4;
    g_cntFreq = CntFreq();
    printf("coal test start: counter=%llu Hz\n", (unsigned long long)g_cntFreq);
    RunAll();
    printf("coal test end\n");
}

} /* extern "C" */
