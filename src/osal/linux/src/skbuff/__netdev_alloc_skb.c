#include <linux/skbuff.h>
#include <string.h>

struct sk_buff *__netdev_alloc_skb(struct net_device *dev, unsigned int len,
    gfp_t gfp_mask)
{
    struct sk_buff *skb;
    void *data;

    len += NET_SKB_PAD;
    len += SKB_DATA_ALIGN(sizeof(struct skb_shared_info));
    len = SKB_DATA_ALIGN(len);

    data = PRT_MemAlloc(OS_MID_APP, OS_MEM_DEFAULT_FSC_PT, len);
    if (!data) {
        return NULL;
    }

    skb = PRT_MemAlloc(OS_MID_APP, OS_MEM_DEFAULT_FSC_PT, sizeof(struct sk_buff));
    if (!skb) {
        PRT_MemFree(OS_MID_APP, data);
        return NULL;
    }

    memset(skb, 0, offsetof(struct sk_buff, tail));
    len -= SKB_DATA_ALIGN(sizeof(struct skb_shared_info));
    skb->truesize = SKB_TRUESIZE(len);
    skb->head = data;
    skb->data = data;
    skb_reset_tail_pointer(skb);
    skb_set_end_offset(skb, len);

    skb_reserve(skb, NET_SKB_PAD);
    skb->dev = dev;

    return skb;
}