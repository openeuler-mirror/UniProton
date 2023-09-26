#ifndef RTT_VIEWER_H
#define RTT_VIEWER_H

#ifdef __cplusplus
extern "C"
{
#endif

#define RTT_VIEWER_MODE_NO_BLOCK_SKIP                     0
#define RTT_VIEWER_MODE_NO_BLOCK_TRIM                     1
#define RTT_VIEWER_MODE_BLOCK_IF_FIFO_FULL                2
#define RTT_VIEWER_MODE_MASK                              3

#ifndef RTT_VIEWER_MODE_DEFAULT
#define RTT_VIEWER_MODE_DEFAULT                           RTT_VIEWER_MODE_NO_BLOCK_SKIP
#endif

#ifndef RTT_VIEWER_WB_MAX_NUM
#define RTT_VIEWER_WB_MAX_NUM                             1
#endif

#ifndef RTT_VIEWER_RB_MAX_NUM
#define RTT_VIEWER_RB_MAX_NUM                             1
#endif

#ifndef RTT_VIEWER_WB_BUFFER_SIZE
#define RTT_VIEWER_WB_BUFFER_SIZE                         1024
#endif

#ifndef RTT_VIEWER_RB_BUFFER_SIZE
#define RTT_VIEWER_RB_BUFFER_SIZE                         16
#endif

#ifndef RTT_VIEWER_MAX_INT_PRI
#define RTT_VIEWER_MAX_INT_PRI                            0x20
#endif

#ifndef RTT_VIEWER_MAX_SHOW_LEN
#define RTT_VIEWER_MAX_SHOW_LEN                           0x200
#endif

#define RTT_DMB() __asm volatile ("dmb\n" : : :);

#define RTT_VIEWER_CTRL_RESET                "\x1B[0m"
#define RTT_VIEWER_CTRL_CLEAR                "\x1B[2J"

#define RTT_VIEWER_CTRL_TEXT_BLACK           "\x1B[2;30m"
#define RTT_VIEWER_CTRL_TEXT_RED             "\x1B[2;31m"
#define RTT_VIEWER_CTRL_TEXT_GREEN           "\x1B[2;32m"
#define RTT_VIEWER_CTRL_TEXT_YELLOW          "\x1B[2;33m"
#define RTT_VIEWER_CTRL_TEXT_BLUE            "\x1B[2;34m"
#define RTT_VIEWER_CTRL_TEXT_MAGENTA         "\x1B[2;35m"
#define RTT_VIEWER_CTRL_TEXT_CYAN            "\x1B[2;36m"
#define RTT_VIEWER_CTRL_TEXT_WHITE           "\x1B[2;37m"

#define RTT_VIEWER_CTRL_TEXT_BRIGHT_BLACK    "\x1B[1;30m"
#define RTT_VIEWER_CTRL_TEXT_BRIGHT_RED      "\x1B[1;31m"
#define RTT_VIEWER_CTRL_TEXT_BRIGHT_GREEN    "\x1B[1;32m"
#define RTT_VIEWER_CTRL_TEXT_BRIGHT_YELLOW   "\x1B[1;33m"
#define RTT_VIEWER_CTRL_TEXT_BRIGHT_BLUE     "\x1B[1;34m"
#define RTT_VIEWER_CTRL_TEXT_BRIGHT_MAGENTA  "\x1B[1;35m"
#define RTT_VIEWER_CTRL_TEXT_BRIGHT_CYAN     "\x1B[1;36m"
#define RTT_VIEWER_CTRL_TEXT_BRIGHT_WHITE    "\x1B[1;37m"

#define RTT_VIEWER_CTRL_BG_BLACK             "\x1B[24;40m"
#define RTT_VIEWER_CTRL_BG_RED               "\x1B[24;41m"
#define RTT_VIEWER_CTRL_BG_GREEN             "\x1B[24;42m"
#define RTT_VIEWER_CTRL_BG_YELLOW            "\x1B[24;43m"
#define RTT_VIEWER_CTRL_BG_BLUE              "\x1B[24;44m"
#define RTT_VIEWER_CTRL_BG_MAGENTA           "\x1B[24;45m"
#define RTT_VIEWER_CTRL_BG_CYAN              "\x1B[24;46m"
#define RTT_VIEWER_CTRL_BG_WHITE             "\x1B[24;47m"

#define RTT_VIEWER_CTRL_BG_BRIGHT_BLACK      "\x1B[4;40m"
#define RTT_VIEWER_CTRL_BG_BRIGHT_RED        "\x1B[4;41m"
#define RTT_VIEWER_CTRL_BG_BRIGHT_GREEN      "\x1B[4;42m"
#define RTT_VIEWER_CTRL_BG_BRIGHT_YELLOW     "\x1B[4;43m"
#define RTT_VIEWER_CTRL_BG_BRIGHT_BLUE       "\x1B[4;44m"
#define RTT_VIEWER_CTRL_BG_BRIGHT_MAGENTA    "\x1B[4;45m"
#define RTT_VIEWER_CTRL_BG_BRIGHT_CYAN       "\x1B[4;46m"
#define RTT_VIEWER_CTRL_BG_BRIGHT_WHITE      "\x1B[4;47m"

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

/*
 * rtt viewer环形缓冲区
 */
typedef struct {
    const char *sName;
    char *pBuffer;
    unsigned int size;
    unsigned int wIdx;
    volatile unsigned int rIdx;
    unsigned int flags;
} RTT_VIEWER_RING_BUFFER;

typedef struct {
    char acID[16];
    int wbMaxNum;
    int rbMaxNum;
    RTT_VIEWER_RING_BUFFER aUp[RTT_VIEWER_WB_MAX_NUM];
    RTT_VIEWER_RING_BUFFER aDown[RTT_VIEWER_RB_MAX_NUM];
} TagRttViewerCb;

extern TagRttViewerCb _SEGGER_RTT;
#define g_rttViewerCb _SEGGER_RTT

void RttViewerInit(void);
unsigned int RttViewerModeSet(unsigned int chn, unsigned int mode);
unsigned int RttViewerWrite(unsigned int chn, const void *buffer, unsigned int numBytes);
unsigned int RttViewerRead(unsigned int chn, void *buffer, unsigned int numBytes);
unsigned int RttViewerPutChar(unsigned int chn, char c);
unsigned int RttViewerPrintf(unsigned int chn, const char *format, ...);

#ifdef __cplusplus
}
#endif

#endif /* RTT_VIEWER_H */
