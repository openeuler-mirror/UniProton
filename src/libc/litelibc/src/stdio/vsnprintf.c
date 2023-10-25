#include <stdio.h>
#include <stdarg.h>
#include <math.h>
#include "securec.h"
#include "stdint.h"

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

/**
 * 模块内typedef声明
 */
typedef struct tag_IEEEdp
{
    unsigned int manl : 32;
    unsigned int manh : 20;
    unsigned int exp : 11;
    unsigned int sign : 1;
} IEEEdp;

char *PRT_exponent(char *ptr, int exp, unsigned char fmtch)
{
    register char *temp;
    char expBuf[MAXEXP];

    *ptr++ = (char)fmtch;

    if (exp < 0) {
        exp = -exp;
        *ptr++ = '-';
    } else {
        *ptr++ = '+';
    }

    temp = expBuf + MAXEXP;

    if (exp > 9) {
        do {
            *--temp = (char)(tochar(exp % 10));
        } while ((exp /= 10) > 9);

        *--temp = (char)(tochar(exp));

        for (; temp < expBuf + MAXEXP; *ptr++ = *temp++) {
        }
    } else {
        *ptr++ = '0';
        *ptr++ = (char)(tochar(exp));
    }
    return (ptr);
}

char *PRT_round(double fract, int *exp, char *start, char *end, char charPara, char *signp)
{
    double tmpValue;

    if (fract) {
        (void)modf(fract * 10, &tmpValue);
    } else {
        tmpValue = todigit(charPara);
    }

    if (tmpValue > 4) {
        for (;; --end) {
            if ('.' == *end) {
                --end;
            }

            if (++*end <= '9') {
                break;
            }

            *end = '0';
            if (end == start) {
                if (exp) {
                    /* e/E; increment PRT_exponent */
                    *end = '1';
                    ++*exp;
                } else {
                    /* f; add extra digit */
                    *--end = '1';
                    --start;
                }
                break;
            }
        }
    } else if ('-' == *signp) {
        /* ``"%.3f", (int)-0.0004'' gives you a negative 0. */
        for (;; --end) {
            if ('.' == *end) {
                --end;
            }

            if ('0' != *end) {
                break;
            }

            if (end == start) {
                *signp = 0;
            }
        }
    }
    return (start);
}

int PRT_isnan(double *d)
{
    register IEEEdp *temp = (IEEEdp *)(void *)d;

    return (0x7FF == temp->exp && (temp->manh || temp->manl));
}

int PRT_isinf(double *d)
{
    register IEEEdp *temp = (IEEEdp *)(void *)d;

    return (0x7FF == temp->exp && !temp->manh && !temp->manl);
}

