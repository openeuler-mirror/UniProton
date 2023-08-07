#ifndef _PRT_LIBC_H_
#define _PRT_LIBC_H_

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/**
 * 模块内宏定义
 */
#define    DEFPREC         7
#define    DEFLPREC        16

/* 11-bit PRT_exponent (VAX G floating point) is 308 decimal digits */
#define    MAXEXP          308
/* 128 bit fraction takes up 39 decimal digits; max reasonable precision */
#define    MAXFRACT        39

#define    BUF             (MAXEXP + MAXFRACT + 1)    /* + decimal point */

#define ARG(basetype) \
    (ulonglongValue = flags & LONGLONGINT ? va_arg(argp, long long basetype) : \
        flags & LONGINT ? va_arg(argp, long basetype) : \
        flags & SHORTINT ? (short basetype)va_arg(argp, int) : \
        va_arg(argp, int))

#define todigit(c)         ((c) - '0')
#define tochar(n)          ((n) + '0')

#define LONGINT            0x01        /* long integer */
#define LONGDBL            0x02        /* long double; unimplemented */
#define SHORTINT           0x04        /* short integer */
#define ALT                0x08        /* alternate form */
#define LADJUST            0x10        /* left adjustment */
#define ZEROPAD            0x20        /* zero (as opposed to blank) pad */
#define HEXPREFIX          0x40        /* add 0x or 0X prefix */
#define LONGLONGINT        0x80        /* long long integer */

#ifndef isascii
#define isascii(c)         (((unsigned char)(c)) <= 0x7f)
#endif

#ifndef isdigit
#define isdigit(c)         ((c) >= '0' && (c) <= '9')
#endif

#define EXC_PREFIX_OFFSET   5
#define HWI_PREFIX_OFFSET   5
#define SYS_PREFIX_OFFSET   5
#define TSK_PREFIX_OFFSET   6
#define TSK_TAIL_OFFSET     7

#define AGENT_TASK_NAME     "ShellAgent"

#define SECUREC_VSPRINTF_PARAM_ERROR(format, strDest, destMax, maxLimit) \
    ((format) == NULL || (strDest) == NULL || (destMax) == 0 || (destMax) > (maxLimit))

#define SECUREC_VSPRINTF_CLEAR_DEST(strDest, destMax, maxLimit) do { \
    if ((strDest) != NULL && (destMax) > 0 && (destMax) <= (maxLimit)) { \
        *(strDest) = '\0'; \
    } \
} while (0)
#define SECUREC_FORMAT_FLAG_TABLE_SIZE 128

/* The return value of the internal function, which is returned when truncated */
#define SECUREC_PRINTF_TRUNCATE (-2)

/**
 * 模块内typedef声明
 */
typedef struct tag_IEEEdp
{
    unsigned int manl : 32;
    unsigned int manh : 20;
    unsigned int exp : 11;
    unsigned int sign : 1;
}IEEEdp;


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif