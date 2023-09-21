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

#ifndef _I210_H_
#define _I210_H_

#define RESERVE_MEM_BASE 0xf00200000ULL
#define NR_DESC 128
#define DMA_SIZE 2048

#define RX_RING_BASE RESERVE_MEM_BASE
#define RX_RING_SIZE (sizeof(struct mac_rx_desc) * NR_DESC)
#define RX_DMA_BASE (RX_RING_BASE + RX_RING_SIZE)
#define RX_DMA_SIZE (DMA_SIZE * NR_DESC)

#define TX_RING_BASE (RX_DMA_BASE + RX_DMA_SIZE)
#define TX_RING_SIZE (sizeof(struct mac_tx_desc) * NR_DESC)
#define TX_DMA_BASE (TX_RING_BASE + TX_RING_SIZE)
#define TX_DMA_SIZE (DMA_SIZE * NR_DESC)

#define MAC_DEV_BASE (TX_DMA_BASE + TX_DMA_SIZE)
#define MAC_DEV_SIZE (sizeof(struct mac_dev))
#define TOTAL_SIZE (MAC_DEV_BASE + MAC_DEV_SIZE - RESERVE_MEM_BASE)

#define CRC_SIZE 4

#define E1000_RDBAL(_n) (0x0c000 + ((_n) * 0x40))
#define E1000_RDBAH(_n) (0x0c004 + ((_n) * 0x40))
#define E1000_RDLEN(_n) (0x0c008 + ((_n) * 0x40))
#define E1000_RDH(_n)   (0x0c010 + ((_n) * 0x40))
#define E1000_RDT(_n)   (0x0c018 + ((_n) * 0x40))
#define E1000_RXDCTL(_n) (0x0c028 + ((_n) * 0x40))

#define E1000_TDBAL(_n) (0x0e000 + ((_n) * 0x40))
#define E1000_TDBAH(_n) (0x0e004 + ((_n) * 0x40))
#define E1000_TDLEN(_n) (0x0e008 + ((_n) * 0x40))
#define E1000_TDH(_n)   (0x0e010 + ((_n) * 0x40))
#define E1000_TDT(_n)   (0x0e018 + ((_n) * 0x40))

#define E1000_STATUS    0x00008         /* Device Status - RO */
#define E1000_STATUS_LU 0x00000002      /* Link up.0=no,1=link */

#define E1000_RAL  0x05400
#define E1000_RAH  0x05404

/* Transmit Descriptor */
struct mac_tx_desc {
    unsigned long long buffer_addr; /* Address of the descriptor's data buf */
    union {
        unsigned int data;
        struct {
            unsigned short length;  /* Data buffer length */
            unsigned char cso;  /* Checksum offset */
            unsigned char cmd;  /* Descriptor control */
#define E1000_TXD_CMD_EOP 0x01   /* End of Packet */
#define E1000_TXD_CMD_IFCS 0x02  /* Insert FCS (Ethernet CRC) */
#define E1000_TXD_CMD_IC 0x04    /* Insert Checksum */
#define E1000_TXD_CMD_RS 0x08    /* Report Status */
#define E1000_TXD_CMD_RPS 0x10   /* Report Packet Sent */
#define E1000_TXD_CMD_DEXT 0x20  /* Desc extension (0 = legacy) */
#define E1000_TXD_CMD_VLE 0x40   /* Add VLAN tag */
#define E1000_TXD_CMD_IDE 0x80   /* Interrupt Delay Enable */
        } flags;
    } lower;
    union {
        unsigned int data;
        struct {
            unsigned char status;   /* Descriptor status */
            unsigned char css; /* Checksum start */
            unsigned short speial;
        } fields;
    } upper;
};

/* Receive Descriptor - Extended */
struct mac_rx_desc {
    unsigned long long buffer_addr;
    /* writeback */
    unsigned long long writeback;
};

struct mac_queue {
    unsigned long long virt_addr_offset;
    unsigned long long bus_addr_offset;
    unsigned long long rx_ring_dma;
    unsigned int rx_size;
    unsigned long long tx_ring_dma;
    unsigned int tx_size;
    struct mac_rx_desc *rx_desc;
    int rx_desc_nr;
    int rx_head;
    int rx_tail;
    struct mac_tx_desc *tx_desc;
    int tx_desc_nr;
    int tx_head;
    int tx_tail;
};

struct mac_dev {
    struct mac_queue queue;
    unsigned long long mac_base;
    unsigned long long pci_cfg_base;
    unsigned int pm_cap;
    unsigned int msi_cap;
    int locked;
};

struct pkt {
    unsigned long long dma;
    unsigned char *virt;
    int size;
};

void i210_init(void);
int i210_packet_recv(unsigned char *packet, int size);
int i210_packet_send(const unsigned char *packet, int length);
bool i210_get_link_status(void);
int i210_get_mac_address(unsigned char *buf, int buf_len);

#endif