int PRT_cvt(double number, register int precValue, int flags, unsigned char fmtch, char *signp, char *startP, char *endP)
{
    register char *ptr, *temp;
    register double fract;
    int doTrim, expCnt, gFormat;
    double integerValue, tmpValue;

    /* do this before tricky precision changes */
    if (PRT_isinf(&number)) {
        (void)strncpy_s(startP, strlen("Inf") + 1, "Inf", strlen("Inf"));
        return 3; /* strlen("Inf") */
    }

    if (PRT_isnan(&number)) {
        (void)strncpy_s(startP, strlen("NaN") + 1, "NaN", strlen("NaN"));
        return 3; /* strlen("NaN") */
    }

    expCnt = gFormat = 0;
    fract = modf(number, &integerValue);

    /* get an extra slot for rounding. */
    temp = ++startP;

    /*
     * get integer portion of number; put into the end of the buffer; the
     * .01 is added for modf(356.0 / 10, &integer) returning .59999999...
     */

    for (ptr = endP - 1; integerValue; ++expCnt) {
        tmpValue = modf(integerValue / 10, &integerValue);
        *ptr-- = (char)tochar((int)((tmpValue + .01) * 10));
    }

    switch (fmtch) {
        case 'f':
            /* reverse integer into beginning of buffer */
            if (expCnt) {
                for (; ++ptr < endP; *temp++ = *ptr) {
                }
            } else {
                *temp++ = '0';
            }

            /*
             * if precision required or alternate flag set, add in a
             * decimal point.
             */
            if (precValue || flags & ALT) {
                *temp++ = '.';
            }

            /* if requires more precision and some fraction left */
            if (fract) {
                if (precValue) {
                    do {
                        fract = modf(fract * 10, &tmpValue);
                        *temp++ = (char)tochar((int)tmpValue);
                    } while (--precValue && fract);
                }
                if (fract) {
                    startP = PRT_round(fract, (int *)NULL, startP,
                        temp - 1, (char)0, signp);
                }
            }

            for (; precValue--; *temp++ = '0') {
            }
            break;

        case 'e':
        case 'E':
eformat:
            if (expCnt) {
                *temp++ = *++ptr;

                if (precValue || (flags & ALT)) {
                    *temp++ = '.';
                }

                /* if requires more precision and some integer left */
                for (; precValue && ++ptr < endP; --precValue) {
                    *temp++ = *ptr;
                }

                /*
                 * if done precision and more of the integer component,
                 * round using it; adjust fract so we don't re-round
                 * later.
                 */
                if (!precValue && ++ptr < endP) {
                    fract = 0;
                    startP = PRT_round((double)0, &expCnt, startP,
                        temp - 1, *ptr, signp);
                }
                /* adjust expCnt for digit in front of decimal */

                --expCnt;
            } else if (fract) {
                /* until first fractional digit, decrement PRT_exponent */
                /* adjust expCnt for digit in front of decimal */
                for (expCnt = -1;; --expCnt) {
                    fract = modf(fract * 10, &tmpValue);

                    if (tmpValue) {
                        break;
                    }
                }

                *temp++ = (char)tochar((int)tmpValue);

                if (precValue || (flags & ALT)) {
                    *temp++ = '.';
                }
            } else {
                *temp++ = '0';

                if (precValue || flags & ALT) {
                    *temp++ = '.';
                }
            }

            /* if requires more precision and some fraction left */
            if (fract) {
                if (precValue) {
                    do {
                        fract = modf(fract * 10, &tmpValue);
                        *temp++ = (char)tochar((int)tmpValue);
                    } while (--precValue && fract);
                }

                if (fract) {
                    startP = PRT_round(fract, &expCnt, startP,
                        temp - 1, (char)0, signp);
                }
            }
            /* if requires more precision */
            for (; precValue--; *temp++ = '0') {
            }

            /* unless alternate flag, trim any g/G format trailing 0's */
            if (gFormat && !(flags & ALT)) {
                while (temp > startP && '0' == *--temp) {
                }

                if ('.' == *temp) {
                    --temp;
                }

                ++temp;
            }

            temp = PRT_exponent(temp, expCnt, fmtch);
            break;

        case 'g':
        case 'G':
            /* a precision of 0 is treated as a precision of 1. */
            if (!precValue) {
                ++precValue;
            }

            /*
             * ``The style used depends on the value converted; style e
             * will be used only if the PRT_exponent resulting from the
             * conversion is less than -4 or greater than the precision.''
             *    -- ANSI X3J11
             */
            if ((expCnt > precValue) || (!expCnt && fract && (fract < .0001))) {
                /*
                 * g/G format counts "significant digits, not digits of
                 * precision; for the e/E format, this just causes an
                 * off-by-one problem, i.e. g/G considers the digit
                 * before the decimal point significant and e/E doesn't
                 * count it as precision.
                 */
                --precValue;
                fmtch -= 2;        /* G->E, g->e */
                gFormat = 1;
                goto eformat;
            }
            /*
             * reverse integer into beginning of buffer,
             * note, decrement precision
             */
            if (expCnt) {
                for (; ++ptr < endP; *temp++ = *ptr, --precValue) {
                }
            } else {
                *temp++ = '0';
            }

            /*
             * if precision required or alternate flag set, add in a
             * decimal point.  If no digits yet, add in leading 0.
             */
            if (precValue || (flags & ALT)) {
                doTrim = 1;
                *temp++ = '.';
            } else {
                doTrim = 0;
            }

            /* if requires more precision and some fraction left */
            if (fract) {
                if (precValue) {
                    do {
                        fract = modf(fract * 10, &tmpValue);
                        *temp++ = (char)tochar((int)tmpValue);
                    } while (!tmpValue);

                    while (--precValue && fract) {
                        fract = modf(fract * 10, &tmpValue);
                        *temp++ = (char)tochar((int)tmpValue);
                    }
                }

                if (fract) {
                    startP = PRT_round(fract, (int *)NULL, startP,
                        temp - 1, (char)0, signp);
                }
            }

            /* alternate format, adds 0's for precision, else trim 0's */
            if (flags & ALT) {
                for (; precValue--; *temp++ = '0') {
                }
            } else if (doTrim) {
                while (temp > startP && '0' == *--temp) {
                }

                if ('.' != *temp) {
                    ++temp;
                }
            }
            break;

        default:
            break;
    }

    return (temp - startP);
}

