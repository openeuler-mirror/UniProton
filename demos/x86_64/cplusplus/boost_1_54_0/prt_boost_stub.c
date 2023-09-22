#include <wchar.h>
#include <time.h>
#include <locale.h>

size_t mbstowcs(wchar_t *restrict ws, const char *restrict s, size_t wn)
{
    return 0;
}

const char *__strftime_fmt_1(char (*a)[100], size_t *b, int c, const struct tm *d, locale_t e, int f)
{
    return NULL;
}

wchar_t *wmemcpy(wchar_t *restrict d, const wchar_t *restrict s, size_t n)
{
    wchar_t *a = d;
    while (n--) {
        *d++ = *s++;
    }
    return a;
}

char *getenv(const char *name)
{
    return NULL;
}

locale_t __uselocale(locale_t a)
{
    return 0;
}

size_t mbsrtowcs(wchar_t *restrict ws, const char **restrict src, size_t wn, mbstate_t *restrict st)
{
    return 0;
}

size_t wcslen(const wchar_t *a)
{
    return 0;
}

int wcscmp(const wchar_t *a, const wchar_t *b)
{
    return 0;
}

typedef void* iconv_t;
iconv_t iconv_open(const char *a, const char *b)
{
    return NULL;
}

size_t iconv(iconv_t a, char **__restrict b, size_t *__restrict c, char **__restrict d, size_t *__restrict e)
{
    return 0;
}

int iconv_close(iconv_t a)
{
    return 0;
}

wchar_t *wmemchr(const wchar_t *s, wchar_t c, size_t n)
{
    return NULL;
}

wchar_t *wmemset(wchar_t *a, wchar_t b, size_t c)
{
    return NULL;
}

float __strtof_l(const char *restrict s, char **restrict p, locale_t l)
{
    return 0;
}

double __strtod_l(const char *restrict s, char **restrict p, locale_t l)
{
    return 0;
}

long double __strtold_l(const char *restrict s, char **restrict p, locale_t l)
{
    return 0;
}

wchar_t *wmemmove(wchar_t *d, const wchar_t *s, size_t n)
{
    return NULL;
}

wctype_t __wctype_l(const char *s, locale_t l)
{
    return 0;
}

typedef unsigned wint_t;
wint_t __towupper_l(wint_t c, locale_t l)
{
    return 0;
}

wint_t __towlower_l(wint_t c, locale_t l)
{
    return 0;
}

int __iswctype_l(wint_t c, wctype_t t, locale_t l)
{
    return 0;
}

int wctob(wint_t c)
{
    return 0;
}

locale_t __newlocale(int mask, const char *name, locale_t loc)
{
    return 0;
}

void __freelocale(locale_t l)
{
    return;
}

locale_t __duplocale(locale_t old)
{
    return 0;
}

wint_t btowc(int c)
{
    return 0;
}

long double strtold_l(const char *restrict s, char **restrict p, locale_t l)
{
    return 0;
}

char *strdup(const char *s)
{
    return NULL;
}

size_t wcsnrtombs(char *restrict dst, const wchar_t **restrict wcs, size_t wn, size_t n, mbstate_t *restrict st)
{
    return 0;
}

size_t mbsnrtowcs(wchar_t *restrict wcs, const char **restrict src, size_t n, size_t wn, mbstate_t *restrict st)
{
    return 0;
}

size_t __ctype_get_mb_cur_max(void)
{
    return 0;
}

size_t wcrtomb(char *restrict s, wchar_t wc, mbstate_t *restrict st)
{
    return 0;
}

size_t mbrtowc(wchar_t *restrict wc, const char *restrict src, size_t n, mbstate_t *restrict st)
{
    return 0;
}

int __strcoll_l(const char *l, const char *r, locale_t loc)
{
    return 0;
}

size_t __strxfrm_l(char *restrict dest, const char *restrict src, size_t n, locale_t loc)
{
    return 0;
}

int __wcscoll_l(const wchar_t *l, const wchar_t *r, locale_t locale)
{
    return 0;
}

size_t __wcsxfrm_l(wchar_t *restrict dest, const wchar_t *restrict src, size_t n, locale_t loc)
{
    return 0;
}

char *bind_textdomain_codeset(const char *domainname, const char *codeset)
{
    return NULL;
}

char *dgettext(const char *domainname, const char *msgid)
{
    return NULL;
}

size_t __strftime_l(char *restrict a, size_t b, const char *restrict c, const struct tm *restrict d, locale_t e)
{
    return 0;
}

_Noreturn void __assert_fail(const char *expr, const char *file, int line, const char *func)
{
    abort();
}