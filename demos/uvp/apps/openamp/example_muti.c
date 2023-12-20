#include "openamp/open_amp.h"
#include "prt_buildef.h"
#include "prt_proxy_ext.h"

#define RPMSG_ENDPOINT_NAME "console"
#define RPMSG_RPC_SERVICE_NAME "rpmsg-rpc-proxy"

static struct rpmsg_endpoint g_ept_default;
static struct rpmsg_endpoint g_ept_rpc;

extern int rpmsg_client_cb(struct rpmsg_endpoint *ept,
                    void *data, size_t len,
                    uint32_t src, void *priv);
extern void rpmsg_set_default_ept(struct rpmsg_endpoint *ept);

static void rpmsg_service_unbind(struct rpmsg_endpoint *ep)
{
    rpmsg_destroy_ept(ep);
}

int send_message(unsigned char *message, int len)
{
    return rpmsg_send(&g_ept_default, message, len);
}

static char *g_s1 = "Hello, UniProton! \r\n";
static int rpmsg_endpoint_cb_default(struct rpmsg_endpoint *ept, void *data, size_t len, uint32_t src, void *priv)
{
    send_message((void *)g_s1, strlen(g_s1) * sizeof(char));

    return OS_OK;
}

int rpmsg_endpoint_init(struct rpmsg_device *rdev)
{
    int ret = rpmsg_create_ept(&g_ept_default, rdev, RPMSG_ENDPOINT_NAME,
                    0xF, RPMSG_ADDR_ANY,
                    rpmsg_endpoint_cb_default, rpmsg_service_unbind);
    if (ret) {
        printf("default endpoint init fail!\n");
        return ret;
    }

    ret = rpmsg_create_ept(&g_ept_rpc, rdev, RPMSG_RPC_SERVICE_NAME,
                    0xFE, RPMSG_ADDR_ANY,
                    rpmsg_client_cb, rpmsg_service_unbind);
    if (ret) {
        printf("rpc endpoint init fail!\n");
        return ret;
    }

    rpmsg_set_default_ept(&g_ept_rpc);

    return ret;
}

void example_init()
{
    volatile int ret1 = is_rpmsg_ept_ready(&g_ept_default);
    volatile int ret2 = is_rpmsg_ept_ready(&g_ept_rpc);
    while (ret1 == 0 || ret2 == 0) {
        ret1 = is_rpmsg_ept_ready(&g_ept_default);
        ret2 = is_rpmsg_ept_ready(&g_ept_rpc);
        printf("check default endpoint is ready: ret1: %d\n", ret1);
        printf("check rpc endpoint is ready: ret2: %d\n", ret2);
        __asm__ __volatile__ ("mfence");
    }
}
