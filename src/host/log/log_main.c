/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024. All rights reserved
 *
 * SPDX-License-Identifier: GPL-2.0
 */

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/file.h>
#include <linux/kthread.h>
#include <linux/device.h>
#include <linux/types.h>
#include <linux/io.h>
#include <linux/err.h>
#include <linux/vmalloc.h>
#include <linux/sched.h>
#include <linux/printk.h>
#include <linux/jiffies.h>
#include <linux/string.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include "log_main.h"
#include "log_list.h"

/* uint32_t [0, 4294967295] 条日志, 按照每秒1w条日志计算, 需要连续运行5天才会翻转 */
static struct task_struct *g_worker_thread = NULL;
static void *g_ring_buffer = NULL;
static uint8_t g_flag_valid;
static uint8_t g_tmp_valid_flags[BUFFER_BLOCK_NUM];

/**
 * struct log_mem - internal memory structure
 * @phy_addr: physical address of the memory region
 * @size: total size of the memory region
 */
static struct log_mem g_log_mem[LOG_MEM_MAX] = {0};

static struct class *g_log_class;
static int g_log_major;

#ifdef DEBUG
static uint64_t g_pintk_count = 0;
#define VPRINTK_DEBUG_FMT   "[%llu][Uni][%llu.%09llu][seq:%llu][pid:%u] %s"
#else
#define VPRINTK_FMT         "[Uni][%llu.%09llu][seq:%llu][pid:%u] %s"
#endif

static int vprintk_emit_wrapper(int facility, int level, ...)
{
    va_list args;
    int ret;

    va_start(args, level);
#ifdef DEBUG
    ret = vprintk_emit(facility, level, NULL, VPRINTK_DEBUG_FMT, args);
#else
    ret = vprintk_emit(facility, level, NULL, VPRINTK_FMT, args);
#endif
    va_end(args);
    return ret;
}

/* 至少输出 emit_num 条日志，0代表全部输出 */
static void log_emit(uint32_t emit_num)
{
    int ret, i;
    uint32_t total_emit_num;
    struct log_elem_content log_elem;
    struct log_header *cur_log;

    uint32_t count = 0;
    while (pop_log_elem(&log_elem) != -1) {
        count++;
        for (i = 0; i < log_elem.log_num; i++) {
            cur_log = ((struct log_header *)(log_elem.log_mem + i * BUFFER_BLOCK_SIZE));
            cur_log->log_content[cur_log->len] = '\0';
#ifdef DEBUG
            g_pintk_count++;
            ret = vprintk_emit_wrapper((int)cur_log->facility, (int)cur_log->level, g_pintk_count, cur_log->sec,
                cur_log->nano_sec, cur_log->sequence_num, cur_log->task_pid, cur_log->log_content);
#else
            ret = vprintk_emit_wrapper((int)cur_log->facility, (int)cur_log->level, cur_log->sec, cur_log->nano_sec,
                cur_log->sequence_num, cur_log->task_pid, cur_log->log_content);
#endif
        }
        total_emit_num += log_elem.log_num;
        if (log_elem.log_mem != NULL) {
            vfree(log_elem.log_mem);
        } else {
            printk(KERN_ERR "[logWorker] log mem is NULL?\n");
        }

        if (emit_num != 0 && total_emit_num > emit_num) {
            break;
        }
    }
    /* TODO 避免连续输出日志, 每100条等待10ms ? */
    return;
}

static uint8_t reverse_flag(uint8_t flag) 
{
    if (flag == 0) {
        return 1;
    }
    if (flag == 1) {
        return 0;
    }
    printk(KERN_ERR "[logWorker] error: illegal flag\n");
    return flag;
}

static uint32_t check_valid_num(uint32_t head_index, uint32_t copy_num)
{
    const uint8_t *const valid_ptr = (uint8_t *)g_ring_buffer + VALID_FLAGS_OFFSET;
    uint32_t tail_copy_num, head_copy_num, valid_num;
    uint8_t valid_flag = g_flag_valid;
    uint8_t rev_valid_flag = reverse_flag(valid_flag);
    int i;

    if (head_index + copy_num > BUFFER_BLOCK_NUM) {
        // 到达结尾后，从头拷贝
        tail_copy_num = BUFFER_BLOCK_NUM - head_index;
        head_copy_num = head_index + copy_num - BUFFER_BLOCK_NUM;
    } else {
        tail_copy_num = copy_num;
        head_copy_num = 0;
    }

    valid_num = 0;
    memcpy_fromio(g_tmp_valid_flags, valid_ptr + head_index, tail_copy_num);
    for (i = 0; i < tail_copy_num; i++) {
        if (g_tmp_valid_flags[i] == valid_flag) {
            valid_num++;
        } else {
            return valid_num;
        }
    }

    if (head_copy_num != 0) {
        memcpy_fromio(g_tmp_valid_flags, valid_ptr, head_copy_num);
        for (i = 0; i < head_copy_num; i++) {
            if (g_tmp_valid_flags[i] == rev_valid_flag) {
                valid_num++;
            } else {
                return valid_num;
            }
        }
    }
    return valid_num;
}

