/*
 * Copyright (c) 2024-2024 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Create: 2024-1-24
 * Description: I40e网卡功能适配
 */

#include <string.h>
#include "securec.h"
#include <pthread.h>
#include <linux/byteorder/generic.h>
#include "prt_typedef.h"
#include "i40ecs.h"

#define PF_NUM 2
struct i40ecs_rtos_pf *g_rpf[PF_NUM];
int g_socketfd;

void i40e_init(void)
{
    g_rpf[0]= (struct i40ecs_rtos_pf*)RPF_BASE;
    g_rpf[1]= (struct i40ecs_rtos_pf*)(0xf04200000 + RPF_BASE - TX_DESC_BASE);

    for(int i = 0; i < PF_NUM; i++){
        // g_rpf[i] = (struct i40ecs_rtos_pf*)(RPF_BASE + RPF_TOTAL_SIZE * i);
        // g_rpf[i] = (struct i40ecs_rtos_pf*)(RPF_BASE + 0x100000 * i);
        // 打印下mac地址
        printf("MAC addr: %x:%x:%x:%x:%x:%x, hw_addr:%llx\n",
            g_rpf[i]->hw.mac.addr[0], g_rpf[i]->hw.mac.addr[1], g_rpf[i]->hw.mac.addr[2],
            g_rpf[i]->hw.mac.addr[3], g_rpf[i]->hw.mac.addr[4], g_rpf[i]->hw.mac.addr[5], g_rpf[i]->hw_addr);
        if(g_rpf[i]->hw_addr == 0) {
            g_rpf[i] = NULL;
        }
    }
}

static void i40ecs_buffer_dump(U8 *data, U32 size)
{
    U32 i;
    for(i = 0; i < size; i += 8){
        if(i < size - 8) {
            printf("%02x %02x %02x %02x %02x %02x %02x %02x\n",
            data[i], data[i + 1], data[i + 2], data[i + 3],
            data[i + 4], data[i + 5], data[i + 6], data[i + 7]);
            continue;
        }
        switch (size - i){
            case 1:
                printf("%02x\n", data[i]);
                break;
            case 2:
                printf("%02x %02x\n", data[i], data[i + 1]);
                break;
            case 3:
                printf("%02x %02x %02x\n", data[i], data[i + 1], data[i + 2]);
                break;
            case 4:
                printf("%02x %02x %02x %02x\n", data[i], data[i + 1], data[i + 2], data[i +3]);
                break;
            case 5:
                printf("%02x %02x %02x %02x %02x\n", data[i], data[i + 1], data[i + 2], data[i +3],
                    data[i +4]);
                break;
            case 6:
                printf("%02x %02x %02x %02x %02x %02x\n", data[i], data[i + 1], data[i + 2], data[i +3],
                    data[i +4], data[i +5]);
                break;
            case 7:
                printf("%02x %02x %02x %02x %02x %02x %02x\n",
                    data[i], data[i + 1], data[i + 2], data[i +3], data[i +4], data[i +5], data[i +6]);
                break;
        }
    }
}

static U16 i40ecs_rx_ring_get_unused(struct rx_ring *rxr)
{
    return ((rxr->next_to_clean > rxr->next_to_use) ? 0 : rxr->count) +rxr->next_to_clean - rxr->next_to_use - 1;
}

static void i40ecs_alloc_rx_buffers(struct i40ecs_rtos_pf *rpf, U16 cleaned_count)
{
    struct rx_ring *rxr = &rpf->vsi.queues[0].rxr;
    U16 ntu = rxr->next_to_use;
    union i40e_rx_desc *rxd = NULL;
    if(cleaned_count == 0) {
        return;
    }
    do {
        rxd = &(((union i40e_rx_desc *)rxr->desc->vaddr)[ntu]);
        rxd->read.pkt_addr = cpu_to_le64(&((struct i40ecs_rx_buf *)rxr->buf->paddr)[ntu]);
        rxd->read.hdr_addr = 0;
        ntu++;
        if(ntu >= rxr->count) {
            ntu = 0;
        }
        rxd = &(((union i40e_rx_desc *)rxr->desc->vaddr)[ntu]);
        rxd->wb.qword1.status_error_len = 0;
        cleaned_count--;
    } while (cleaned_count != 0);
    if(rxr->next_to_use != ntu) {
        rxr->next_to_use = ntu;
        // wmb(); // *** 先屏蔽掉
        mac_write(rpf,rxr->tail, ntu);
    }
}

