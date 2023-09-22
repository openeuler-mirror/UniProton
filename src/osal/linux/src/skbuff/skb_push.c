#include <linux/skbuff.h>

void *skb_push(struct sk_buff *skb, unsigned int len)
{
    skb->data -= len;
    skb->len += len;
    if (skb->data < skb->head) {
        printf("Fatal Error: skb data exceeds!\n");
        PRT_ErrHandle("skb_push.c", __LINE__, OS_ERRNO_BUILD_FATAL(0x11, 1), 0, NULL);
    }

    return skb->data;
}