#include <netdb.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/un.h>
#include <net/if.h>
#include <sys/uio.h>
#include "lwip/ip_addr.h"
#include "lwip/sockets.h"
#include "lwip/etharp.h"
#include "lwip/tcpip.h"
#include "net.h"

static struct netif test_netif1, test_netif2;
static ip4_addr_t test_gw1, test_ipaddr1, test_netmask1;
int g_socketfd;
extern struct ethernet_api g_eth_api;

static err_t default_send(struct netif *netif, struct pbuf *p)
{
	err_t errVal;
	struct pbuf *q = NULL;
	u8_t buffer[256];
	uint16_t frameLength = 0;
	uint32_t bufferOffset = 0;
	uint32_t payloadLength = 0;

	(void)netif;
	printf("pbuf total len : %d\n", p->tot_len);

	bufferOffset = 0;

	/* copy frame from pbufs to driver buffers */
	for(q = p; q != NULL; q = q->next) {
		printf("pbuf len : %d\n", q->len);

		/* Get bytes in current lwIP buffer */
		payloadLength = q->len;

		/* copy data to Tx buffer */
		memcpy((u8_t*)((u8_t*)buffer + bufferOffset), (u8_t*)((u8_t*)q->payload), payloadLength);

		bufferOffset = bufferOffset + payloadLength;
		frameLength = frameLength + payloadLength;
	}

	if (g_eth_api.send) {
        (void)g_eth_api.send(netif, buffer, frameLength);
    }

	errVal = ERR_OK;
	return errVal;
}
#define IFNAME0 's'
#define IFNAME1 't'

static void arp_timer(void *arg)
{
	etharp_tmr();
	sys_timeout(ARP_TMR_INTERVAL, arp_timer, NULL);
}

static struct pbuf *low_level_input(struct netif *netif)
{
	struct pbuf *p = NULL, *q;
	uint32_t bufferOffset = 0;
	uint32_t payloadOffset = 0;
	uint32_t payloadLength = 0;
	uint32_t i = 0;
	int len;

	u8_t packet_rx[256];
	u32_t len_rx = 256;
	(void)netif;
	if (g_eth_api.recv) {
        len = g_eth_api.recv(netif, packet_rx, len_rx);
    }

	if(len > 0) {
		/* We allocate a pbuf chain of pbufs from the Lwip buffer pool */
		p = pbuf_alloc(PBUF_RAW, len, PBUF_POOL);
	}

	if(p != NULL) {
		bufferOffset = 0;
		printf("len : %d, totallen : %d\n", len, p->tot_len);
		for(q = p; q != NULL; q = q->next) {
			printf("----------len : %d\n", q->len);
			payloadLength = q->len;

			/* Copy remaining data in pbuf */
			memcpy(q->payload, packet_rx + bufferOffset, payloadLength);
			bufferOffset = bufferOffset + payloadLength;
			if (bufferOffset >= len) {
				break;
			}
		}
	}

	return p;
}

void ethernetif_input(void *pvParameters)
{
	struct pbuf *p;
	err_t err;

	/* move received packet into a new pbuf */
	do {
		SYS_ARCH_DECL_PROTECT(sr);

		SYS_ARCH_PROTECT(sr);
		p = low_level_input(&test_netif1);
		SYS_ARCH_UNPROTECT(sr);

		if(p == NULL) {
			return;
		}
		/* netif_add func now use tcpip_input */
		err = tcpip_input(p, &test_netif1);
		if (err != ERR_OK) {
			LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_input : IP input error\n"));
			pbuf_free(p);
			p = NULL;
		}
	} while(p != NULL);
}

static void EthThread(void *arg)
{
	printf("EthThread enter\n");
	while(1) {
		ethernetif_input(arg);
		PRT_TaskDelay(5);
	}
}

err_t ethernetif_init(struct netif *netif)
{
	LWIP_ASSERT("netif != NULL", (netif != NULL));

#if LWIP_NETIF_HOSTNAME
	/* Initialize interface hostname */
	netif->hostname = "lwip";
#endif

#if LWIP_IPV4 && LWIP_IPV6
#elif LWIP_IPV6
	netif->output_ip6 = ethip6_output;
#else
	netif->output = etharp_output;
#endif

	netif->linkoutput = default_send;
	/* set MAC hardware address length */
	netif->hwaddr_len = ETH_HWADDR_LEN;
	/* maximum transfer uint */
	netif->mtu = 1500;

	netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP;

	if (g_eth_api.init) {
		printf("ethernetif_init\n");
        (void)g_eth_api.init(netif);
    }

	sys_thread_new((char *)"Eth_if", EthThread, &test_netif1, 0x1000, 0x6);

	etharp_init();
	sys_timeout(ARP_TMR_INTERVAL, arp_timer, NULL);

	return ERR_OK;
}

void lwipInit(void)
{
    tcpip_init(NULL, NULL);
	struct netif *n;

	IP4_ADDR(&test_ipaddr1, 192,168,2,88); // c0 a8 1 58
	IP4_ADDR(&test_netmask1, 255,255,255,0);
	IP4_ADDR(&test_gw1, 192,168,0,254);

	n = netif_add(&test_netif1, &test_ipaddr1, &test_netmask1,
				  &test_gw1, NULL, &ethernetif_init, &tcpip_input);

	netif_set_default(&test_netif1);
	netif_set_up(&test_netif1);
	netif_set_link_up(&test_netif1);
}

#define MAXLINE 1024
void lwip_test_udp(void)
{
    g_socketfd = socket(AF_INET, SOCK_DGRAM, 0);
	char buffer[MAXLINE];
	int port = 10000;
	struct sockaddr_in server_addr;

	const char *device_name = "ethxxxyyy";
	int ret = setsockopt(g_socketfd, SOL_SOCKET, SO_BINDTODEVICE, device_name, strlen(device_name) + 1);

	u8_t pkt[]= "This is a UDP Client test.....\n";

	ip4_addr_t serverip;
	/* server init */
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	IP4_ADDR(&serverip, 192,168,2,99); // c0 a8 1 63
	server_addr.sin_addr.s_addr = serverip.addr;
	memset(&(server_addr.sin_zero), 0, sizeof(server_addr.sin_zero));
	sendto(g_socketfd, pkt, sizeof(pkt), 0, (struct sockaddr *)&server_addr, sizeof(struct sockaddr));

	socklen_t len = (socklen_t)sizeof(struct sockaddr);
	int n = recvfrom(g_socketfd, (char *)buffer, MAXLINE, MSG_WAITALL, (struct sockaddr *)&server_addr, &len);
	buffer[n] = '\0';
	printf("server: %s", buffer);

	close(g_socketfd);
}