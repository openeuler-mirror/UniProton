#include <stdio.h>
#include <stdarg.h>
#include <math.h>
#include "securec.h"
#include "libc.h"
#include "stdint.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

const char *SecSkipKnownFlags(const char *format)
{
    static const unsigned char flagTable[SECUREC_FORMAT_FLAG_TABLE_SIZE] = {
        /*
         * Known flag is "0123456789 +-#hlLwZzjqt*I$"
         */
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x01, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0x01, 0x00, 0x00,
        0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00,
        0x00, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    const char *fmt = format;
    while (*fmt != '\0') {
        char fmtChar = *fmt;
        if ((unsigned char)fmtChar > 0x7f) { /* 0x7f is upper limit of format char value */
            break;
        }
        if (flagTable[(unsigned char)fmtChar] == 0) {
            break;
        }
        ++fmt;
    }
    return fmt;
}

int SecFormatContainN(const char *format)
{
    const char *fmt = format;
    while (*fmt != '\0') {
        ++fmt;
        /* Skip normal char */
        if (*(fmt - 1) != '%') {
            continue;
        }
        /* Meet %% */
        if (*fmt == '%') {
            ++fmt; /* Point to the character after the %. Correct handling %%xx */
            continue;
        }
        /* Now parse %..., fmt point to the character after the % */
        fmt = SecSkipKnownFlags(fmt);
        if (*fmt == 'n') {
            return 1;
        }
    }
    return 0;
}

int SecVsnprintfImpl(char *string, size_t count, const char *format, va_list argList)
{
    int retVal;
    if (SecFormatContainN(format) != 0) {
        string[0] = '\0';
        return -1;
    }

    retVal = vsnprintf(string, count, format, argList);
    if (retVal >= (int)count) { /* The size_t to int is ok, count max is SECUREC_STRING_MAX_LEN */
        /* The buffer was too small; we return truncation */
        string[count - 1] = '\0';
        return SECUREC_PRINTF_TRUNCATE;
    }
    if (retVal < 0) {
        string[0] = '\0'; /* Empty the dest strDest */
        return -1;
    }
    return retVal;
}

int vsprintf_ck(char *strDest, size_t destMax, const char *format, va_list argList)
{
    int retVal;

    if (SECUREC_VSPRINTF_PARAM_ERROR(format, strDest, destMax, SECUREC_STRING_MAX_LEN)) {
        SECUREC_VSPRINTF_CLEAR_DEST(strDest, destMax, SECUREC_STRING_MAX_LEN);
        return -1;
    }

    retVal = SecVsnprintfImpl(strDest, destMax, format, argList);
    if (retVal < 0) {
        strDest[0] = '\0';
        if (retVal == SECUREC_PRINTF_TRUNCATE) {
            /* Buffer is too small */
        }
        return -1;
    }

    return retVal;
}

int sprintf_ck(char *strDest, size_t destMax, const char *format, ...)
{
    int ret;
    va_list argList;

    va_start(argList, format);
    ret = vsprintf_ck(strDest, destMax, format, argList);
    va_end(argList);
    (void)argList;

    return ret;
}


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif