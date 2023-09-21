#define __GLIBC__       2
#define __GLIBC_MINOR__ 36
#define __GLIBC_PREREQ(maj, min) \
        ((__GLIBC__ << 16) + __GLIBC_MINOR__ >= ((maj) << 16) + (min))
        
typedef struct __locale_struct *__locale_t;

#ifndef _ISbit
# if __BYTE_ORDER == __BIG_ENDIAN
#  define _ISbit(bit)   (1 << (bit))
# else /* __BYTE_ORDER == __LITTLE_ENDIAN */
#  define _ISbit(bit)   ((bit) < 8 ? ((1 << (bit)) << 8) : ((1 << (bit)) >> 8))
# endif

enum
{
  _ISupper = _ISbit (0),        /* UPPERCASE.  */
  _ISlower = _ISbit (1),        /* lowercase.  */
  _ISalpha = _ISbit (2),        /* Alphabetic.  */
  _ISdigit = _ISbit (3),        /* Numeric.  */
  _ISxdigit = _ISbit (4),       /* Hexadecimal numeric.  */
  _ISspace = _ISbit (5),        /* Whitespace.  */
  _ISprint = _ISbit (6),        /* Printing.  */
  _ISgraph = _ISbit (7),        /* Graphical.  */
  _ISblank = _ISbit (8),        /* Blank (usually SPC and TAB).  */
  _IScntrl = _ISbit (9),        /* Control character.  */
  _ISpunct = _ISbit (10),       /* Punctuation.  */
  _ISalnum = _ISbit (11)        /* Alphanumeric.  */
};
#endif