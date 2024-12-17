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
 * Create: 2024-03-15
 * Description: RingBuf
 */

#ifndef PRT_RINGBUF_H
#define PRT_RINGBUF_H

#include "prt_typedef.h"
#include "prt_atomic.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

typedef enum {
    RBUF_UNINIT,          // Ringbuf is not inited
    RBUF_INITED           // Ringbuf is inited
} RingbufStatus;

// Ringbuf information structure.
typedef struct {
    U32 startIdx;            // Ringbuf read index
    U32 endIdx;              // Ringbuf write index
    U32 size;                // Ringbuf total size
    U32 remain;              // Ringbuf free size
    struct PrtSpinLock lock; // Lock for read and write
    RingbufStatus status;    // Ringbuf status
    char *fifo;              // Buf to store data
} Ringbuf;

/**
 * @brief Init a ringbuf.
 *
 * @par Description:
 * This API is used to init a ringbuf.
 * @attention
 * The size must not be bigger than the fifo's actual size.
 *
 * @param  ringbuf        [OUT] Ringbuf control block.
 * @param  fifo           [IN] Data buf address.
 * @param  size           [IN] Data buf size.
 *
 * @retval #OS_ERROR      Init failed, check the legality of function parameters.
 * @retval #OS_OK         Init success.
 *
 * @par Dependency:
 * <ul><li>prt_ringbuf.h: the header file that contains the API declaration.</li></ul>
 * @see PRT_RingbufInit
 */
extern U32 PRT_RingbufInit(Ringbuf *ringbuf, char *fifo, U32 size);

/**
 * @brief Reset a ringbuf.
 *
 * @par Description:
 * This API is used to reset a ringbuf to the init status.
 * @attention
 * The specific ringbuf must be inited first.
 *
 * @param  ringbuf        [IN] Ringbuf created by PRT_RingbufInit.
 *
 * @retval None.
 *
 * @par Dependency:
 * <ul><li>prt_ringbuf.h: the header file that contains the API declaration.</li></ul>
 * @see PRT_RingbufReset
 */
extern void PRT_RingbufReset(Ringbuf *ringbuf);

/**
 * @brief Write data to ringbuf.
 *
 * @par Description:
 * This API is used to write data to ringbuf.
 * @attention
 * The specific ringbuf must be inited first.
 *
 * @param  ringbuf        [IN] The ringbuf write data to.
 * @param  buf            [IN] The source buf address.
 * @param  size           [IN] The source buf size.
 *
 * @retval #U32        The actual written size.
 *
 * @par Dependency:
 * <ul><li>prt_ringbuf.h: the header file that contains the API declaration.</li></ul>
 * @see PRT_RingbufWrite
 */
extern U32 PRT_RingbufWrite(Ringbuf *ringbuf, const char *buf, U32 size);

/**
 * @brief Read data from ringbuf.
 *
 * @par Description:
 * This API is used to get data from ringbuf.
 * @attention
 * The specific ringbuf must be inited first.
 *
 * @param  ringbuf        [IN] The ringbuf read data from.
 * @param  buf            [OUT] The dest buf address.
 * @param  size           [IN] The dest buf size.
 *
 * @retval #U32        The actual read size.
 *
 * @par Dependency:
 * <ul><li>prt_ring.h: the header file that contains the API declaration.</li></ul>
 * @see PRT_RingbufRead
 */
extern U32 PRT_RingbufRead(Ringbuf *ringbuf, char *buf, U32 size);

/**
 * @brief Get a ringbuf's used size.
 *
 * @par Description:
 * This API is used to get a ringbuf's used size.
 * @attention
 * The specific ringbuf must be inited first.
 *
 * @param  ringbuf        [IN] The ringbuf address
 *
 * @retval #U32        The used size of ringbuf.
 *
 * @par Dependency:
 * <ul><li>prt_ringbuf.h: the header file that contains the API declaration.</li></ul>
 * @see PRT_RingbufUsedSize
 */
extern U32 PRT_RingbufUsedSize(Ringbuf *ringbuf);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* PRT_RINGBUF_H */