static void copy_log_buffer(void *log_buffer, uint32_t head_index, uint32_t copy_num)
{
    uint32_t tail_copy_num, head_copy_num;
    if (head_index + copy_num > BUFFER_BLOCK_NUM) {
        // 到达结尾，从头拷贝
        tail_copy_num = BUFFER_BLOCK_NUM - head_index;
        head_copy_num = head_index + copy_num - BUFFER_BLOCK_NUM;
    } else {
        tail_copy_num = copy_num;
        head_copy_num = 0;
    }

    memcpy_fromio(log_buffer, g_ring_buffer + (head_index * BUFFER_BLOCK_SIZE), (tail_copy_num * BUFFER_BLOCK_SIZE));
    if (head_copy_num != 0) {
        memcpy_fromio(log_buffer + (tail_copy_num * BUFFER_BLOCK_SIZE), g_ring_buffer, (head_copy_num * BUFFER_BLOCK_SIZE));
    }
    return;
}

static int init_reserved_mem(void)
{
    int n = 0;
    int i, count, ret;
    struct device_node *np;
    struct device_node *node;
    struct resource res;
    uint64_t size;

    /* 即使找不到log_mem也可以正常返回，可以后续通过ioctl设置地址 */
    np = of_find_compatible_node(NULL, NULL, "oe,log_mem");
    if (np == NULL) {
        printk(KERN_ERR "[logWorker] can not find log_mem node\n");
        return 0;
    }

    count = of_count_phandle_with_args(np, "memory-region", NULL);
    if (count <= 0) {
        printk(KERN_ERR "[logWorker] reserved mem is required for log\n");
        return 0;
    }

    for (i = 0; i < count; i++) {
        node = of_parse_phandle(np, "memory-region", i);
        ret = of_address_to_resource(node, 0, &res);
        if (ret) {
            printk(KERN_ERR "[logWorker] unable to resolve memory region\n");
            return ret;
        }

        if (n >= LOG_MEM_MAX) {
            break;
        }

        size = resource_size(&res);
        if (size < SHM_MAP_SIZE) {
             printk(KERN_ERR "[logWorker] insufficient mem size, cur:0x%llx, needed:%lx\n", size, SHM_MAP_SIZE);
            return -EINVAL;
        }

        if (!request_mem_region(res.start, size, "log_mem")) {
            printk(KERN_ERR "[logWorker] Can not request log_mem, 0x%llx-0x%llx\n", res.start, res.end);
            return -EINVAL;
        }

        g_log_mem[n].phy_addr = res.start;
        g_log_mem[n].size = size;
        n++;
    }
    return 0;
}

/* 不应该返回 NULL, 返回错误码或者内存 */
static void *log_read(uint32_t *num, bool check_log, uint32_t head_index)
{
    void *vmem = NULL;
    uint32_t copy_num;

    if (num == NULL) {
        return ERR_PTR(-EFAULT);
    }
    copy_num = *num;

    if (copy_num == 0 || copy_num > BUFFER_BLOCK_NUM || head_index >= BUFFER_BLOCK_NUM) {
        *num = 0;
        return ERR_PTR(-EINVAL);
    }

    if (check_log == true) {
        copy_num = check_valid_num(head_index, copy_num);
    }
    LOAD_FENCE();

    if (copy_num == 0) {
        *num = 0;
        return ERR_PTR(-EAGAIN);
    }

    // TODO 设置每次拷贝的最大数量

    vmem = vmalloc(copy_num * BUFFER_BLOCK_SIZE);
    if (vmem == NULL) {
        *num = 0;
        return ERR_PTR(-ENOMEM);
    }

    copy_log_buffer(vmem, head_index, copy_num);
    *num = copy_num;
    return vmem;
}

