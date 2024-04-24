/*
 * Copyright (c) 2024-2024 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 * 	http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * See the Mulan PSL v2 for more details.
 * Create: 2024-04-18
 * Description: soem demo
 */

#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include "ethercat.h"

#define EC_TIMEOUTMON 500

static char IOmap[4096];
static OSAL_THREAD_HANDLE thread1;
static int expectedWKC;
static boolean needlf;
static volatile int wkc;
static boolean inOP;
static uint8 currentgroup = 0;

#pragma pack(1)
typedef struct {
    volatile uint16_t ControlWord;
    volatile int32_t TargetPos;
    volatile uint8_t TargetMode;
} PDO_output;
#pragma pack()

#pragma pack(1)
typedef struct {
    volatile uint16_t StatusWord;
    volatile int32_t CurrentPosition;
    volatile int32_t CurrentVelocity;
    volatile uint16_t ErrorCode;
    volatile uint8_t CurrentMode;
} PDO_input;
#pragma pack()

#define UNCHANGE_COUNT 10

static volatile PDO_output *g_output[EC_MAXSLAVE];
static volatile PDO_input *g_input[EC_MAXSLAVE];
static char slv_step[EC_MAXSLAVE];
static char slv_unchage[EC_MAXSLAVE] = {0};
static uint32_t target_pos[EC_MAXSLAVE] = {0};

int ASDA_setup(uint16_t slave) {
    uint16_t u16val;
    uint8_t u8val;
    uint32_t u32val;

    u8val = 0;
    ec_SDOwrite(slave, 0x1c12, 0x00, FALSE, sizeof(u8val), &u8val, EC_TIMEOUTSAFE);
    u16val = 0x1600U;
    ec_SDOwrite(slave, 0x1c12, 0x01, FALSE, sizeof(u16val), &u16val, EC_TIMEOUTSAFE);
    u8val = 1;
    ec_SDOwrite(slave, 0x1c12, 0x00, FALSE, sizeof(u8val), &u8val, EC_TIMEOUTSAFE);

    u8val = 0;
    ec_SDOwrite(slave, 0x1600, 0x00, FALSE, sizeof(u8val), &u8val, EC_TIMEOUTSAFE);
    u32val = 0x60400010;
    ec_SDOwrite(slave, 0x1600, 0x01, FALSE, sizeof(u32val), &u32val, EC_TIMEOUTSAFE);
    u32val = 0x607A0020;
    ec_SDOwrite(slave, 0x1600, 0x02, FALSE, sizeof(u32val), &u32val, EC_TIMEOUTSAFE);
    u32val = 0x60600008;
    ec_SDOwrite(slave, 0x1600, 0x03, FALSE, sizeof(u32val), &u32val, EC_TIMEOUTSAFE);
    u8val = 3;
    ec_SDOwrite(slave, 0x1600, 0x00, FALSE, sizeof(u8val), &u8val, EC_TIMEOUTSAFE);

    u8val = 0;
    ec_SDOwrite(slave, 0x1c13, 0x00, FALSE, sizeof(u8val), &u8val, EC_TIMEOUTSAFE);
    u16val = 0x1A00U;
    ec_SDOwrite(slave, 0x1c13, 0x01, FALSE, sizeof(u16val), &u16val, EC_TIMEOUTSAFE);
    u8val = 1;
    ec_SDOwrite(slave, 0x1c13, 0x00, FALSE, sizeof(u8val), &u8val, EC_TIMEOUTSAFE);

    u8val = 0;
    ec_SDOwrite(slave, 0x1A00, 0x00, FALSE, sizeof(u8val), &u8val, EC_TIMEOUTSAFE);
    u32val = 0x60410010;
    ec_SDOwrite(slave, 0x1A00, 0x01, FALSE, sizeof(u32val), &u32val, EC_TIMEOUTSAFE);
    u32val = 0x60640020;
    ec_SDOwrite(slave, 0x1A00, 0x02, FALSE, sizeof(u32val), &u32val, EC_TIMEOUTSAFE);
    u32val = 0x606C0020;
    ec_SDOwrite(slave, 0x1A00, 0x03, FALSE, sizeof(u32val), &u32val, EC_TIMEOUTSAFE);
    u32val = 0x603F0010;
    ec_SDOwrite(slave, 0x1A00, 0x04, FALSE, sizeof(u32val), &u32val, EC_TIMEOUTSAFE);
    u32val = 0x60610008;
    ec_SDOwrite(slave, 0x1A00, 0x05, FALSE, sizeof(u32val), &u32val, EC_TIMEOUTSAFE);
    u8val = 5;
    ec_SDOwrite(slave, 0x1A00, 0x00, FALSE, sizeof(u8val), &u8val, EC_TIMEOUTSAFE);

    // 设置速度
    u32val = 1000000;
    ec_SDOwrite(slave, 0x6081, 0x00, FALSE, sizeof(u32val), &u32val, EC_TIMEOUTSAFE);
}

