#ifndef UNIPROTON_MBEDTLS_TIMING_ALT_H
#define UNIPROTON_MBEDTLS_TIMING_ALT_H

#include <stdint.h>

struct mbedtls_timing_hr_time {
    uint64_t timer_ms;
};

typedef struct mbedtls_timing_delay_context {
    struct mbedtls_timing_hr_time timer;
    uint32_t int_ms;
    uint32_t fin_ms;
} mbedtls_timing_delay_context;

#endif /* UNIPROTON_MBEDTLS_TIMING_ALT_H */