/* 返回NULL代表没有尝试读取日志 */
static void *log_read_or_emit(uint32_t local_head, uint32_t curr_tail, uint32_t *read_num)
{
    void *log_mem = NULL;
    uint32_t num = curr_tail - local_head;
    uint32_t head_index = local_head % BUFFER_BLOCK_NUM;

    if (curr_tail < local_head || curr_tail > local_head + BUFFER_BLOCK_NUM) {
        printk(KERN_ERR "[logWorker] invalid tail/head, tail:%u, head:%u\n", curr_tail, local_head);
        return ERR_PTR(-EINVAL);
    }

    if (BUFFER_BLOCK_NUM - num < BUFFER_NO_CHECK_COPY_THRESHOLD) {
        // tail 即将追上一圈, 读取日志，不检查有效标记位
        // TODO 1k缓冲可能不够
        printk(KERN_ERR "[logWorker] tail is about to be full\n");
        log_mem = log_read(&num, false, head_index);
    } else if (num > BUFFER_COPY_THRESHOLD) {
        // 有足够多的日志读取
        log_mem = log_read(&num, true, head_index);
    } else if (peek_log_elem(NULL) == -1) {
        // 链表为空，继续读取日志
        if (num == 0) {
            // 没有可处理/读取的日志，休眠一下, TODO休眠暂定10ms
            schedule_timeout(HZ/100);
            LOAD_FENCE()
            return NULL;
        }
        log_mem = log_read(&num, true, head_index);
    } else {
        // 处理链表中的日志
        log_emit(LEAST_LOG_EMIT_NUM);
        return NULL;
    }

    if (IS_ERR(log_mem) && PTR_ERR(log_mem) == -EAGAIN) {
        /* 有日志阻塞, 无法读取有效日志, 优先处理链表中的日志 */
        if (peek_log_elem(NULL) == -1) {
            // 没有可处理/读取的日志，休眠一下, TODO休眠暂定10ms
            schedule_timeout(HZ/100);
        } else {
            log_emit(LEAST_LOG_EMIT_NUM);
        }
        return NULL;
    }

    *read_num = num;
    return log_mem;
}

static void log_worker_fix_overflow(uint32_t *local_head, uint32_t *local_tail, bool *is_overflow)
{
    uint32_t head = *local_head;
    uint32_t tail = *local_tail;
    bool overflow = *is_overflow;
    if (overflow) {
        /* 恢复前一轮翻转特殊处理过的head */
        head -= 2 * BUFFER_BLOCK_NUM;
        overflow = false;
    }

    if (head > (__UINT32_MAX__ - BUFFER_BLOCK_NUM)) {
        /* __UINT32_MAX__ + 1 是 2 * BUFFER_BLOCK_NUM  的整倍数 */
        /* 接近翻转的情况下，让head和tail都向前移动两圈 */
        head += 2 * BUFFER_BLOCK_NUM;
        tail += 2 * BUFFER_BLOCK_NUM;
        overflow = true;
    }
    *local_head = head;
    *local_tail = tail;
    *is_overflow = overflow;
    return;
}

static int log_worker_last_read(uint32_t local_head, uint32_t curr_tail, bool is_overflow)
{
    uint32_t read_num;
    int ret = 0;
    void *log_mem = NULL;

    log_worker_fix_overflow(&local_head, &curr_tail, &is_overflow);

    /* 如果 curr_tail 和 local_head 不一致，需要读取剩下的所有日志 */
    if (curr_tail == local_head) {
        g_worker_thread = NULL;
        return 0;
    }

    read_num = curr_tail - local_head;
    if (curr_tail < local_head || curr_tail > local_head + BUFFER_BLOCK_NUM) {
        printk(KERN_ERR "[logWorker] last read, invalid tail/head, tail:%u, head:%u\n", curr_tail, local_head);
        return -EINVAL;
    }
    /* 读取所有日志, 无论是否有效 */
    log_mem = log_read(&read_num, false, local_head % BUFFER_BLOCK_NUM);
    ret = add_log_elem((struct log_elem_content){log_mem, read_num});
    return ret;
}

