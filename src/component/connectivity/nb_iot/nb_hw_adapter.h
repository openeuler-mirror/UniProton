#ifndef UNIPROTON_CONNECTIVITY_NB_IOT_NB_HW_ADAPTER_H
#define UNIPROTON_CONNECTIVITY_NB_IOT_NB_HW_ADAPTER_H

#include <stdint.h>

#include "nb_iot/los_nb_api.h"

#ifdef __cplusplus
extern "C" {
#endif

int nb_hw_adapter_init(sec_param_s *psk);
int nb_hw_adapter_detect(void);
int nb_hw_adapter_wait_network(void);
int nb_hw_adapter_query_ip(void);
int nb_hw_adapter_set_cdpserver(const char *host, const char *port);
int nb_hw_adapter_send_payload(const char *buf, int len);
int nb_hw_adapter_oob_register(char *featurestr, int cmdlen, oob_callback callback, oob_cmd_match cmd_match);
int nb_hw_adapter_deinit(void);

#ifdef __cplusplus
}
#endif

#endif
