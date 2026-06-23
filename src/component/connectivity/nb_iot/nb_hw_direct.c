#include "nb_iot/nb_hw_adapter.h"

#include <stdio.h>
#include <string.h>

#include "nb_direct_link.h"

#ifndef UNIPROTON_NB_DIRECT_PORT
#define UNIPROTON_NB_DIRECT_PORT 56830U
#endif

#define NB_DIRECT_CHANNEL "nb_iot"
#define NB_DIRECT_FEATURE_LEN 32U

static char g_feature[NB_DIRECT_FEATURE_LEN];
static oob_callback g_callback;
static oob_cmd_match g_match;
static int g_initialized;
static int g_connected;

static int nb_direct_request(const char *req)
{
    char channel[32];
    char resp[32];
    uint32_t resp_len = sizeof(resp) - 1U;

    if (req == NULL) {
        return -1;
    }

    if (nb_direct_link_request(NB_DIRECT_CHANNEL, (const uint8_t *)req, (uint32_t)strlen(req),
        channel, sizeof(channel), (uint8_t *)resp, &resp_len) != 0) {
        return -1;
    }

    resp[resp_len] = '\0';
    return (strcmp(channel, NB_DIRECT_CHANNEL) == 0 && strcmp(resp, "OK") == 0) ? 0 : -1;
}

int nb_hw_adapter_init(sec_param_s *psk)
{
    (void)psk;
    g_initialized = 1;
    g_connected = 0;
    return 0;
}

int nb_hw_adapter_detect(void)
{
    return g_initialized ? 0 : -1;
}

int nb_hw_adapter_wait_network(void)
{
    return g_initialized ? 0 : -1;
}

int nb_hw_adapter_query_ip(void)
{
    return g_initialized ? 0 : -1;
}

int nb_hw_adapter_set_cdpserver(const char *host, const char *port)
{
    char req[96];

    if (!g_initialized || host == NULL || port == NULL) {
        return -1;
    }

    if (nb_direct_link_open(host, UNIPROTON_NB_DIRECT_PORT) != 0) {
        return -1;
    }
    g_connected = 1;

    (void)snprintf(req, sizeof(req), "CONNECT %s %s", host, port);
    return nb_direct_request(req);
}

int nb_hw_adapter_send_payload(const char *buf, int len)
{
    char req[192];

    if (!g_connected || buf == NULL || len <= 0) {
        return -1;
    }

    (void)snprintf(req, sizeof(req), "REPORT %.*s", len, buf);
    return nb_direct_request(req);
}

int nb_hw_adapter_oob_register(char *featurestr, int cmdlen, oob_callback callback, oob_cmd_match cmd_match)
{
    int copy_len;

    if (featurestr == NULL || cmdlen <= 0 || callback == NULL || cmd_match == NULL) {
        return -1;
    }

    copy_len = (cmdlen < (int)sizeof(g_feature) - 1) ? cmdlen : (int)sizeof(g_feature) - 1;
    (void)memcpy(g_feature, featurestr, (uint32_t)copy_len);
    g_feature[copy_len] = '\0';
    g_callback = callback;
    g_match = cmd_match;
    return 0;
}

int nb_hw_adapter_deinit(void)
{
    g_feature[0] = '\0';
    g_callback = NULL;
    g_match = NULL;
    g_initialized = 0;
    g_connected = 0;
    nb_direct_link_close();
    return 0;
}
