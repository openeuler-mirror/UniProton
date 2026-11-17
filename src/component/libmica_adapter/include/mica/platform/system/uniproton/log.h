/* SPDX-License-Identifier: MulanPSL-2.0 */
#ifndef MICA_UNIPROTON_LOG_H
#define MICA_UNIPROTON_LOG_H

#include "prt_typedef.h"

#ifdef __cplusplus
extern "C" {
#endif

extern U32 PRT_Printf(const char *format, ...);

/* Independent early console: never route runtime diagnostics through TTY. */
#define mica_log(...) ((void)PRT_Printf(__VA_ARGS__))

#ifdef __cplusplus
}
#endif

#endif
