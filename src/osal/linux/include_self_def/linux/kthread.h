#ifndef _LINUX_KTHREAD_H
#define _LINUX_KTHREAD_H

#include <linux/sched.h>

enum KTHREAD_BITS {
    KTHREAD_IS_PER_CPU = 0,
    KTHREAD_SHOULD_STOP,
    KTHREAD_SHOULD_PARK,
};

/**
 * kthread_create - create a kthread on the current node
 * @threadfn: the function to run in the thread
 * @data: data pointer for @threadfn()
 * @namefmt: printf-style format string for the thread name
 * @arg...: arguments for @namefmt.
 *
 * This macro will create a kthread on the current node, leaving it in
 * the stopped state.  This is just a helper for kthread_create_on_node();
 * see the documentation there for more details.
 */
#define kthread_create(threadfn, data, namefmt, arg...) kthread_create_on_node(threadfn, data, -1, namefmt, ##arg)

/**
 * kthread_run - create and wake a thread.
 * @threadfn: the function to run until signal_pending(current).
 * @data: data ptr for @threadfn.
 * @namefmt: printf-style name for the thread.
 *
 * Description: Convenient wrapper for kthread_create() followed by
 * wake_up_process().  Returns the kthread or ERR_PTR(-ENOMEM).
 */
#define kthread_run(threadfn, data, namefmt, ...)                                                                      \
    ({                                                                                                                 \
        struct task_struct *__k = kthread_create(threadfn, data, namefmt, ##__VA_ARGS__);                              \
        if (!IS_ERR(__k))                                                                                              \
            wake_up_process(__k);                                                                                      \
        __k;                                                                                                           \
    })

struct task_struct *kthread_create_on_node(int (*threadfn)(void *data), void *data, int node, const char namefmt[],
                                           ...);
bool kthread_should_stop(void);
int kthread_stop(struct task_struct *k);

#endif /* _LINUX_KTHREAD_H */