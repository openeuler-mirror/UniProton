#include "nb_direct_link.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define CONNECTIVITY_FRAME_PREFIX "IOTOTA1"
#define CONNECTIVITY_FRAME_LINE_LEN 64U

static int g_direct_link_fd = -1;

static uint32_t min_u32(uint32_t a, uint32_t b)
{
    return (a < b) ? a : b;
}

static int read_full(int fd, uint8_t *buf, uint32_t len)
{
    uint32_t done = 0;
    int ret;

    while (done < len) {
        ret = (int)recv(fd, &buf[done], len - done, 0);
        if (ret <= 0) {
            return -1;
        }
        done += (uint32_t)ret;
    }
    return 0;
}

static int write_full(int fd, const uint8_t *buf, uint32_t len)
{
    uint32_t done = 0;
    int ret;

    while (done < len) {
        ret = (int)send(fd, &buf[done], len - done, 0);
        if (ret <= 0) {
            return -1;
        }
        done += (uint32_t)ret;
    }
    return 0;
}

static int read_line(int fd, char *line, uint32_t line_len)
{
    uint32_t pos = 0;
    char ch;
    int ret;

    if (line == NULL || line_len == 0) {
        return -1;
    }

    while (pos + 1 < line_len) {
        ret = (int)recv(fd, &ch, 1, 0);
        if (ret <= 0) {
            return -1;
        }
        if (ch == '\n') {
            line[pos] = '\0';
            return 0;
        }
        line[pos++] = ch;
    }
    line[pos] = '\0';
    return -1;
}

int nb_direct_link_open(const char *host, uint16_t port)
{
    struct sockaddr_in addr;
    int fd;

    if (host == NULL || port == 0) {
        return -1;
    }

    if (g_direct_link_fd >= 0) {
        return 0;
    }

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        return -1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (inet_pton(AF_INET, host, &addr.sin_addr) != 1) {
        (void)close(fd);
        return -1;
    }

    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
        (void)close(fd);
        return -1;
    }

    g_direct_link_fd = fd;
    return 0;
}

void nb_direct_link_close(void)
{
    if (g_direct_link_fd >= 0) {
        (void)close(g_direct_link_fd);
        g_direct_link_fd = -1;
    }
}

int nb_direct_link_request(const char *channel, const uint8_t *tx, uint32_t tx_len,
    char *rx_channel, uint32_t rx_channel_len, uint8_t *rx, uint32_t *rx_len)
{
    char line[CONNECTIVITY_FRAME_LINE_LEN];
    char prefix[16];
    unsigned int channel_len;
    unsigned int resp_channel_len;
    unsigned int resp_len;
    uint32_t copy_len;
    int ret;

    if (g_direct_link_fd < 0 || channel == NULL || tx == NULL || rx_channel == NULL || rx_channel_len == 0 ||
        rx == NULL || rx_len == NULL) {
        return -1;
    }

    channel_len = (unsigned int)strlen(channel);
    ret = snprintf(line, sizeof(line), "%s %u %u\n", CONNECTIVITY_FRAME_PREFIX, channel_len, (unsigned int)tx_len);
    if (ret <= 0 || (uint32_t)ret >= sizeof(line)) {
        return -1;
    }
    if (write_full(g_direct_link_fd, (const uint8_t *)line, (uint32_t)strlen(line)) != 0 ||
        write_full(g_direct_link_fd, (const uint8_t *)channel, channel_len) != 0 ||
        write_full(g_direct_link_fd, tx, tx_len) != 0) {
        return -1;
    }

    if (read_line(g_direct_link_fd, line, sizeof(line)) != 0 ||
        sscanf(line, "%15s %u %u", prefix, &resp_channel_len, &resp_len) != 3 ||
        strcmp(prefix, CONNECTIVITY_FRAME_PREFIX) != 0 || resp_channel_len + 1U > rx_channel_len) {
        return -1;
    }
    if (read_full(g_direct_link_fd, (uint8_t *)rx_channel, resp_channel_len) != 0) {
        return -1;
    }
    rx_channel[resp_channel_len] = '\0';

    copy_len = min_u32(*rx_len, (uint32_t)resp_len);
    if (read_full(g_direct_link_fd, rx, copy_len) != 0) {
        return -1;
    }
    if ((uint32_t)resp_len > copy_len) {
        uint8_t discard[32];
        uint32_t left = (uint32_t)resp_len - copy_len;
        while (left > 0) {
            uint32_t step = min_u32(left, sizeof(discard));
            if (read_full(g_direct_link_fd, discard, step) != 0) {
                return -1;
            }
            left -= step;
        }
    }

    *rx_len = copy_len;
    return 0;
}
