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
 * Create: 2023-06-09
 * Description: 信号模块内部头文件
 */
#ifndef PRT_SIGNAL_EXTERNAL_H
#define PRT_SIGNAL_EXTERNAL_H

#include "prt_task_external.h"

extern void OsInitSigVectors(struct TagTskCb *taskCb);
extern void OsSigDefaultHandler(int signum);
extern void OsHandleUnBlockSignal(struct TagTskCb *runTsk);

#endif /* PRT_SIGNAL_EXTERNAL_H */