int log_worker_thd(void *param)
{
    void *log_mem = NULL;
    uint32_t curr_tail, read_num;
    long ret = 0;
    bool is_overflow = 0;
    uint32_t local_head = __UINT32_MAX__ - 2 * BUFFER_BLOCK_NUM + 1;
    volatile uint32_t *const head_ptr = (uint32_t *)(g_ring_buffer + HEAD_PTR_OFFSET);
    volatile uint32_t *const tail_ptr = (uint32_t *)(g_ring_buffer + TAIL_PTR_OFFSET);
    (void)param;

    if (g_ring_buffer == NULL) {
        printk(KERN_ERR "[logWorker] error: ring buffer null\n");
        return 0;
    }

    while (!kthread_should_stop()) {
        curr_tail = *tail_ptr;
        log_worker_fix_overflow(&local_head, &curr_tail, &is_overflow);

        /* 根据head和tail, 读取日志或处理日志 */
        log_mem = log_read_or_emit(local_head, curr_tail, &read_num);
        if (IS_ERR(log_mem)) {
            ret = PTR_ERR(log_mem);
            goto LOG_EXIT;
        }
        /* 未进行日志读取 */
        if (log_mem == NULL) {
            continue;
        }
        printk(KERN_INFO "[logWorker] log head:%u, curr_tail:%u, read num:%u\n", local_head, curr_tail, read_num);
        /* 更新标记位，head指针 */
        if (local_head % BUFFER_BLOCK_NUM + read_num >= BUFFER_BLOCK_NUM) {
            g_flag_valid = reverse_flag(g_flag_valid);
        }
        local_head += read_num;
        if (is_overflow) {
            local_head -= 2 * BUFFER_BLOCK_NUM;
            is_overflow = false;
        }
        *head_ptr = local_head;
        STORE_FENCE();
        /* 读取到日志，添加进链表中 */
        ret = add_log_elem((struct log_elem_content){log_mem, read_num});
        if (ret != 0) {
            vfree(log_mem);
            goto LOG_EXIT;
        }
    }
    curr_tail = *tail_ptr;
    /* 读取剩余日志 */
    ret = log_worker_last_read(local_head, curr_tail, is_overflow);
LOG_EXIT:
    /* 输出剩余日志 */
    log_emit(0);
    return ret;
}

/**
 * 读取日志内容，日志有效标记，最好有cache（memcpy应该会有预取），读取日志有效标记后需要加读取屏障。
 * 
 * 读取tail，最好没有cache，有cache获取更新会不太及时，但是每次读取有效标记后已经会有读屏障了，tail的值足够新。
 * 
 * 写入head，最好没有cache，有cache更新会不太及时，使用MEMREMAP_WT write through，只要不读取head，写入时就不会写入cache也
 * 不会建立cache，所以不能读取head，并且和读取的内容不能在同一个cacheline里面。
 * 
 * 初始化写入全0，最好没有cache，速度快慢无所谓，MEMREMAP_WT可以满足。
*/
static int log_worker_start(uint64_t phymem)
{
    volatile uint32_t *head_ptr;
    volatile uint32_t *tail_ptr;
    printk(KERN_INFO "[logWorker] start enter, %llx, %u\n", phymem, HZ);

    g_ring_buffer = memremap(phymem, SHM_MAP_SIZE, MEMREMAP_WT);

    if (g_ring_buffer == NULL) {
        printk(KERN_ERR "[logWorker] log worker memremap fail\n");
        return -ENOMEM;
    } else {
        printk(KERN_INFO "[logWorker] ring_buffer alloc success, %p\n", g_ring_buffer);
    }

    memset_io(g_ring_buffer, 0, SHM_MAP_SIZE);
    // TODO head tail 初始化, 为测试U32翻转功能正常，后续应该删除
    head_ptr = (volatile uint32_t *)(g_ring_buffer + HEAD_PTR_OFFSET);
    tail_ptr = (volatile uint32_t *)(g_ring_buffer + TAIL_PTR_OFFSET);
    *head_ptr = __UINT32_MAX__ - 2 * BUFFER_BLOCK_NUM + 1;
    *tail_ptr = __UINT32_MAX__ - 2 * BUFFER_BLOCK_NUM + 1;
    printk(KERN_ERR "[logWorker] log worker ring start:%lx, ring end:%lx\n",
        (uintptr_t)g_ring_buffer, ((uintptr_t)g_ring_buffer + SHM_MAP_SIZE));
    STORE_FENCE();
    printk(KERN_INFO "[logWorker] head:%u tail:%u\n", *head_ptr, *tail_ptr);

    // 初始化标记位，链表
    g_flag_valid = 1;
    init_log_list();

    // 拉起线程
    g_worker_thread = kthread_run(log_worker_thd, g_ring_buffer, "Uniproton_log_worker");
    if (IS_ERR(g_worker_thread)) {
        printk(KERN_ERR "[logWorker] thread run fail, ret: %ld\n", PTR_ERR(g_worker_thread));
        g_worker_thread = NULL;
        memunmap(g_ring_buffer);
        g_ring_buffer = NULL;
        return PTR_ERR(g_worker_thread);
    }
    return 0;
}