static void ASDA_control(int pos)
{
    volatile PDO_input *input = g_input[pos];
    volatile PDO_output *output = g_output[pos];

    if (input->StatusWord & 0x0008) {
        output->ControlWord = 0x80;
        output->TargetMode = 0x01;
        goto EXIT;
    }

    switch (slv_step[pos]) {
        case 1:
            if ((input->StatusWord & 0x06F) == 0x021) {
                slv_step[pos] = 2;
            }
            output->ControlWord = 0x06;
            output->TargetPos = input->CurrentPosition;
            break;
        case 2:
            if ((input->StatusWord & 0x06F) == 0x023) {
                slv_step[pos] = 3;
            }
            output->ControlWord = 0x07;
            break;
        case 3:
            if ((input->StatusWord & 0x06F) == 0x027) {
                slv_step[pos] = 4;
            }
            output->ControlWord = 0x0f;
            break;
        case 4:
            if ((input->StatusWord & 0x06F) != 0x027) {
                slv_step[pos] = 1;
                printf("StatusWord%d = 0x%x\n", pos, input->StatusWord);
                printf("ErrorCode%d = 0x%x\n", pos, input->ErrorCode);
            }
            if ((input->StatusWord & 0x400) != 0) {
                if (slv_unchage[pos] > 0) {
                    slv_unchage[pos]--;
                } else {
                    slv_step[pos] = 5;
                }
            }
            output->ControlWord = 0x1f;
            output->TargetMode = 0x01;
            output->TargetPos = target_pos[pos];
            break;
        case 5:
            slv_step[pos] = 3;
            output->ControlWord = 0x0f;
            output->TargetMode = 0x01;
            target_pos[pos] += 1000000;
            if (target_pos[pos] > 10000000) {
                target_pos[pos] = 0;
            }
            // 更新目的位置后10个周期内不会再次更新目的位置
            slv_unchage[pos] = UNCHANGE_COUNT;
            output->TargetPos = target_pos[pos];
            break;
        default:
            slv_step[pos] = 1;
            output->ControlWord = 0x80;
            output->TargetMode = 0x01;
            break;
    }
    EXIT:;
}

