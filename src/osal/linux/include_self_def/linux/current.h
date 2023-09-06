#ifndef _LINUX_CURRENT_H
#define _LINUX_CURRENT_H

#include <linux/sched.h>

struct task_struct *get_current(void);

#define current get_current()

#endif