static void log_worker_stop(void)
{
    if (g_worker_thread != NULL) {
        kthread_stop(g_worker_thread);
        g_worker_thread = NULL;
    } else {
        printk(KERN_INFO "[logWorker] g_worker_thread is NULL\n");
    }

    if (g_ring_buffer != NULL) {
        memset_io(g_ring_buffer, 0, SHM_USED_SIZE);
        STORE_FENCE();
        memunmap(g_ring_buffer);
        g_ring_buffer = NULL;
    } else {
        printk(KERN_INFO "[logWorker] g_ring_buffer is NULL\n");
    }
}

static long log_ioctl(struct file *f, unsigned int cmd, unsigned long arg) {
    int ret;
    uint64_t phymem = 0;

    if (_IOC_TYPE(cmd) != MAGIC_NUMBER) {
        return -EINVAL;
    }
    if (_IOC_NR(cmd) > IOC_MAXNR) {
        return -EINVAL;
    }

    if (cmd == IOC_START) {
        if (g_ring_buffer != NULL) {
            printk(KERN_ERR "[logWorker] log worker already start\n");
            return -EALREADY;
        }
        ret = copy_from_user(&phymem, (uint64_t __user *)arg, sizeof(uint64_t));
        printk(KERN_INFO "[logWorker] log worker phymem:0x%llx\n", phymem);
        if (ret) {
            return -EFAULT;
        }
        log_worker_stop();

        if (phymem == 0) {
            if (g_log_mem[0].phy_addr == 0) {
                printk(KERN_ERR "[logWorker] log phymem not set\n");
                return -ENOMEM;
            }
            ret = log_worker_start(g_log_mem[0].phy_addr);
        } else {
            ret = log_worker_start(phymem);
        }

        if (ret) {
            printk(KERN_ERR "[logWorker] log worker start fail, ret:%d\n", ret);
            return ret;
        }
    } else if (cmd == IOC_STOP) {
        log_worker_stop();
        return 0;
    } else {
        return -EINVAL;
    }
    return 0;
}

static const struct file_operations log_worker_fops = {
    .unlocked_ioctl = log_ioctl,
    .compat_ioctl = compat_ptr_ioctl,
    .llseek = generic_file_llseek,
};

static int register_log_dev(void)
{
    int ret;
    struct device *log_dev;

    g_log_major = register_chrdev(0, LOG_DEVICE_NAME, &log_worker_fops);
    if (g_log_major < 0) {
        ret = g_log_major;
        printk(KERN_ERR "[logWorker]register_chrdev failed (%d)\n", ret);
        goto err;
    }

    g_log_class = class_create(THIS_MODULE, LOG_DEVICE_NAME);
    if (IS_ERR(g_log_class)) {
        ret = PTR_ERR(g_log_class);
        printk(KERN_ERR "[logWorker] class_create failed (%d)\n", ret);
        goto err_class;
    }

    log_dev = device_create(g_log_class, NULL, MKDEV(g_log_major, 0),
                NULL, LOG_DEVICE_NAME);
    if (IS_ERR(log_dev)) {
        ret = PTR_ERR(log_dev);
        printk(KERN_ERR "[logWorker] device_create failed (%d)\n", ret);
        goto err_device;
    }
    return 0;

err_device:
    class_destroy(g_log_class);
err_class:
    unregister_chrdev(g_log_major, LOG_DEVICE_NAME);
err:
    return ret;
}

static void unregister_log_dev(void)
{
    device_destroy(g_log_class, MKDEV(g_log_major, 0));
    class_destroy(g_log_class);
    unregister_chrdev(g_log_major, LOG_DEVICE_NAME);
}

static int __init log_worker_init(void)
{
    int ret;
    ret = init_reserved_mem();
    if (ret) {
        return ret;
    }

    if (g_log_mem[0].phy_addr != 0) {
        ret = log_worker_start(g_log_mem[0].phy_addr);
        if (ret) {
            return ret;
        }
    }
    return register_log_dev();
}

static void __exit log_worker_exit(void)
{
    log_worker_stop();
    unregister_log_dev();
}

module_init(log_worker_init);
module_exit(log_worker_exit);

MODULE_AUTHOR("OpenEuler Embedded");
MODULE_DESCRIPTION("log worker");
MODULE_LICENSE("Dual BSD/GPL");
