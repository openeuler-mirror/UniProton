/* ----------------------------------------------------------------------------
 * Copyright (c) Huawei Technologies Co., Ltd. 2013-2020. All rights reserved.
 * Description: Minimal commandline debug helpers for UniProton LwM2M core.
 * --------------------------------------------------------------------------- */

#ifndef LWM2M_UNIPROTON_COMMANDLINE_H
#define LWM2M_UNIPROTON_COMMANDLINE_H

#include <stdint.h>
#include <stdio.h>

void output_buffer(FILE *stream, uint8_t *buffer, int length, int indent);

#endif
