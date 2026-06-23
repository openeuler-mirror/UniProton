#ifndef UNIPROTON_CONNECTIVITY_NB_IOT_LOS_NB_API_H
#define UNIPROTON_CONNECTIVITY_NB_IOT_LOS_NB_API_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*oob_callback)(void *arg, int8_t *buf, int32_t buflen);
typedef int (*oob_cmd_match)(const char *buf, char *featurestr, int len);

typedef struct sec_param {
    char *psk;
    char *pskid;
    uint8_t setpsk;
} sec_param_s;

int los_nb_init(const int8_t *host, const int8_t *port, sec_param_s *psk);
int los_nb_report(const char *buf, int buflen);
int los_nb_notify(char *featurestr, int cmdlen, oob_callback callback, oob_cmd_match cmd_match);
int los_nb_deinit(void);

int nb_send_str(const char *buf, int len);

#ifdef __cplusplus
}
#endif

#endif
