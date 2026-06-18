/*
 * Copyright (c) 2026-2026 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * Description: CMSIS test log output helper.
 *
 * The CMSIS tests use only CMSIS APIs for functional validation. PRT_Printf is
 * used here only as the sd3403 serial log sink so test results are visible on
 * the board UART.
 */

#ifndef CMSIS_TEST_LOG_H
#define CMSIS_TEST_LOG_H

extern unsigned int PRT_Printf(const char *format, ...);

#define CMSIS_TEST_LOG(...) ((void)PRT_Printf(__VA_ARGS__))

#endif
