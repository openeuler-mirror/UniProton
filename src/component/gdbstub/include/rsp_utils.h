#ifndef _RSP_UTILS_H_
#define _RSP_UTILS_H_

#include "prt_typedef.h"

#define GDB_RSP_ENO_CHKSUM  1
#define GDB_RSP_ENO_2BIG    2
/**
 * Add preamble and termination to the given data.
 *
 * It returns 0 if the packet was acknowledge, -1 otherwise.
 */
extern int OsGdbSendPacket(const U8 *data, int len);

extern int OsGdbSendPacketNoAck(const U8 *data, int len);

/**
 * Receives one whole GDB packet.
 *
 * @retval  0 Success
 * @retval -1 Checksum error
 * @retval -2 Incoming packet too large
 */
extern int OsGdbGetPacket(U8 *buf, int buf_len, int *len);

/**
 * Send a exception packet "T <value>"
 */
extern int OsGdbSendException(U8 *buf, int len, U8 exception);

extern int OsGdbSendError(U8 *buf, int len, U8 error);

extern int OsGdbChar2Hex(char c, U8 *x);

extern int OsGdbHex2Char(U8 x, char *c);

extern int OsGdbHex2Bin(const char *hex, int hexlen, U8 *buf, int buflen);

extern int OsGdbBin2Hex(const U8 *buf, int buflen, char *hex, int hexlen);
#endif /* _RSP_UTILS_H_ */
