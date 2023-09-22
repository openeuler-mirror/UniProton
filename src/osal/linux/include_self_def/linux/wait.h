#ifndef _LINUX_WAIT_H
#define _LINUX_WAIT_H

#include "prt_sem.h"
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/list.h>
#include <stdio.h>

struct wait_queue_head {
    struct TagListObject waitList;
};

typedef struct wait_queue_head wait_queue_head_t;

/**
 * wait_event - sleep until a condition gets true
 * @wq_head: the waitqueue to wait on
 * @condition: a C expression for the event to wait for
 *
 * The process is put to sleep (TASK_UNINTERRUPTIBLE) until the
 * @condition evaluates to true. The @condition is checked each time
 * the waitqueue @wq_head is woken up.
 *
 * wake_up() has to be called after changing any variable that could
 * change the result of the wait condition.
 */

#define wait_event(wq_head, condition) (void)wait_event_interruptible(wq_head, condition)

/**
 * wait_event_interruptible - sleep until a condition gets true
 * @wq_head: the waitqueue to wait on
 * @condition: a C expression for the event to wait for
 *
 * The process is put to sleep (TASK_INTERRUPTIBLE) until the
 * @condition evaluates to true or a signal is received.
 * The @condition is checked each time the waitqueue @wq_head is woken up.
 *
 * wake_up() has to be called after changing any variable that could
 * change the result of the wait condition.
 *
 * The function will return -ERESTARTSYS if it was interrupted by a
 * signal and 0 if @condition evaluated to true.
 */

#define wait_event_interruptible(wq_head, condition)                                                                   \
    ({                                                                                                                 \
        int ret = 0;                                                                                                   \
        while (!(condition)) {                                                                                         \
            ret = enter_wait_queue(&wq_head);                                                                          \
            if (ret) {                                                                                                 \
                printf("[ERROR] enter wait queue fail, %d", ret);                                                      \
                break;                                                                                                 \
            }                                                                                                          \
        }                                                                                                              \
        ret;                                                                                                           \
    })

#define wake_up_all(x) wake_up(x)
#define wake_up_interruptible(x) wake_up(x)

void init_waitqueue_head(struct wait_queue_head *wq_head);
void wake_up(struct wait_queue_head *wq_head);
U32 enter_wait_queue(struct wait_queue_head *wq_head);

#endif