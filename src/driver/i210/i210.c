/*
 * Copyright (c) 2023-2023 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Create: 2023-06-21
 * Description: I210网卡功能适配
 */
#include <string.h>
#include "securec.h"
#include "prt_typedef.h"
#include "prt_mem.h"
#include "i210.h"

struct mac_dev *g_macdev;

void i210_init(void)
{
    g_macdev = (struct mac_dev*)MAC_DEV_BASE;
}

static inline void plat_writel(U32 val, U32 *addr)
{
    *addr = val;
}

static inline U32 plat_readl(U32 *addr)
{
    return *addr;
}

static void mac_read(struct mac_dev *dev, int offset, U32 *val)
{
    *val = plat_readl((U32*)(size_t)(dev->mac_base + offset));
}

static void mac_write(struct mac_dev *dev, int offset, U32 val)
{
    plat_writel(val, (U32*)(size_t)(dev->mac_base + offset));
}

static void queue_poll_send(struct mac_dev *dev)
{
    unsigned int head;
    mac_read(dev, E1000_TDH(0), &head);
    dev->queue.tx_head = head;
}

static void packet_send(struct mac_dev *dev, struct pkt *pkt)
{
    struct mac_queue *q = &dev->queue;
    struct mac_tx_desc *desc = &q->tx_desc[q->tx_tail];

    queue_poll_send(dev);
    while ((q->tx_tail + 1) % q->rx_desc_nr == q->tx_head) {
        queue_poll_send(dev);
    }

    desc->buffer_addr = pkt->dma;
    desc->lower.flags.length = pkt->size;
    desc->lower.flags.cmd = (E1000_TXD_CMD_IDE | E1000_TXD_CMD_RS
        | E1000_TXD_CMD_IFCS | E1000_TXD_CMD_EOP);
    q->tx_tail = (q->tx_tail + 1) % q->tx_desc_nr;
    mac_write(dev, E1000_TDT(0), q->tx_tail);
}

int i210_packet_send(const unsigned char *packet, int length)
{
    struct pkt pkt = {
        .dma = (TX_DMA_BASE - g_macdev->queue.virt_addr_offset
            + g_macdev->queue.bus_addr_offset),
        .virt = (U8 *)(size_t)(TX_DMA_BASE),
        .size = ((DMA_SIZE > length) ? length : DMA_SIZE),
    };

    memcpy_s(pkt.virt, pkt.size, packet, length);
    packet_send(g_macdev, &pkt);

    return OS_OK;
}

static void queue_poll_recv_once(struct mac_dev *dev, struct pkt *packet)
{
    struct mac_queue *q = &dev->queue;
    unsigned int head;
    unsigned int rx_head = q->rx_head;
    struct mac_rx_desc *rx_desc = q->rx_desc;

    mac_read(dev, E1000_RDH(0), &head);
    
    if (rx_head != head) {
        packet->virt = (U8 *)(size_t)(rx_desc[rx_head].buffer_addr
            - dev->queue.bus_addr_offset + dev->queue.virt_addr_offset);
        packet->size = rx_desc[rx_head].writeback & 0xffff;
        if (++rx_head == (unsigned int)q->rx_desc_nr) {
            rx_head = 0;
        }
        q->rx_head = rx_head;
        q->rx_tail = (rx_head + q->rx_desc_nr - 1) % q->rx_desc_nr;
    }

    mac_write(dev, E1000_RDT(0), q->rx_tail);
}

int i210_packet_recv(unsigned char *packet, int size)
{
    int length;
    struct pkt input = {0};

    if (!packet || size < 0) {
        return 0;
    }

    queue_poll_recv_once(g_macdev, &input);
    length = ((size > input.size) ? input.size : size);
    memcpy_s(packet, size, input.virt, input.size);

    return length;
}

bool i210_get_link_status(void)
{
    U32 status;
    mac_read(g_macdev, E1000_STATUS, &status);

    return ((status & E1000_STATUS_LU) ? true : false);
}

#define MAC_ADDR_BYTE_NUM 6
int i210_get_mac_address(unsigned char *buf, int buf_len)
{
    unsigned char *pmac_addr;
    if (g_macdev == NULL) {
        return -1;
    }

    pmac_addr = (unsigned char*)(uintptr_t)(g_macdev->mac_base + E1000_RAL);
    return memcpy_s(buf, buf_len, pmac_addr, MAC_ADDR_BYTE_NUM);
}