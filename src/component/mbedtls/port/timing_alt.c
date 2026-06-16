#include "mbedtls/timing.h"
#include "prt_clk.h"
#include "prt_config.h"
#include "prt_task.h"
#include "prt_tick.h"

volatile int mbedtls_timing_alarmed;

static uint64_t uniproton_time_ms(void)
{
    return (PRT_TickGetCount() * 1000ULL) / OS_TICK_PER_SECOND;
}

unsigned long mbedtls_timing_hardclock(void)
{
    return (unsigned long)PRT_ClkGetCycleCount64();
}

unsigned long mbedtls_timing_get_timer(struct mbedtls_timing_hr_time *val, int reset)
{
    uint64_t now = uniproton_time_ms();

    if (reset != 0) {
        val->timer_ms = now;
        return 0;
    }

    return (unsigned long)(now - val->timer_ms);
}

void mbedtls_set_alarm(int seconds)
{
    uint32_t ticks;

    mbedtls_timing_alarmed = 0;
    if (seconds <= 0) {
        mbedtls_timing_alarmed = 1;
        return;
    }

    ticks = (uint32_t)seconds * OS_TICK_PER_SECOND;
    PRT_TaskDelay(ticks);
    mbedtls_timing_alarmed = 1;
}

void mbedtls_timing_set_delay(void *data, uint32_t int_ms, uint32_t fin_ms)
{
    mbedtls_timing_delay_context *ctx = (mbedtls_timing_delay_context *)data;

    ctx->int_ms = int_ms;
    ctx->fin_ms = fin_ms;
    if (fin_ms != 0) {
        (void)mbedtls_timing_get_timer(&ctx->timer, 1);
    }
}

int mbedtls_timing_get_delay(void *data)
{
    mbedtls_timing_delay_context *ctx = (mbedtls_timing_delay_context *)data;
    unsigned long elapsed_ms;

    if (ctx->fin_ms == 0) {
        return -1;
    }

    elapsed_ms = mbedtls_timing_get_timer(&ctx->timer, 0);
    if (elapsed_ms >= ctx->fin_ms) {
        return 2;
    }
    if (elapsed_ms >= ctx->int_ms) {
        return 1;
    }

    return 0;
}