void simple_test(char *ifname)
{
    int i, j, slave, oloop, iloop, chk;
    needlf = FALSE;
    inOP = FALSE;
    printf("Starting simple test\n");
    /* initialise SOEM, bind socket to ifname */
    if (!ec_init(ifname)) {
        printf("No socket connection on %s\nExcecute as root\n",ifname);
        return;
    }
    printf("ec_init on %s succeeded.\n",ifname);
    /* find and auto-config slaves */

    if (ec_config_init(FALSE) <= 0) {
        printf("No slaves found!\n");
        goto TEST_END;
    }
    printf("%d slaves found and configured.\n",ec_slavecount);

    for(slave = 1; slave <= ec_slavecount; slave++) {
        ec_slave[slave].PO2SOconfig = &ASDA_setup;
    }

    ec_config_map(&IOmap);
    ec_configdc();

    for(slave = 1; slave <= ec_slavecount; slave++) {
        g_output[slave-1] = ec_slave[slave].outputs;
        g_input[slave-1] = ec_slave[slave].inputs;
    }

    printf("Slaves mapped, state to SAFE_OP.\n");
    /* wait for all slaves to reach SAFE_OP state */
    ec_statecheck(0, EC_STATE_SAFE_OP,  EC_TIMEOUTSTATE * 4);

    oloop = ec_slave[0].Obytes;
    if ((oloop == 0) && (ec_slave[0].Obits > 0)) oloop = 1;
    if (oloop > 8) oloop = 8;
    iloop = ec_slave[0].Ibytes;
    if ((iloop == 0) && (ec_slave[0].Ibits > 0)) iloop = 1;
    if (iloop > 8) iloop = 8;

    printf("segments : %d : %d %d %d %d\n", ec_group[0].nsegments, ec_group[0].IOsegment[0], ec_group[0].IOsegment[1],
        ec_group[0].IOsegment[2], ec_group[0].IOsegment[3]);

    printf("Request operational state for all slaves\n");
    expectedWKC = (ec_group[0].outputsWKC * 2) + ec_group[0].inputsWKC;
    printf("Calculated workcounter %d\n", expectedWKC);
    ec_slave[0].state = EC_STATE_OPERATIONAL;
    /* send one valid process data to make outputs in slaves happy*/
    ec_send_processdata();
    ec_receive_processdata(EC_TIMEOUTRET);
    /* request OP state for all slaves */
    ec_writestate(0);
    chk = 200;
    /* wait for all slaves to reach OP state */
    do {
        ec_send_processdata();
        ec_receive_processdata(EC_TIMEOUTRET);
        ec_statecheck(0, EC_STATE_OPERATIONAL, 50000);
    } while (chk-- && (ec_slave[0].state != EC_STATE_OPERATIONAL));

    if (ec_slave[0].state == EC_STATE_OPERATIONAL) {
        printf("Operational state reached for all slaves.\n");
        inOP = TRUE;
        /* cyclic loop */
        for (i = 1; i <= 10000; i++) {
            ec_send_processdata();
            wkc = ec_receive_processdata(EC_TIMEOUTRET);
            for(slave = 1; slave <= ec_slavecount; slave++) {
                ASDA_control(slave-1);
            }

            if (wkc < expectedWKC) {
                /* 5ms period */
                printf("\nWKC not expected:%d\n", wkc);
                osal_usleep(5000);
                continue;
            }

            printf("Processdata cycle %4d, WKC %d, slv1_curr %ld, O:", i, wkc, g_input[0]->CurrentPosition);
            for (j = 0 ; j < oloop; j++) {
                printf(" %2.2x", *(ec_slave[0].outputs + j));
            }

            printf(" I:");
            for (j = 0 ; j < iloop; j++) {
                printf(" %2.2x", *(ec_slave[0].inputs + j));
            }
            printf(" T:%"PRId64"\r",ec_DCtime);
            needlf = TRUE;
            osal_usleep(5000);
        }
        inOP = FALSE;
    } else {
        printf("Not all slaves reached operational state.\n");
        ec_readstate();
        for (i = 1; i<=ec_slavecount ; i++) {
            if (ec_slave[i].state != EC_STATE_OPERATIONAL) {
                printf("Slave %d State=0x%2.2x StatusCode=0x%4.4x : %s\n",
                    i, ec_slave[i].state, ec_slave[i].ALstatuscode, ec_ALstatuscode2string(ec_slave[i].ALstatuscode));
            }
        }
    }
    printf("\nRequest init state for all slaves\n");
    ec_slave[0].state = EC_STATE_INIT;
    /* request INIT state for all slaves */
    ec_writestate(0);
TEST_END:
    printf("End simple test, close socket\n");
    /* stop SOEM, close socket */
    ec_close();
}

void soem_asda_demo(const char *ifname)
{
   printf("SOEM (Simple Open EtherCAT Master)\nASDA demo\n");

   if (ifname != NULL) {
      simple_test(ifname);
   } else {
      printf("ifname is NULL");
   }

   printf("End program\n");
}

int soem_slave_info(const char *ifname);
int soem_simple_test(const char *ifname);

void soem_test(const char *ifname)
{
    soem_slave_info(ifname);
    soem_simple_test(ifname);
    soem_asda_demo(ifname);
}
