#include "nb_iot/los_nb_api.h"

#include <stddef.h>

#include "nb_iot/nb_hw_adapter.h"

int los_nb_init(const int8_t *host, const int8_t *port, sec_param_s *psk)
{
    if (host == NULL || port == NULL) {
        return -1;
    }

    if (nb_hw_adapter_init(psk) != 0 || nb_hw_adapter_detect() != 0 || nb_hw_adapter_wait_network() != 0 ||
        nb_hw_adapter_query_ip() != 0) {
        return -1;
    }

    return nb_hw_adapter_set_cdpserver((const char *)host, (const char *)port);
}

int los_nb_report(const char *buf, int buflen)
{
    if (buf == NULL || buflen <= 0) {
        return -1;
    }
    return nb_hw_adapter_send_payload(buf, buflen);
}

int los_nb_notify(char *featurestr, int cmdlen, oob_callback callback, oob_cmd_match cmd_match)
{
    return nb_hw_adapter_oob_register(featurestr, cmdlen, callback, cmd_match);
}

int los_nb_deinit(void)
{
    return nb_hw_adapter_deinit();
}

int nb_send_str(const char *buf, int len)
{
    if (buf == NULL || len <= 0) {
        return -1;
    }
    return nb_hw_adapter_send_payload(buf, len);
}
