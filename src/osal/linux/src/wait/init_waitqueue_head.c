#include <linux/wait.h>
#include "prt_sem.h"
#include "prt_list_external.h"

void init_waitqueue_head(struct wait_queue_head *wq_head)
{
    INIT_LIST_OBJECT(&(wq_head->waitList));
    return;
}