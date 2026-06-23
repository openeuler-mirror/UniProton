#ifndef UNIPROTON_NB_DIRECT_LINK_H
#define UNIPROTON_NB_DIRECT_LINK_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int nb_direct_link_open(const char *host, uint16_t port);
void nb_direct_link_close(void);
int nb_direct_link_request(const char *channel, const uint8_t *tx, uint32_t tx_len,
    char *rx_channel, uint32_t rx_channel_len, uint8_t *rx, uint32_t *rx_len);

#ifdef __cplusplus
}
#endif /* UNIPROTON_NB_DIRECT_LINK_H */

#endif
