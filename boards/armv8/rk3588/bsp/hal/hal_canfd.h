/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright (c) 2021 Rockchip Electronics Co., Ltd.
 */

#include "hal_conf.h"

#ifdef HAL_CANFD_MODULE_ENABLED

/** @addtogroup RK_HAL_Driver
 *  @{
 */

/** @addtogroup CANFD
 *  @{
 */

#ifndef _HAL_CANFD_H_
#define _HAL_CANFD_H_

#include "hal_def.h"

/*************************** MACRO Definition ****************************/
/** @defgroup CANFD_Exported_Definition_Group1 Basic Definition
 *  @{
 */

#define CANFD_ID_STANDARD 0
#define CANFD_ID_EXTENDED 1
#define CANFD_RTR_DATA    0
#define CANFD_RTR_REMOTE  1

typedef enum {
    CANFD_BPS_1MBAUD = 1,
    CANFD_BPS_800KBAUD,
    CANFD_BPS_500KBAUD,
    CANFD_BPS_250KBAUD,
    CANFD_BPS_200KBAUD,
    CANFD_BPS_125KBAUD,
    CANFD_BPS_100KBAUD,
    CANFD_BPS_50KBAUD,
} eCANFD_Bps;

typedef enum {
    CANFD_INT_ERR = 1,
    CANFD_INT_RX_OF,
    CANFD_INT_TX_FINISH,
    CANFD_INT_RX_FINISH,
} eCANFD_IntType;

typedef enum {
    CANFD_MODE_LOOPBACK       = 0x1,
    CANFD_MODE_LISTENONLY     = 0x2,
    CANFD_MODE_3_SAMPLES      = 0x4,
} eCANFD_Mode;

struct CANFD_BPS {
    uint32_t sjw;
    uint32_t brq;
    uint32_t tseg1;
    uint32_t tseg2;
    uint32_t tdc;
};

struct CANFD_CONFIG {
    uint32_t canfdMode;
    uint32_t canfdFilterId[6];
    uint32_t canfdFilterMask[6];
    eCANFD_Bps bps;
};

struct CANFD_MSG {
    uint16_t stdId;
    uint32_t extId;
    uint8_t ide;
    uint8_t rtr;
    uint8_t dlc;
    uint8_t data[8];
};

/***************************** Structure Definition **************************/

/** @} */
/***************************** Function Declare ******************************/
/** @defgroup CANFD_Public_Function_Declare Public Function Declare
 *  @{
 */
HAL_Status HAL_CANFD_Init(struct CAN_REG *pReg, struct CANFD_CONFIG *initStrust);
HAL_Status HAL_CANFD_Start(struct CAN_REG *pReg);
HAL_Status HAL_CANFD_Stop(struct CAN_REG *pReg);
HAL_Status HAL_CANFD_SetNBps(struct CAN_REG *pReg, eCANFD_Bps bsp);
HAL_Status HAL_CANFD_SetDBps(struct CAN_REG *pReg, eCANFD_Bps bps);
HAL_Status HAL_CANFD_Transmit(struct CAN_REG *pReg, struct CANFD_MSG *TxMsg);
HAL_Status HAL_CANFD_Receive(struct CAN_REG *pReg, struct CANFD_MSG *RxMsg);
uint32_t HAL_CANFD_GetInterrupt(struct CAN_REG *pReg);
uint32_t HAL_CANFD_GetErrInterruptMaskCombin(eCANFD_IntType type);

/** @} */

#endif

/** @} */

/** @} */

#endif /* HAL_CANFD_MODULE_ENABLED */
