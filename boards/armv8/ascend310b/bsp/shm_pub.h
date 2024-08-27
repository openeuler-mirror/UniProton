/*
 * Copyright (c) 2023-2033 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Create: 2023-09-06
 * Description: Ushare memory info head file。
 */

#ifndef SHM_PUB_H
#define SHM_PUB_H

enum shm_optype {
    SHM_OP_START,              // 操作 初始状态，等待
    SHM_OP_READY_TO_READ,      // 操作 等待读取
    SHM_OP_READ_END,           // 操作 读取完毕
    SHM_OP_DEL,                // 操作 等待删除
    SHM_OP_MAX,
};

/* 信息区域，信息头用来VM间进行数据的说明及放置读写锁等 */
typedef struct shm_info_t {
#ifdef CONFIG_PLAT_ASCEND_310B
    volatile unsigned int max_size;          // 数据区最大可写入长度
    volatile unsigned int used_size;         // 本次传输数据长度
    volatile int op_type;                    // 该内存当前相应的操作状态
    volatile int resevered;
#else
    unsigned int max_size;          // 数据区最大可写入长度
    unsigned int used_size;         // 本次传输数据长度
    int op_type;                    // 该内存当前相应的操作状态
    int resevered;
#endif
    char data[0];                   // 数据区
} shm_info_s;

struct cpu_info {
    unsigned int vmId;
    unsigned int cpuId;
};

struct shm_ipi_func {
    int (* init)(void);
    int (* send_ipi)(struct cpu_info info);
};

extern struct shm_ipi_func linux_shmIpi;
extern struct shm_ipi_func uniproton_shmIpi;

/* 统一的ipi初始化接口 */
void shm_ipi_init(void);
int shm_send_ipi(int vmid);

/**
 * @brief 共享内存写入通用接口
 *
 * @param src [input] 写入共享内存的内容起始指针
 * @param len [input] 写入共享内存的内容长度（该长度可以大于共享内存的实际空间）
 * @param data_type [input] 本次内容约定的数据或处理类型，由业务间约定
 * @param shm [input] 共享内存信息结构体头
 * @param recId [input] 当前为接收方的vm_id
 * @return 返回0代表写入成功
 */
int shm_write(void *src, unsigned int len, int data_type, shm_info_s *shm, int recId);

/**
 * @brief 共享内存读取通用接口
 *
 * @param shm [input] 共享内存信息结构体头
 * @param data [input] 保存读取到的数据空间的头
 * @param len [input] 保存读取到的数据空间的最大长度
 * @return 返回0表示读取成功。读取失败不会清空data
 */
int shm_read(shm_info_s *shm, void *data, unsigned int len);

#endif