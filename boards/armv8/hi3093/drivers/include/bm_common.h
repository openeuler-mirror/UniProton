/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2024-2024. All rights reserved.
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 * 	http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * See the Mulan PSL v2 for more details.
 */
#ifndef __BM_COMMON_H__
#define __BM_COMMON_H__

#include "bm_types.h"
#include "bm_uart.h"
#include "device_resource.h"

unsigned int bm_get_coreid(void);
unsigned long long bm_get_tick(void);
void udelay(unsigned long usec);

#if defined (__UNIPROTON__)
#define BM_ADDR_PROTECT 0
#else
#define BM_ADDR_PROTECT 1
#endif

#define BOOT_SYNC_USING 0x5a5a5a5a
#define BOOT_SYNC_READY 0xabababab

#define isb() __asm volatile("isb sy" : : : "memory")
#define dsb() __asm volatile("dsb sy" : : : "memory")
#define dmb() __asm volatile("dmb sy" : : : "memory")

#define max(a, b) (((a) > (b)) ? (a) : (b))
#define min(a, b) (((a) < (b)) ? (a) : (b))

#if defined (CONFIG_VIRTUAL_SERIAL) && CONFIG_VIRTUAL_SERIAL == 1
#define bm_log(fmt, ...) SRE_Printf("[core%d][%s:%d]:" fmt, bm_get_coreid(), __FUNCTION__, __LINE__, ##__VA_ARGS__)
#elif defined (__UNIPROTON__)
extern unsigned int PRT_Printf(const char *format, ...);
#define bm_log(fmt, ...) PRT_Printf("[core%d][%s:%d]:" fmt, bm_get_coreid(), __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define bm_log(fmt, ...) bm_printf("[core%d][%s:%d]:" fmt, bm_get_coreid(), __FUNCTION__, __LINE__, ##__VA_ARGS__)
#endif

#define BM_LOG_DEBUG 1

#define BM_DEBUG_LEVEL           (1 << 1)
#define BM_INFO_LEVEL            (1 << 2)
#define BM_ERROR_LEVEL           (1 << 3)

#if BM_LOG_DEBUG
#define BM_LOG_LEVEL BM_ERROR_LEVEL
#define bm_debug(level, msg...)              \
    do {                                     \
        if (level >= BM_LOG_LEVEL) {       \
            bm_log(msg); \
        }                                    \
    } while (0)
#else
#define bm_debug(level, msg...) \
    do {                        \
    } while (0)
#endif

#define BASE_CFG_UNSET 0x00
#define BASE_CFG_SET 0x01
#define BASE_CFG_DISABLE 0x00
#define BASE_CFG_ENABLE 0x01
#define base_func_paramcheck_with_ret(param, ret) \
    do { \
        if (!(param)) { \
            return ret; \
        } \
    } while (0)

/* dts convert to header file */
#define dt_device_exists(name, dev) is_enable(DT_N_S_SOC_S_##name##_P_DOMAIN_##dev##_EXISTS)

#define DEVICE_XXXX1 DEVICE_YYYY,
#define is_enable(config_macro) is_enable1(config_macro)
#define is_enable1(config_macro) is_enable2(DEVICE_XXXX##config_macro)
#define is_enable2(one_or_two_args) is_enable3(one_or_two_args 1, 0)
#define is_enable3(ignore_this, val, ...) val
/* dts convert to header file */

#if defined(BM_ADDR_PROTECT) && (BM_ADDR_PROTECT == 1)
#ifndef XFER_DATA
#define XFER_DATA __attribute__((section(".xfer_segment")))
#endif
#else
#define XFER_DATA
#endif

extern char __xfer_segment_start[], __xfer_segment_end[];

static inline int is_invalid_addr_len(unsigned int addr, unsigned int len)
{
#if defined(BM_ADDR_PROTECT) && (BM_ADDR_PROTECT == 1)
    unsigned long long tmp = (unsigned long long)addr + (unsigned long long)len;
    return ((addr >= (unsigned int)(uintptr_t)__xfer_segment_start) &&
               (addr < (unsigned int)(uintptr_t)__xfer_segment_end) &&
               (tmp > (unsigned int)(uintptr_t)__xfer_segment_start) &&
               (tmp <= (unsigned int)(uintptr_t)__xfer_segment_end))
               ? 0
               : 1;
#else
    (void)addr;
    (void)len;
    return 0;
#endif
}
#endif /* __BM_COMMON_H__ */