#ifndef _LOG_WORKER_H_
#define _LOG_WORKER_H_

#define MCS_DEVICE_NAME     "/dev/mcs"

// 最大内存块数量, 实际一块会使用
#define LOG_MEM_MAX         1
#define LOG_DEVICE_NAME     "logW"

#define BUFFER_BLOCK_NUM 0x4000
#define BUFFER_BLOCK_SIZE 0x400 /* 1KB */
// 未读日志数量超过该数值，进行一次读取
#define BUFFER_COPY_THRESHOLD 0x200

// 剩余缓存块低于该数值，读取日志时不检查有效标记位
#define BUFFER_NO_CHECK_COPY_THRESHOLD 0x400

// 空闲时一次性至少输出该数值的日志
#define LEAST_LOG_EMIT_NUM 0x200

/*
    TOTAL size = 0x4000 * 0x400 + 0x4000 + 0x100 * 2 = 0x1004200
    +=========
    |
    |
    |   ring buffer size: BUFFER_BLOCK_NUM * BUFFER_BLOCK_SIZE = 16MB
    |
    |
    +--------
    |
    |   valid flag size: BUFFER_BLOCK_NUM * sizeof(U8) = 16 KB
    |
    +-----------
    |   head (one cacheline 64 byte)
    |   tail (one cacheline 64 byte)
    +=========
*/
#define SHM_MAP_SIZE        0x1100000UL
#define SHM_USED_SIZE       0x1004200UL // ~16MB
#define VALID_FLAGS_OFFSET  0x1000000UL
#define VALID_FLAGS_SIZE    0x4000UL
#define HEAD_PTR_OFFSET     0x1004000UL
#define TAIL_PTR_OFFSET     0x1004100UL

// TODO 适配其他架构/用linux自带的...
// armv8 memory fence
#define STORE_FENCE() __asm__ __volatile__ ("dmb st" : : : "memory");
#define M_FENCE() __asm__ __volatile__ ("dmb sy" : : : "memory");
#define LOAD_FENCE() __asm__ __volatile__ ("dmb ld" : : : "memory");

// ioctl cmd
#define MAGIC_NUMBER		'L'
#define IOC_START		_IOW(MAGIC_NUMBER, 0, uint64_t)
#define IOC_STOP		_IO(MAGIC_NUMBER, 1)
#define IOC_MAXNR		1

struct log_header {
    uint64_t sec;
    uint64_t nano_sec;
    uint64_t sequence_num;
    uint32_t task_pid;
    uint16_t len;
    uint8_t facility;
    uint8_t level;
    uint8_t log_content[];
};

struct log_mem {
    uint64_t phy_addr;
    uint64_t size;
};


#endif /* _LOG_WORKER_H_ */
