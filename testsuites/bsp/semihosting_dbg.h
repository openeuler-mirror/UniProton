/*
 * Copyright (c) 2009-2022 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Create: 2023-7-5
 * Description: semihosting dbg头文件
 */
#ifndef SEMIHOSTING_DBG_H
#define SEMIHOSTING_DBG_H

#include "prt_typedef.h"

#ifdef __cplusplus
extern "C"
{
#endif

int SemihostingDbgWrite(const char *buf, int nbytes);

#ifdef __cplusplus
}
#endif

#endif /* SEMIHOSTING_DBG_H */
