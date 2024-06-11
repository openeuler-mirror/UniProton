#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/un.h>
#include <net/if.h>
#include <sys/uio.h>
#include "net.h"

#define MAXLINE 100
#define UDPPORT 8888

static void build_addr(struct in_addr *addr, int b1, int b2,
                       int b3, int b4)
{
    char *cs = (char *)addr;

    cs[0] = b1 & 0xff;
    cs[1] = b2 & 0xff;
    cs[2] = b3 & 0xff;
    cs[3] = b4 & 0xff;
}

/* udp client */
int proxy_udp_client()
{
    int cfd = 0;
    ssize_t sret = 0;
    socklen_t salen = 0;
    char str[] = "hello server!";
    char buf[MAXLINE];
    struct sockaddr_in saddr;

    if((cfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        printf("socket() error");
        return -1;
    }

    printf("test_udp_client-----%d\n", cfd);


    salen = sizeof(struct sockaddr_in);
    memset(&saddr, 0, salen);
    saddr.sin_family = AF_INET;
    saddr.sin_port = UDPPORT;
    build_addr(&saddr.sin_addr , 127, 0, 0, 1);

    sret = sendto(cfd, str, sizeof(str), 0, (struct sockaddr *)&saddr, salen);
    printf("sendto ret: %ld, addrlen: %d, data: %s\n", sret, salen, str);
    if(sret < 0) {
        printf("sendto() error");
        close(cfd);
        return -1;
    }

    sret = recvfrom(cfd, buf, sizeof(buf), 0, (struct sockaddr *)&saddr, &salen);
    printf("recvfrom ret: %ld, addrlen: %d, data: %s\n", sret, salen, buf);
    if(sret < 0) {
        printf("recvfrom() error");
        close(cfd);
        return -1;
    }

    close(cfd);
    return 0;
}