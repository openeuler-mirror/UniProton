#include <string.h>
#include "stdlib.h"
#include "test.h"

#define length(x) (sizeof(x) / sizeof *(x))

static struct {
    char *s;
    double f;
    int ndigit;
    int decpt;
    int sign;
} t[] = {
    {"00", 0.0, 1, 1, 0},
    {"000", 0.0, 2, 1, 0},
    {"000000", -0.0, 5, 1, 1},
    {"596186034813181", 0.59618603481318067, 17, 0, 0}, 
    {"596186034813181", 0.59618603481318067, 16, 0, 0},
    {"596186034813181", 0.59618603481318067, 15, 0, 0}, 
    {"59618603481318", 0.59618603481318067, 14, 0, 0}, 
    {"6", 0.59618603481318067, 1, 0, 0}, 
    {"118150131692180", 1.1815013169218038, 14, 1, 0}, 
    {"424207082357534", 42.420708235753445, 13, 2, 0}, 
    {"665665468630652", 665.66546863065162, 12, 3, 0}, 
    {"610185292297087", 6101.8529229708685, 11, 4, 0}, 
    {"769669520823697", 76966.952082369677, 10, 5, 0}, 
    {"250506532222868", 250506.53222286823, 9, 6, 0}, 
    {"274003723022800", 2740037.2302280052, 8, 7, 0},
       {"274003723022800", -2740037.2302280052, 8, 7, 1},
    {"207230935004974", 20723093.500497428, 7, 8, 0},
       {"207230935004974", -20723093.500497428, 7, 8, 1},
};

int fcvt_test(void)
{
    int i;
    int dp, sign;
    char *p;
    
    for (i = 0; i < length(t); i++) 
    {
        p = fcvt(t[i].f, t[i].ndigit, &dp, &sign);
        if (0 != strcmp((const char*)t[i].s, (const char*)p))
        {
            printf("Array Number: t[%d]\n", i);
            printf("fcvt(\"%lf\") want %s got %s\n", t[i].f, t[i].s, p);
            t_status = 1;
        }
        if (dp != t[i].decpt) {
            printf("Array Number: t[%d]\n", i);
            printf("fcvt(\"%lf\" decpt) want %d got %d\n", t[i].f, t[i].decpt, dp);
            t_status = 1;
        }

        if (sign != t[i].sign) {
            printf("Array Number: t[%d]\n", i);
            printf("fcvt(\"%lf\" sign) want %d got %d\n",t[i].f, t[i].sign, sign);
            t_status = 1;
        }
    }
    return t_status;
}