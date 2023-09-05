#include "openamp/open_amp.h"

#define RPMSG_ENDPOINT_NAME "console"

static struct rpmsg_endpoint g_ept;
static U32 g_receivedMsg;
static bool g_openampFlag = false;

static void rpmsg_service_unbind(struct rpmsg_endpoint *ep)
{
    rpmsg_destroy_ept(ep);
}

static int send_message(unsigned char *message, int len)
{
    return rpmsg_send(&g_ept, message, len);
}

static char *g_s1 = "Hello, UniProton! \r\n";
static int rpmsg_endpoint_cb(struct rpmsg_endpoint *ept, void *data, size_t len, uint32_t src, void *priv)
{
    g_openampFlag = true;
    send_message((void *)g_s1, strlen(g_s1) * sizeof(char));

    return OS_OK;
}

int rpmsg_endpoint_init(struct rpmsg_device *rdev)
{
    return rpmsg_create_ept(&g_ept, rdev, RPMSG_ENDPOINT_NAME,
                    0xF, RPMSG_ADDR_ANY,
                    rpmsg_endpoint_cb, rpmsg_service_unbind);
}

void example_init()
{
    send_message((void *)&g_receivedMsg, sizeof(U32));

    while (!g_openampFlag);
}