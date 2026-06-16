#include <string.h>
#include "mbedtls/entropy.h"
#include "prt_clk.h"
#include "prt_tick.h"

int mbedtls_hardware_poll(void *data, unsigned char *output, size_t len, size_t *olen)
{
    static uint64_t counter;
    size_t copied = 0;

    (void)data;
    counter++;

    while (copied < len) {
        uint64_t sample = PRT_ClkGetCycleCount64() ^ (PRT_TickGetCount() << 32) ^ counter;
        size_t chunk = len - copied;
        if (chunk > sizeof(sample)) {
            chunk = sizeof(sample);
        }
        memcpy(output + copied, &sample, chunk);
        copied += chunk;
        counter += 0x9e3779b97f4a7c15ULL;
    }

    *olen = len;
    return 0;
}
