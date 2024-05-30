#ifndef _LOG_LIST_H_
#define _LOG_LIST_H_

#include <linux/printk.h>

#ifdef DEBUG
#define LOG_DEBUG(fmt, ...) printk(KERN_INFO fmt, ##__VA_ARGS__)
#else
#define LOG_DEBUG(fmt, ...)
#endif

struct log_elem_content {
    void *log_mem;
    uint32_t log_num;
};

extern void init_log_list(void);

extern long add_log_elem(struct log_elem_content cont);

extern long peek_log_elem(struct log_elem_content *cont);

extern long pop_log_elem(struct log_elem_content *cont);

#endif
