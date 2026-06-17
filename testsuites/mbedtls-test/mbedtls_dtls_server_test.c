#include "mbedtls_dtls_server_test.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/entropy.h"
#include "mbedtls/error.h"
#include "mbedtls/net_sockets.h"
#include "mbedtls/platform.h"
#include "mbedtls/ssl.h"
#include "mbedtls/timing.h"

extern unsigned int PRT_Printf(const char *format, ...);

#define TEST_LOG(fmt, ...) PRT_Printf("[MBEDTLS_TEST] " fmt "\n", ##__VA_ARGS__)

#define SERVER_PORT "5658"
#define SERVER_PSK "11223344556677881122334455667788"
#define SERVER_IDENTITY "testserver1"
#define SERVER_REPLY "Hi Client"
#define READ_TIMEOUT_MS 10000

static void print_mbedtls_error(const char *step, int ret)
{
    char err_buf[128];

    mbedtls_strerror(ret, err_buf, sizeof(err_buf));
    TEST_LOG("[ERROR] %s failed: -0x%x (%s)", step, -ret, err_buf);
}

void mbedtls_dtls_server_test(void)
{
    int ret;
    unsigned char buf[128];
    const char *pers = "uniproton_dtls_server";
    mbedtls_net_context listen_fd;
    mbedtls_net_context client_fd;
    mbedtls_ssl_context ssl;
    mbedtls_ssl_config conf;
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_timing_delay_context timer;

    TEST_LOG("DTLS PSK server start, port=%s", SERVER_PORT);

    mbedtls_net_init(&listen_fd);
    mbedtls_net_init(&client_fd);
    mbedtls_ssl_init(&ssl);
    mbedtls_ssl_config_init(&conf);
    mbedtls_entropy_init(&entropy);
    mbedtls_ctr_drbg_init(&ctr_drbg);

    ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy,
        (const unsigned char *)pers, strlen(pers));
    if (ret != 0) {
        print_mbedtls_error("ctr_drbg_seed", ret);
        goto cleanup;
    }

    ret = mbedtls_ssl_config_defaults(&conf, MBEDTLS_SSL_IS_SERVER,
        MBEDTLS_SSL_TRANSPORT_DATAGRAM, MBEDTLS_SSL_PRESET_DEFAULT);
    if (ret != 0) {
        print_mbedtls_error("ssl_config_defaults", ret);
        goto cleanup;
    }

    mbedtls_ssl_conf_rng(&conf, mbedtls_ctr_drbg_random, &ctr_drbg);
    mbedtls_ssl_conf_authmode(&conf, MBEDTLS_SSL_VERIFY_NONE);
    ret = mbedtls_ssl_conf_psk(&conf, (const unsigned char *)SERVER_PSK,
        strlen(SERVER_PSK), (const unsigned char *)SERVER_IDENTITY,
        strlen(SERVER_IDENTITY));
    if (ret != 0) {
        print_mbedtls_error("ssl_conf_psk", ret);
        goto cleanup;
    }

    ret = mbedtls_ssl_setup(&ssl, &conf);
    if (ret != 0) {
        print_mbedtls_error("ssl_setup", ret);
        goto cleanup;
    }
    mbedtls_ssl_set_timer_cb(&ssl, &timer, mbedtls_timing_set_delay,
        mbedtls_timing_get_delay);

    ret = mbedtls_net_bind(&listen_fd, NULL, SERVER_PORT, MBEDTLS_NET_PROTO_UDP);
    if (ret != 0) {
        print_mbedtls_error("net_bind", ret);
        goto cleanup;
    }

    TEST_LOG("waiting for DTLS client");
    ret = mbedtls_net_accept(&listen_fd, &client_fd, NULL, 0, NULL);
    if (ret != 0) {
        print_mbedtls_error("net_accept", ret);
        goto cleanup;
    }
    mbedtls_ssl_set_bio(&ssl, &client_fd, mbedtls_net_send, mbedtls_net_recv,
        mbedtls_net_recv_timeout);

    TEST_LOG("handshake start");
    do {
        ret = mbedtls_ssl_handshake(&ssl);
    } while (ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE);
    if (ret != 0) {
        print_mbedtls_error("ssl_handshake", ret);
        goto cleanup;
    }
    TEST_LOG("handshake ok");

    memset(buf, 0, sizeof(buf));
    do {
        ret = mbedtls_ssl_read(&ssl, buf, sizeof(buf) - 1);
    } while (ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE ||
        ret == MBEDTLS_ERR_SSL_TIMEOUT);
    if (ret <= 0) {
        print_mbedtls_error("ssl_read", ret);
        goto cleanup;
    }
    TEST_LOG("read %d bytes: %s", ret, buf);

    do {
        ret = mbedtls_ssl_write(&ssl, (const unsigned char *)SERVER_REPLY,
            strlen(SERVER_REPLY));
    } while (ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE);
    if (ret < 0) {
        print_mbedtls_error("ssl_write", ret);
        goto cleanup;
    }
    TEST_LOG("write %d bytes: %s", ret, SERVER_REPLY);
    TEST_LOG("DTLS PSK server test passed");

cleanup:
    (void)mbedtls_ssl_close_notify(&ssl);
    mbedtls_net_free(&client_fd);
    mbedtls_net_free(&listen_fd);
    mbedtls_ssl_free(&ssl);
    mbedtls_ssl_config_free(&conf);
    mbedtls_ctr_drbg_free(&ctr_drbg);
    mbedtls_entropy_free(&entropy);
}
