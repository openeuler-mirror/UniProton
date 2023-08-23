#include "openamp/open_amp.h"

#define RPMSG_RPC_SERVICE_NAME "rpmsg-rpc"
#define ADDR                    0xFE
#define DEST_ADDR               0xFF

static struct rpmsg_endpoint ept;

extern void rpmsg_set_default_ept(struct rpmsg_endpoint *ept);

extern int rpmsg_client_cb(struct rpmsg_endpoint *ept,
                    void *data, size_t len,
                    uint32_t src, void *priv);

int send_message(unsigned char *message, int len) 
{
    return 0;
}

int rpmsg_endpoint_init(struct rpmsg_device *rdev)
{
    int err;

    err = rpmsg_create_ept(&ept, rdev, RPMSG_RPC_SERVICE_NAME,
                           ADDR, DEST_ADDR,
                           rpmsg_client_cb, rpmsg_destroy_ept);
    if (err) {
        return err;
    }

    rpmsg_set_default_ept(&ept);
    return err;
}

void example_init()
{
}
