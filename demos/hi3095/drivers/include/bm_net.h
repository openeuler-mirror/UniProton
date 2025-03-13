/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2024-2024. All rights reserved.
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 * 	http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * See the Mulan PSL v2 for more details.
 */
#ifndef __BM_NET_H__
#define __BM_NET_H__
/**
 * @defgroup bm_net bm_net
 */

typedef enum {
    ETH0 = 0,
    ETH1,
    ETH2,
    ETH3,
    ETH_BUTT,
} bm_eth;

/* Maximum physical address */
#define GMAC_PHY_MAX_ADDR 0x1F

/**
 *
 * @ingroup bm_net
 * @brief net initialization.
 *
 * @par description:
 * net initialization.
 *
 * @param eth      [in]: gmac number, bm_eth
 * @param phy_addr [in]: range in (0<phy_addr<GMAC_PHY_MAX_ADDR)
 *
 * @return BM_OK/BM_FAIL
 *
 */
int bm_net_init(bm_eth eth, unsigned int phy_addr);

/**
 *
 * @ingroup bm_net
 * @brief net deinitialization.
 *
 * @par description:
 * net deinitialization.
 *
 * @param eth [in]: gmac number, bm_eth
 *
 * @return BM_OK/BM_FAIL
 *
 */
int bm_net_deinit(bm_eth eth);

/**
 *
 * @ingroup bm_net
 * @brief net send packet.
 *
 * @par description:
 * net send packet.
 *
 * @param eth    [in]: gmac number, bm_eth
 * @param packet [in]: send packet pointer
 * @param length [in]: send packet length, send length range (0 < length < 1600)
 *
 * @return BM_OK/BM_FAIL
 *
 */
int bm_net_send(bm_eth eth, const unsigned char *packet, unsigned int length);

/**
 *
 * @ingroup bm_net
 * @brief net receive packet.
 *
 * @par description:
 * net receive packet.
 *
 * @param eth     [in] : gmac number, bm_eth
 * @param rx_len  [out]: receive packet length pointer
 * @param packetp [out]: receive packet level-2 pointer
 *
 * @return BM_OK/BM_FAIL
 *
 */
int bm_net_receive(bm_eth eth, unsigned int *rx_len, unsigned char **packetp);

/**
 *
 * @ingroup bm_net
 * @brief net set mac addr.
 *
 * @par description:
 * net set mac addr.
 *
 * @param eth    [in]: gmac number, bm_eth
 * @param rx_len [in]: mac addr pointer, the length of its pointed buffer must be longer than 6 bytes
 *
 * @return BM_OK/BM_FAIL
 *
 */
int bm_net_set_mac_addr(bm_eth eth, const unsigned char *mac_addr);

#endif /* __BM_NET_H__ */