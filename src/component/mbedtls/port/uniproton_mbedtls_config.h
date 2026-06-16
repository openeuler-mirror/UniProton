/*
 * Minimal Mbed TLS configuration for UniProton sd3403 DTLS PSK server tests.
 */
#ifndef UNIPROTON_MBEDTLS_CONFIG_H
#define UNIPROTON_MBEDTLS_CONFIG_H

/* System support */
#define MBEDTLS_PLATFORM_C
#define MBEDTLS_PLATFORM_MEMORY

/* DTLS/PSK feature support */
#define MBEDTLS_KEY_EXCHANGE_PSK_ENABLED
#define MBEDTLS_SSL_PROTO_TLS1_2
#define MBEDTLS_SSL_PROTO_DTLS
#define MBEDTLS_SSL_DTLS_ANTI_REPLAY

/* Crypto primitives used by MBEDTLS_TLS_PSK_WITH_AES_128_CBC_SHA256. */
#define MBEDTLS_AES_C
#define MBEDTLS_CIPHER_C
#define MBEDTLS_CIPHER_MODE_CBC
#define MBEDTLS_CTR_DRBG_C
#define MBEDTLS_ENTROPY_C
#define MBEDTLS_MD_C
#define MBEDTLS_SHA256_C

/* TLS modules */
#define MBEDTLS_SSL_SRV_C
#define MBEDTLS_SSL_TLS_C
#define MBEDTLS_SSL_CIPHERSUITES MBEDTLS_TLS_PSK_WITH_AES_128_CBC_SHA256
#define MBEDTLS_SSL_MAX_CONTENT_LEN 1024

/* UniProton supplies socket, timing and entropy implementations. */
#define MBEDTLS_NET_C
#define MBEDTLS_TIMING_C
#define MBEDTLS_TIMING_ALT
#define MBEDTLS_ENTROPY_HARDWARE_ALT
#define MBEDTLS_NO_PLATFORM_ENTROPY
#define MBEDTLS_ENTROPY_MAX_SOURCES 2

/* Diagnostics used by the test entry. */
#define MBEDTLS_ERROR_C

#include "mbedtls/check_config.h"

#endif /* UNIPROTON_MBEDTLS_CONFIG_H */
