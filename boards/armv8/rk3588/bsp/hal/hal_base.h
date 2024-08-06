/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright (c) 2020-2021 Rockchip Electronics Co., Ltd.
 */

/** @addtogroup RK_HAL_Driver
 *  @{
 */

/** @addtogroup HAL_BASE
 *  @{
 */

#ifndef _HAL_BASE_H_
#define _HAL_BASE_H_

#include "hal_conf.h"
#include "hal_driver.h"
#include "hal_debug.h"
#include "prt_clk.h"

static inline HAL_Status HAL_DelayMs(uint32_t ms)
{
    PRT_ClkDelayMs(ms);
    return HAL_OK;
}

static inline HAL_Status HAL_DelayUs(uint32_t us)
{
    PRT_ClkDelayUs(us);
    return HAL_OK;
}

static inline HAL_Status HAL_CPUDelayUs(uint32_t us)
{
    PRT_ClkDelayUs(us);
    return HAL_OK;
}

#endif

