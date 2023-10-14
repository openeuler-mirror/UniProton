#include <string.h>
#include <stdarg.h>

typedef struct __FILE_TMP {
    unsigned stub;
} FILE;

int fputs(const char *restrict s, FILE *restrict f)
{
    return 0;
}

int fflush(FILE *f)
{
    return 0;
}

char *fgets(char *restrict s, int n, FILE *restrict f)
{
    return s;
}

int fprintf(FILE *restrict f, const char *restrict fmt, ...)
{
    return 0;
}

size_t fwrite(const void *restrict src, size_t size, size_t nmemb, FILE *restrict f)
{
    return 0;
}

int feof(FILE *f)
{
    return 0;
}

size_t fread(void *restrict destv, size_t size, size_t nmemb, FILE *restrict f)
{
    return 0;
}

int getc(FILE *f)
{
    return 0;
}

FILE *fopen(const char *restrict filename, const char *restrict mode)
{
    return 0;
}

int ferror(FILE *f)
{
    return 0;
}

int fclose(FILE *f)
{
    return 0;
}

FILE *freopen(const char *restrict filename, const char *restrict mode, FILE *restrict f)
{
    return f;
}

void flockfile(FILE *f)
{

}

int (getc_unlocked)(FILE *f)
{
    return 0;
}

void funlockfile(FILE *f)
{

}

int pclose(FILE *f)
{
    return 0;
}

FILE *tmpfile(void)
{
    return 0;
}

FILE *popen(const char *cmd, const char *mode)
{
    return 0;
}

void clearerr(FILE *f)
{

}

int ungetc(int c, FILE *f)
{
    return 0;
}

int setvbuf(FILE *restrict f, char *restrict buf, int type, size_t size)
{
    return 0;
}

int fseek(FILE *f, long off, int whence)
{
    return 0;
}

int fseeko(FILE * f, long a, int b)
{
    return 0;
}

long ftell(FILE *f)
{
    return 0;
}

long ftello(FILE * f)
{
    return 0;
}

char *setlocale(int cat, const char *name)
{
    return 0;
}

int rename(const char *old, const char *new)
{
    return 0;
}

int remove(const char *path)
{
    return 0;
}

int mkstemp(char *template)
{
    return 0;
}

int close(int fd)
{
    return 0;
}

int isatty(int fd)
{
    return 0;
}

int system(const char *cmd)
{
    return 0;
}

void (*signal(int sig, void (*func)(int)))(int)
{
    return func;
}