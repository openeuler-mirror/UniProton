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
 * Create: 2024-01-23
 * Description: openamp backend
 */

#include "openamp/open_amp.h"
#include "openamp_common.h"

#ifdef __cplusplus
extern "C" {
#endif

extern int rpmsg_service_init(void);

/*
 * @brief Initialize RPMsg backend
 *
 * @param io   Shared memory IO region. This is an output parameter providing
 *             a pointer to an actual shared memory IO region structure.
 *             Caller of this function shall pass an address at which the
 *             pointer to the shared memory IO region structure is stored.
 * @param vdev Pointer to the virtio device initialized by this function.
 *
 * @retval 0 Initialization successful
 * @retval <0 Initialization error reported by OpenAMP
 */
extern int rpmsg_backend_init(struct metal_io_region **io, struct virtio_device *vdev);

#ifdef __cplusplus
}
#endif
