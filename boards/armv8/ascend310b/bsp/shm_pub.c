#include <unistd.h>
#include "shm_pub.h"

struct shm_ipi_func *shm_ipi;

void shm_ipi_init(void)
{
#ifdef CONFIG_LINUX_IPI
    shm_ipi = &linux_shmIpi;
#elif defined CONFIG_UNIPROTON_IPI
    shm_ipi = &uniproton_shmIpi;
#endif
}

int shm_send_ipi(int vmid)
{
    struct cpu_info info = {vmid, 0};
    int ret = shm_ipi->send_ipi(info);
    return ret;
}

/* 原子等待type的值变为m */
static inline void wait_until_type(int *type, int m)
{
    int tmp;
    do {
        __asm__ volatile (
            "dmb sy\n"
            "LDAXR %w0, [%1]\n"
            : "=r" (tmp)
            : "r" (type)
            : "memory"
        );
    } while (tmp != m);
}

/* 原子等待type的值不再为m */
static inline void wait_until_not_type(int *type, int m)
{
    int tmp;
    do {
        __asm__ volatile (
            "dmb sy\n"
            "LDAXR %w0, [%1]\n"
            : "=r" (tmp)
            : "r" (type)
            : "memory"
        );
    } while (tmp == m);
}

/* 原子改写type的值为m */
static inline int change_type(int *type, int m)
{
    int tmp;
    __asm__ volatile (
        "dmb sy\n"
        "LDAXR %w0, [%1]\n"
        "STLXR %w0, %w2, [%1]\n"
        : "=&r" (tmp)
        : "r" (type), "r" (m)
        : "memory"
    );
    return tmp;
}

void raw_memcpy(void *dest, const void *src, int count)
{
    if (dest == NULL || src == NULL) {
        return;
    }
    for (int i = 0; i < count; i++) {
        *((char *)dest + i) = *((char *)src + i);
    }
}

int shm_write(void *src, unsigned int len, int data_type, shm_info_s *shm, int recId)
{
    /* 数据长度单次可以完成传输 */
    if (len < shm->max_size) {
        wait_until_not_type(&shm->op_type, SHM_OP_READY_TO_READ);
        shm->resevered = data_type;
        shm->used_size = len;
        raw_memcpy(shm->data, src, len);
        change_type(&shm->op_type, SHM_OP_READY_TO_READ);
        return shm_send_ipi(recId);
    }

    /* 数据长度较长，需要多次连续写入 */
    int i = 0;
    while (len > shm->max_size) {
        wait_until_not_type(&shm->op_type, SHM_OP_READY_TO_READ);
        raw_memcpy(shm->data, (char *)src + i * shm->max_size, shm->max_size);
        change_type(&shm->op_type, SHM_OP_READY_TO_READ);
        /* 只发送一次中断触发读取，接收方触发连续读取，读写次序由锁来控制 */
        if (i == 0) {
            shm->resevered = data_type;
            shm->used_size = len;
            int ret = shm_send_ipi(recId);
            if (ret < 0) {
                return ret;
            }
        }
        i++;
        len -= shm->max_size;
    }
    /* 最后一次拷贝 */
    wait_until_not_type(&shm->op_type, SHM_OP_READY_TO_READ);
    raw_memcpy(shm->data, (char *)src + i * shm->max_size, len);
    change_type(&shm->op_type, SHM_OP_READY_TO_READ);
    return 0;
}

int shm_read(shm_info_s *shm, void *data, unsigned int len)
{
    unsigned int tmp_len = shm->used_size;
    if (tmp_len > len || shm->op_type < 0 || shm->op_type >= SHM_OP_MAX) {
        return -1;
    }

    /* 数据长度较短，单次读取 */
    if (tmp_len <= shm->max_size) {
        wait_until_type(&shm->op_type, SHM_OP_READY_TO_READ);
        raw_memcpy(data, shm->data, shm->used_size);
        change_type(&shm->op_type, SHM_OP_READ_END);
        return 0;
    }

    /* 数据长度较长，多次读取 */
    int loop_cnt = (tmp_len + shm->max_size - 1) / shm->max_size;
    for (int i = 0; i < loop_cnt; i++) {
        wait_until_type(&shm->op_type, SHM_OP_READY_TO_READ);
        int cpy_len = tmp_len >= shm->max_size ? shm->max_size : tmp_len;
        raw_memcpy((char *)data + i * shm->max_size, shm->data, cpy_len);
        tmp_len -= shm->max_size;
        change_type(&shm->op_type, SHM_OP_READ_END);
    }

    return 0;
}