static int i40ecs_xmit(struct i40ecs_rtos_pf *rpf, U8 *buf, U32 size)
{
    if(rpf == NULL || buf == NULL || size > TX_BUF_SIZE) {
        return -1;
    }
    printf("rpf->hw_addr:%llx\n", rpf->hw_addr);
    struct i40ecs_vsi *vsi = &rpf->vsi;
    struct i40ecs_queue *que = &vsi->queues[0];
    struct tx_ring *txr = &que->txr;
    struct i40e_tx_desc *txd = NULL;

    U32 tail_pos = mac_read(rpf, I40E_QTX_TAIL(que->me));
    U32 head_pos = mac_read(rpf, I40E_QTX_HEAD(que->me));
    printf("i40ecs_xmit head_pos:%u tail_pos:%u buf_size:%u\n", head_pos, tail_pos, size);
    while ((tail_pos + 1)% que->num_desc == head_pos) {
        head_pos = mac_read(rpf, I40E_QTX_HEAD(que->me));
    }

    txd = &(((struct i40e_tx_desc *)txr->desc->vaddr)[tail_pos]);
    U8 *ring_buf_addr = (U8 *)((U64)txr->buf->vaddr + (sizeof(struct i40ecs_tx_buf) * tail_pos));
    printf("txd addr:%p, txb addr:%p\n", txd, ring_buf_addr);

    memcpy(ring_buf_addr, buf, size);

    txd->buffer_addr = cpu_to_le64(txr->buf->paddr + (sizeof(struct i40ecs_tx_buf) * tail_pos));
    txd->cmd_type_offset_bsz = cpu_to_le64((U64)size  << I40E_TXD_QW1_TX_BUF_SZ_SHIFT | \
        ((U64)(I40E_TX_DESC_CMD_EOP | I40E_TX_DESC_CMD_RS | I40E_TX_DESC_CMD_ICRC) << I40E_TXD_QW1_CMD_SHIFT));
    // wmb(); // *** 先屏蔽掉
    tail_pos = (tail_pos + 1) % que->num_desc;
    mac_write(rpf, txr->tail, tail_pos);

    i40ecs_flush(rpf);
    return 0;
}

int i40e_packet_send(u8 pf_id, const unsigned char *packet, int length)
{
    printf("i40e send data, pf_id:%d\n", pf_id);
    for (int i = 0; i < length; i+=8) {
        printf("%x %x %x %x %x %x %x %x \n", packet[i], packet[i+1], packet[i+2],
        packet[i+3], packet[i+4], packet[i+5],packet[i+6], packet[i+7]);
    }
    struct i40ecs_rtos_pf *rpf = g_rpf[pf_id];
    if (rpf == NULL || rpf->hw_addr == 0) {
        printf("pf_id:%d not exist\n", pf_id);
        return OS_INVALID;
    }
    i40ecs_xmit(rpf, packet, length);
    return OS_OK;
}