int osVsnprintf(char *outBuf, unsigned int strLen, const char *fmt0, va_list argp, unsigned char ucFlag)
{
    unsigned char *strfmt;   /* format string */
    int charTmp;              /* character from fmt */
    int count;             /* random handy integer */
    char *buffPtr;        /* buffer pointer */
    unsigned long long ulonglongValue;    /* integer arguments %[diouxX] */
    int base;              /* base for [diouxX] conversion */
    int decPrec;           /* decimal precision in [diouxX] */
    int fieldSize;         /* field size expanded by sign, etc */
    int flags;             /* flags as above */
    int fPPrec;            /* `extra' floating precision in [eEfgG] */
    int precValue;              /* precision from format (%.3d), or -1 */
    int realSize;          /* field size expanded by decimal precision */
    int size;              /* size of converted field or string */
    int width;             /* width from format (%8d), or 0 */
    char sign;             /* sign prefix (' ', '+', '-', or \0) */
    char *digits;         /* digits for [diouxX] conversion */
    char buf[BUF];         /* space for %c, %[diouxX], %[eEfgG] */
    char *outBufPtr = outBuf;
    char softSign;         /* temporary negative sign for floats */
    unsigned int writeLen = strLen;

    strfmt = (unsigned char *)fmt0;
    digits = "0123456789abcdef";

    for (;; ++strfmt) {
        for (; ((charTmp = *strfmt) != 0) && charTmp != '%'; ++strfmt) {
            if (!ucFlag || writeLen--) {
                *outBufPtr++ = (char)charTmp;
            } else {
                goto END;
            }
        }
        if (!charTmp) {
            goto lvspret;
        }

        flags = 0;
        decPrec = 0;
        fPPrec = 0;
        width = 0;
        precValue = -1;
        sign = '\0';

rflag:
        switch (*++strfmt) {
            case ' ':
                /*
                 * ``If the space and + flags both appear, the space
                 * flag will be ignored.''
                 *    -- ANSI X3J11
                 */
                if (!sign) {
                    sign = ' ';
                }
                goto rflag;
            case '#':
                flags |= ALT;
                goto rflag;
            case '*':
            {
                /*
                 * ``A negative field width argument is taken as a
                 * - flag followed by a positive field width.''
                 *    -- ANSI X3J11
                 * They don't exclude field widths read from args.
                 */
                int siTmpLocal = 0;
                if ((width = va_arg(argp, int)) >= siTmpLocal) {
                    goto rflag;
                }
                width = -width;
                flags |= LADJUST;
                goto rflag;
            }
            case '-':
                flags |= LADJUST;
                goto rflag;
            case '+':
                sign = '+';
                goto rflag;
            case '.':
                if (*++strfmt == '*') {
                    count = va_arg(argp, int);
                } else {
                    count = 0;
                    while (isascii(*strfmt) && isdigit(*strfmt)) {
                        count = 10 * count + todigit(*strfmt++);
                    }
                    --strfmt;
                }
                precValue = count < 0 ? -1 : count;
                goto rflag;
            case '0':
                /*
                 * ``Note that 0 is taken as a flag, not as the
                 * beginning of a field width.''
                 *    -- ANSI X3J11
                 */
                flags |= ZEROPAD;
                goto rflag;
            case '1': case '2': case '3': case '4':
            case '5': case '6': case '7': case '8': case '9':
                count = 0;
                do {
                    count = 10 * count + todigit(*strfmt);
                } while (isascii(*++strfmt) && isdigit(*strfmt));

                width = count;
                --strfmt;
                goto rflag;
            case 'L':
                flags |= LONGDBL;
                goto rflag;
            case 'h':
                flags |= SHORTINT;
                goto rflag;
            case 'l':
                if (flags & LONGINT) {
                    flags |= LONGLONGINT;
                } else {
                    flags |= LONGINT;
                }
                goto rflag;
            case 'c':
                *(buffPtr = buf) = (char)va_arg(argp, int);
                size = 1;
                sign = '\0';
                goto pforw;
            case 'D':
                flags |= LONGINT;
                /* FALLTHROUGH */
            case 'd':
            case 'i':
                ARG(int);
                if ((long long)ulonglongValue < 0) {
                    long long sllTmpint = (long long)ulonglongValue;
                    sllTmpint = -sllTmpint;
                    ulonglongValue = sllTmpint;
                    sign = '-';
                }
                base = 10;
                goto number;
            case 'e':
            case 'E':
            case 'f':
            case 'g':
            case 'G':
            {
                union {
                    double _double; /* int precision arguments %[eEfgG] */
                    uint64_t _uint64; /* avoid strict aliasing warning */
                } doubleValue;
                doubleValue._double = va_arg(argp, double);
                /*
                 * don't do unrealistic precision; just pad it with
                 * zeroes later, so buffer size stays rational.
                 */
                if (precValue > MAXFRACT) {
                    if (('g' != *strfmt && 'G' != *strfmt) || (flags & ALT)) {
                        fPPrec = precValue - MAXFRACT;
                    }
                    precValue = MAXFRACT;
                } else if (-1 == precValue) {
                    if (flags & LONGINT) {
                        precValue = DEFPREC;
                    } else {
                        precValue = DEFPREC;
                    }
                }
                /*
                 * softsign avoids negative 0 if _double is < 0 and
                 * no significant digits will be shown
                 */
                if (doubleValue._double < 0 ||
                    ((doubleValue._double == 0) &&
                     (doubleValue._uint64 & ((uint64_t)1 << 63)) != 0)) {
                    softSign = '-';
                    doubleValue._double = -doubleValue._double;
                } else {
                    softSign = 0;
                }
                /*
                 * PRT_cvt may have to round up past the "start" of the
                 * buffer, i.e. ``intf("%.2f", (int)9.999);'';
                 * if the first char isn't NULL, it did.
                 */
                *buf = 0;

                size = PRT_cvt(doubleValue._double, precValue, flags, *strfmt, &softSign, buf, buf + sizeof(buf));
                if (softSign) {
                    sign = '-';
                }
                buffPtr = *buf ? buf : buf + 1;
                goto pforw;
            }
            case 'O':
                flags |= LONGINT;
                /* FALLTHROUGH */
            case 'o':
                ARG(unsigned);
                base = 8;
                goto nosign;
            case 'p':
                /*
                 * ``The argument shall be a pointer to void. The
                 * value of the pointer is converted to a sequence
                 * of printable characters, in an implementation-
                 * defined manner.''
                 *    -- ANSI X3J11
                 */
                /* NOSTRICT */
                ulonglongValue = (unsigned long)va_arg(argp, void *);
                base = 16;
                goto nosign;
            case 's':
            {
                char *pscTmpChar = NULL;
                if ((buffPtr = va_arg(argp, char *)) == pscTmpChar) {
                    buffPtr = "(null)";
                }
                if (precValue >= 0) {
                    /*
                     * can't use strlen; can only look for the
                     * NUL in the first `prec' characters, and
                     * strlen() will go further.
                     */
                    char *ptr;

                    if ((ptr = (char *)memchr((const char *)buffPtr, 0, precValue)) != 0) {
                        size = ptr - buffPtr;
                        if (size > precValue) {
                            size = precValue;
                        }
                    } else {
                        size = precValue;
                    }
                } else {
                    size = strlen(buffPtr);
                }
                sign = '\0';
                goto pforw;
            }
            case 'U':
                flags |= LONGINT;
                /* FALLTHROUGH */
            case 'u':
                ARG(unsigned);
                base = 10;
                goto nosign;
            case 'X':
                digits = "0123456789ABCDEF";
                /* FALLTHROUGH */
            case 'x':
                ARG(unsigned);
                base = 16;
                /* leading 0x/X only if non-zero */
                if (flags & ALT && 0 != ulonglongValue) {
                    flags |= HEXPREFIX;
                }

                /* unsigned conversions */
nosign:         sign = '\0';
                /*
                 * ``... diouXx conversions ... if a precision is
                 * specified, the 0 flag will be ignored.''
                 *    -- ANSI X3J11
                 */
number:         if ((decPrec = precValue) >= 0) {
                    flags &= ~ZEROPAD;
                }

                /*
                 * ``The result of converting a zero value with an
                 * explicit precision of zero is no characters.''
                 *    -- ANSI X3J11
                 */
                buffPtr = buf + BUF;
                if (0 != ulonglongValue || 0 != precValue) {
                    do {
                        *--buffPtr = digits[ulonglongValue % base];
                        ulonglongValue /= base;
                    } while (ulonglongValue);

                    digits = "0123456789abcdef";
                    if (flags & ALT && 8 == base && '0' != *buffPtr) {
                        *--buffPtr = '0'; /* octal leading 0 */
                    }
                }
                size = buf + BUF - buffPtr;

pforw:
                /*
                 * All reasonable formats wind up here. At this point,
                 * `t' points to a string which (if not flags & LADJUST)
                 * should be padded out to `width' places. If
                 * flags & ZEROPAD, it should first be prefixed by any
                 * sign or other prefix; otherwise, it should be blank
                 * padded before the prefix is emitted. After any
                 * left-hand padding and prefixing, emit zeroes
                 * required by a decimal [diouxX] precision, then print
                 * the string proper, then emit zeroes required by any
                 * leftover floating precision; finally, if LADJUST,
                 * pad with blanks.
                 */

                /*
                 * compute actual size, so we know how much to pad
                 * fieldsz excludes decimal prec; realsz includes it
                 */
                fieldSize = size + fPPrec;
                if (sign) {
                    fieldSize++;
                }

                if (flags & HEXPREFIX) {
                    fieldSize += 2;
                }

                realSize = decPrec > fieldSize ? decPrec : fieldSize;

                /* right-adjusting blank padding */
                if (0 == (flags & (LADJUST | ZEROPAD)) && width) {
                    for (count = realSize; count < width; count++) {
                        if (!ucFlag || writeLen--) {
                            *outBufPtr++ = ' ';
                        } else {
                            goto END;
                        }
                    }
                }
                /* prefix */
                if (sign) {
                    if (!ucFlag || writeLen--) {
                        *outBufPtr++ = sign;
                    } else {
                        goto END;
                    }
                }

                if (flags & HEXPREFIX) {
                    if (!ucFlag || writeLen--) {
                        *outBufPtr++ = '0';
                    } else {
                        goto END;
                    }

                    if (!ucFlag || writeLen--) {
                        *outBufPtr++ = (char)*strfmt;
                    } else {
                        goto END;
                    }
                }
                /* right-adjusting zero padding */
                if ((flags & (LADJUST | ZEROPAD)) == ZEROPAD) {
                    for (count = realSize; count < width; count++) {
                        if (!ucFlag || writeLen--) {
                            *outBufPtr++ = '0';
                        } else {
                            goto END;
                        }
                    }
                }
                /* leading zeroes from decimal precision */
                for (count = fieldSize; count < decPrec; count++) {
                    if (!ucFlag || writeLen--) {
                        *outBufPtr++ = '0';
                    } else {
                        goto END;
                    }
                }

                /* the string or number proper */
                count = size;
                while (--count >= 0) {
                    if (!ucFlag || writeLen--) {
                        *outBufPtr++ = *buffPtr++;
                    } else {
                        goto END;
                    }
                }
                /* trailing f.p. zeroes */
                while (--fPPrec >= 0) {
                    if (!ucFlag || writeLen--) {
                        *outBufPtr++ = '0';
                    } else {
                        goto END;
                    }
                }
                /* left-adjusting padding (always blank) */
                if (flags & LADJUST) {
                    for (count = realSize; count < width; count++) {
                        if (!ucFlag || writeLen--) {
                            *outBufPtr++ = ' ';
                        } else {
                            goto END;
                        }
                    }
                }
                /* finally, adjust cnt */

                break;
            case '\0':    /* "%?" prints ?, unless ? is NULL */
                goto lvspret;
            default:
                if (!ucFlag || writeLen--) {
                    *outBufPtr++ = (char)*strfmt;
                } else {
                    goto END;
                }
        }
    }

lvspret:
    if (!ucFlag || writeLen--) {
        *outBufPtr = 0;
    } else {
        goto END;
    }

    (void)argp;
    /* end of addition */
    return (int)(outBufPtr - outBuf);
    /* NOTREACHED */

END:
    outBufPtr[-1] = 0;
    return (int)-1;
}

int vsnprintf(char *str, size_t size, const char *format, va_list ap)
{
    int num;

    if (!str || !format || !size) {
        return -1;
    }

    num = osVsnprintf(str, size, format, ap, 1);
    if (num >= 0) {
        return num;
    } else {
        return -1;
    }
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
