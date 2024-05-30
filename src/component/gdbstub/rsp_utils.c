#include "prt_typedef.h"
#include "ringbuffer.h"
#include "rsp_utils.h"
#include "gdbstub_common.h"

STUB_TEXT int OsGdbChar2Hex(char c, U8 *x)
{
    if (c >= '0' && c <= '9') {
        *x = c - '0';
    } else if (c >= 'a' && c <= 'f') {
        *x = c - 'a' + 10;
    } else if (c >= 'A' && c <= 'F') {
        *x = c - 'A' + 10;
    } else {
        return -1;
    }

    return 0;
}

STUB_TEXT int OsGdbHex2Char(U8 x, char *c)
{
    if (x <= 9) {
        *c = x + '0';
    } else if (x <= 15) {
        *c = x - 10 + 'a';
    } else {
        return -1;
    }

    return 0;
}

STUB_TEXT int OsGdbHex2Bin(const char *hex, int hexlen, U8 *buf, int buflen)
{
    U8 dec;

    if (buflen < hexlen / 2 + hexlen % 2) {
        return 0;
    }

    /* if hexlen is uneven, insert leading zero nibble */
    if (hexlen % 2) {
        if (OsGdbChar2Hex(hex[0], &dec) < 0) {
            return 0;
        }
        buf[0] = dec;
        hex++;
        buf++;
    }

    /* regular hex conversion */
    for (int i = 0; i < hexlen / 2; i++) {
        if (OsGdbChar2Hex(hex[2 * i], &dec) < 0) {
            return 0;
        }
        buf[i] = dec << 4;

        if (OsGdbChar2Hex(hex[2 * i + 1], &dec) < 0) {
            return 0;
        }
        buf[i] += dec;
    }

    return hexlen / 2 + hexlen % 2;
}

STUB_TEXT int OsGdbBin2Hex(const U8 *buf, int buflen, char *hex, int hexlen)
{
    if ((hexlen + 1) < buflen * 2) {
        return 0;
    }

    for (int i = 0; i < buflen; i++) {
        if (OsGdbHex2Char(buf[i] >> 4, &hex[2 * i]) < 0) {
            return 0;
        }
        if (OsGdbHex2Char(buf[i] & 0xf, &hex[2 * i + 1]) < 0) {
            return 0;
        }
    }

    return 2 * buflen;
}

STUB_TEXT int OsGdbSendPacketNoAck(const U8 *data, int len)
{
    U8 buf[2];
    U8 checksum = 0;

    /* Send packet start */
    OsGdbPutchar('$');

    /* Send packet data and calculate checksum */
    while (len-- > 0) {
        checksum += *data;
        OsGdbPutchar(*data++);
    }

    /* Send the checksum */
    OsGdbPutchar('#');

    if (OsGdbBin2Hex(&checksum, 1, buf, sizeof(buf)) == 0) {
        return -1;
    }

    OsGdbPutchar(buf[0]);
    OsGdbPutchar(buf[1]);
    OsGdbFlush();
    return 0;
}
/**
 * Add preamble and termination to the given data.
 *
 * It returns 0 if the packet was acknowledge, -1 otherwise.
 */
STUB_TEXT int OsGdbSendPacket(const U8 *data, int len)
{
    int ret = OsGdbSendPacketNoAck(data, len);
    if (ret) {
        return ret;
    }
    if (OsGdbGetchar() == '+') {
        return 0;
    }

    /* Just got an invalid response */
    return -1;
}

/**
 * Receives one whole GDB packet.
 *
 * @retval  0 Success
 * @retval -1 Checksum error
 * @retval -2 Incoming packet too large
 */
STUB_TEXT int OsGdbGetPacket(U8 *buf, int buf_len, int *len)
{
    U8 ch = '0';
    U8 expect, actual = 0;
    U8 checksumBuf[2];

    /* Wait for packet start */
    actual = 0;

    /* wait for the start character, ignore the rest */
    while (ch != '$') {
        ch = OsGdbGetchar();
    }

    *len = 0;
    /* Read until receive '#' */
    while (1) {
        ch = OsGdbGetchar();

        if (ch == '#') {
            break;
        }

        /* Only put into buffer if not full */
        if (*len < (buf_len - 1)) {
            buf[*len] = ch;
        }

        actual += ch;
        (*len)++;
    }

    buf[*len] = '\0';

    /* Get checksum now */
    checksumBuf[0] = OsGdbGetchar();
    checksumBuf[1] = OsGdbGetchar();

    if (OsGdbHex2Bin(checksumBuf, 2, &expect, 1) == 0) {
        return -GDB_RSP_ENO_CHKSUM;
    }

    /* Verify checksum */
    if (actual != expect) {
        /* NACK packet */
        OsGdbPutchar('-');
        return -GDB_RSP_ENO_CHKSUM;
    }

    /* ACK packet */
    OsGdbPutchar('+');

    if (*len >= (buf_len - 1)) {
        return -GDB_RSP_ENO_2BIG;
    } else {
        return 0;
    }
}

/**
 * Send a error packet
 */
static STUB_TEXT int OsGdbSendErrPkt(U8 *buf, int len, U8 type, U8 error)
{
    int size;

    *buf = type;
    size = OsGdbBin2Hex(&error, 1, buf + 1, len - 1);
    if (size == 0) {
        return -1;
    }

    /* Related to 'E' */
    size++;

    return OsGdbSendPacket(buf, size);
}

/**
 * Send a exception packet "T <value>"
 */
STUB_TEXT int OsGdbSendException(U8 *buf, int len, U8 exception)
{
    return OsGdbSendErrPkt(buf, len, 'T', exception);
}

/**
 * Send a error packet
 */
STUB_TEXT int OsGdbSendError(U8 *buf, int len, U8 error)
{
    return OsGdbSendErrPkt(buf, len, 'E', error);
}
