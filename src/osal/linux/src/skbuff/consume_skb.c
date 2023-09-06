#include <linux/skbuff.h>

void consume_skb(struct sk_buff *skb)
{
    PRT_MemFree(OS_MID_APP, (void *)skb->data);
    PRT_MemFree(OS_MID_APP, (void *)skb);
}