static int i40ecs_rx_once(struct i40ecs_rtos_pf *rpf, unsigned char *packet, int length)
{
    struct rx_ring *rxr = &rpf->vsi.queues[0].rxr;
    union i40e_rx_desc *rxd = NULL;
    struct i40ecs_rx_buf *rbuf = NULL;
    U64 qword;
    U32 size = 0;
    U16 cleaned_count = i40ecs_rx_ring_get_unused(rxr);
    if (cleaned_count >= 16) {
        i40ecs_alloc_rx_buffers(rpf, cleaned_count);
    }
    rxd = &((union i40e_rx_desc *)(rxr->desc->vaddr))[rxr->next_to_clean];
    qword = LE64_TO_CPU(rxd->wb.qword1.status_error_len);
    // rmb(); // *** 先屏蔽掉
    if (qword != 0) {
        rbuf = (struct i40ecs_rx_buf *)(rxr->buf->vaddr);
        size = (qword & I40E_RXD_QW1_LENGTH_PBUF_MASK) >> I40E_RXD_QW1_LENGTH_PBUF_SHIFT;
        if(size > 0) {
            printf("pf[%u] vsi[%u] pos[%u] receive %u bytes\n", rpf->hw.pf_id, rpf->vsi.seid, rxr->next_to_clean, size);
            rbuf = &((struct i40ecs_rx_buf *)(rxr->buf->vaddr))[rxr->next_to_clean];
            i40ecs_buffer_dump(rbuf->buf, size);
        }
        rxd->wb.qword1.status_error_len = 0;
        rxr->next_to_clean = (rxr->next_to_clean + 1) % rxr->que->num_desc;

        memcpy_s(packet, length, rbuf->buf, size);
    } else {
        // printf("No packet\n");
    }

    return length > size ? size: length;
}

int i40e_packet_recv(u8 pf_id, unsigned char *packet, int length)
{
    struct i40ecs_rtos_pf *rpf = g_rpf[pf_id];
    if (rpf == NULL || rpf->hw_addr == 0) {
        printf("pf_id:%d not exist\n", pf_id);
        return OS_INVALID;
    }
    return i40ecs_rx_once(rpf, packet, length);
}

static pthread_t td;
static pthread_mutex_t rpf_mutex;

static void *i40e_tx_rx_test(void *arg)
{
    u8 packet_rx[60];
    u32 len_rx = 60;

    test_udp();

    // 收发20次
    for(int i = 0; i < PF_NUM; ++i) {
        if(g_rpf[i] == NULL || g_rpf[i]->hw_addr == 0) continue;
        printf("Test for pf_id:%d start\n", i);
        for(int j = 0; j < 10; ++j) {
            pthread_testcancel();
            pthread_mutex_lock(&rpf_mutex);
            // 收包
            i40e_packet_recv(i, packet_rx, len_rx);
            // 发包
            if(i == 0) {
                /* broadcast PING packet with MAC address, will receive ARP response but we ignore */
                u8 pkt[] = {
                    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x9c, 0x73,
                    0x70, 0x8c, 0xd2, 0x81, 0x08, 0x06, 0x00, 0x01,
                    0x08, 0x00, 0x06, 0x04, 0x00, 0x01, 0x9c, 0x73,
                    0x70, 0x8c, 0xd2, 0x81, 0xc0, 0xa8, 0x01, 0x64,
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0xa8,
                    0x01, 0x63
                };
                /* use pf_0 to xmit */
                i40e_packet_send(i, pkt, sizeof(pkt));
            } else {
                /* broadcast PING packet with MAC address, will receive ARP response but we ignore */
                u8 pkt[] = {
                    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x9c, 0x73,
                    0x70, 0x8c, 0xd2, 0x82, 0x08, 0x06, 0x00, 0x01,
                    0x08, 0x00, 0x06, 0x04, 0x00, 0x01, 0x9c, 0x73,
                    0x70, 0x8c, 0xd2, 0x82, 0xc0, 0xa8, 0x02, 0x64,
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0xa8,
                    0x02, 0x63
                };
                /* use pf_0 to xmit */
                i40e_packet_send(i, pkt, sizeof(pkt));
            }

            sleep(1);
            pthread_mutex_unlock(&rpf_mutex);
        }
    }

    return;
}

void i40ecs_test_start(void)
{
    printf("I40ECS Test Start\n");
    tcpip_init(NULL, NULL);

    g_socketfd = lwip_socket(AF_INET, SOCK_DGRAM, 0);
    pthread_mutex_init(&rpf_mutex, NULL);
    int ret = pthread_create(&td, NULL, i40e_tx_rx_test, NULL);
}

void i40ecs_test_stop(void)
{
    printf("I40ECS Test Stop\n");
    pthread_cancel(td);
}