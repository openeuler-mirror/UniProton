/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#include "bm_common.h"
#include "sample_common.h"
#include "bm_net.h"

#define SAMPLE_NET_SEND_LENGTH 78
#define SAMPLE_NET_SEND_TIMES 10
/* 网口数据需要符合以太网规范，如下数组只是一个样例： */
XFER_DATA unsigned char g_test_tx_data[SAMPLE_NET_SEND_LENGTH + 1] = {
    0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x33, 0x08, 0x00, 0x45, 0x00,
    0x00, 0x40, 0x00, 0x01, 0x00, 0x00, 0x40, 0x01,
    0x69, 0x48, 0xc0, 0xa8, 0xc8, 0x21, 0xc0, 0xa8,
    0xc8, 0x01, 0x08, 0x00, 0x5f, 0x5d, 0x00, 0x01,
    0x00, 0x01, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36,
    0x37, 0x38, 0x39, 0x30, 0x61, 0x62, 0x63, 0x64,
    0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c,
    0x6d, 0x6e, 0x6f, 0x70, 0x71, 0x72, 0x73, 0x74,
    0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, };

XFER_DATA unsigned char g_mac_addr[] = {0xa, 0xb, 0xc, 0xd, 0xe, 0xf};

/* ************ sample_net_send start ************ */
static int sample_net_send(unsigned int core, bm_eth eth, unsigned int phy_addr)
{
    (void)core;

    if (bm_net_init(eth, phy_addr)) {
        return BM_FAIL;
    }

    int ret = bm_net_set_mac_addr(eth, g_mac_addr);
    if (ret) {
        bm_log("sample_net: send_receive, set mac addr failed\n");
        return BM_FAIL;
    }

    for (int i = 0; i < SAMPLE_NET_SEND_TIMES; i++) {
        bm_net_send(eth, g_test_tx_data, SAMPLE_NET_SEND_LENGTH);
        udelay(1000000);  // delay 1000000 us
    }

    return BM_OK;
}
/* ************ sample_net_send end ************ */

/* ************ sample_net_receive start ************ */
static int sample_net_receive(unsigned int core, bm_eth eth, unsigned int phy_addr)
{
    (void)core;

    if (bm_net_init(eth, phy_addr)) {
        return BM_FAIL;
    }

    unsigned char *rx_addr = NULL;
    unsigned int count = 0;
    unsigned int len = 0;

    int ret = bm_net_set_mac_addr(eth, g_mac_addr);
    if (ret) {
        bm_log("sample_net: send_receive, set mac addr failed\n");
        return BM_FAIL;
    }

    while (1) {
        ret = bm_net_receive(eth, &len, &rx_addr);
        if ((ret != BM_FAIL) && len) {
            bm_log("sample_net: receive, rx_len = %d\n", len);
            count++;
        }
        if (count > 6) {  // 统计 6 次
            break;
        }
        len = 0;
        bm_log("sample_net: receive, try rx data\n");
        udelay(1000000);  // delay 1000000
    }

    return BM_OK;
}
/* ************ sample_net_receive end ************ */

/* ************ sample_net_send_receive start ************ */
static int sample_net_send_receive(unsigned int core, bm_eth eth, unsigned int phy_addr)
{
    (void)core;

    if (bm_net_init(eth, phy_addr)) {
        return BM_FAIL;
    }

    int ret = bm_net_set_mac_addr(eth, g_mac_addr);
    if (ret) {
        bm_log("sample_net: send_receive, set mac addr failed\n");
        return BM_FAIL;
    }

    for (int i = 0; i < SAMPLE_NET_SEND_TIMES; i++) {
        bm_net_send(eth, g_test_tx_data, SAMPLE_NET_SEND_LENGTH);
        udelay(1000000);  // delay 1000000 us
    }

    unsigned char *rx_addr = NULL;
    unsigned int count = 0;
    unsigned int len = 0;

    while (1) {
        ret = bm_net_receive(eth, &len, &rx_addr);
        if ((ret != BM_FAIL) && len) {
            bm_log("sample_net: send_receive, rx_len = %d\n", len);
            count++;
        }
        if (count > 6) {  // 统计 6 次
            break;
        }
        len = 0;
        bm_log("sample_net: send_receive, try rx data\n");
        udelay(1000000);  // delay 1000000
    }
    return BM_OK;
}
/* ************ sample_net_send_receive end ************ */

/* ************ sample_net start ************ */
#if defined(__BAREMETAL__)
int main(void)
#elif defined(__UNIPROTON__)
int app_main(void)
#endif
{
    sample_prepare();
    unsigned int net_core = bm_get_coreid();
    /* GMAC0:2, GMAC1:1, GMAC2:3, GMAC3:7 */
    bm_log("net_core:%d\n", net_core);
    bm_eth eth = ETH0;
    unsigned int phy_addr = 2;
    switch (net_core) {
        case SAMPLE_CORE0:
            return BM_OK;
        case SAMPLE_CORE1:
            break;
        case SAMPLE_CORE2:
            break;
        case SAMPLE_CORE3:
            sample_net_send(net_core, eth, phy_addr);
            sample_net_receive(net_core, eth, phy_addr);
            sample_net_send_receive(net_core, eth, phy_addr);
            break;
        default:
            bm_log("sample_net: invalid core_num\n");
            break;
    }
    bm_log("success\n");
    while (1) {
    }
    return BM_OK;
}
/* ************ sample_net end ************ */