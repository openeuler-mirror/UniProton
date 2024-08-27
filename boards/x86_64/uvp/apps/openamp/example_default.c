#include "openamp/open_amp.h"
#include "prt_buildef.h"
#include "prt_proxy_ext.h"

#define RPMSG_ENDPOINT_NAME "console"

static struct rpmsg_endpoint g_ept;

extern int rpmsg_client_cb(struct rpmsg_endpoint *ept,
                    void *data, size_t len,
                    uint32_t src, void *priv);
extern void rpmsg_set_default_ept(struct rpmsg_endpoint *ept);

extern char *g_printf_buffer;

static void rpmsg_service_unbind(struct rpmsg_endpoint *ep)
{
    rpmsg_destroy_ept(ep);
}

int send_message(unsigned char *message, int len)
{
    return PRT_ProxyWriteStdOut(message, len);
}

int rpmsg_endpoint_init(struct rpmsg_device *rdev)
{
    int err;
    err = rpmsg_create_ept(&g_ept, rdev, RPMSG_ENDPOINT_NAME,
                    0xF, RPMSG_ADDR_ANY,
                    rpmsg_client_cb, rpmsg_service_unbind);
    if (err) {
        return err;
    }

    rpmsg_set_default_ept(&g_ept);

    return err;
}

void example_init()
{
    volatile int ret = is_rpmsg_ept_ready(&g_ept);
    while (ret == 0) {
        ret = is_rpmsg_ept_ready(&g_ept);
        __asm__ __volatile__ ("mfence");
    }
    g_printf_buffer = (char *)malloc(PRINTF_BUFFER_LEN);
}
