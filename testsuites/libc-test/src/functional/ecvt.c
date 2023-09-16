#include <string.h>
#include "stdio.h"
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
    {"0", 0.0, 1, 1, 0},
    {"00", 0.0, 2, 1, 0},
    {"00000", -0.0, 5, 1, 1},
    {"596186034813181", 0.59618603481318067, 17, 0, 0}, 
    {"596186034813181", 0.59618603481318067, 16, 0, 0}, 
    {"596186034813181", 0.59618603481318067, 15, 0, 0}, 
    {"59618603481318", 0.59618603481318067, 14, 0, 0}, 
    {"6", 0.59618603481318067, 1, 0, 0}, 
    {"118150131692180", 1.1815013169218038, 16, 1, 0}, 
    {"424207082357534", 42.420708235753445, 15, 2, 0}, 
    {"66566546863065", 665.66546863065162, 14, 3, 0}, 
    {"6101852922971", 6101.8529229708685, 13, 4, 0}, 
    {"769669520824", 76966.952082369677, 12, 5, 0}, 
    {"25050653222", 250506.53222286823, 11, 6, 0}, 
    {"2740037230", 2740037.2302280052, 10, 7, 0}, 
       {"2740037230", -2740037.2302280052, 10, 7, 1}, 
    {"207230935", 20723093.500497428, 9, 8, 0},
       {"207230935", -20723093.500497428, 9, 8, 1},
};

int ecvt_test(void)
{
    int i;
    int n, dp, sign;
    char *p;
    
    for (i = 0; i < length(t); i++) 
    {
        p = ecvt(t[i].f, t[i].ndigit, &dp, &sign);
        if (0 != strcmp((const char*)t[i].s, (const char*)p) ) {
            printf("Array Number: t[%d]\n", i);
            printf("ecvt(\"%f\") want %s got %s\n", t[i].f, t[i].s, p);
            t_status = 1;
        }

        if (dp != t[i].decpt) {
            printf("Array Number: t[%d]\n", i);
            printf("ecvt(\"%f\" decpt) want %d got %d\n", t[i].f, t[i].decpt, dp);
            t_status = 1;
        }

        if (sign != t[i].sign) {
            printf("Array Number: t[%d]\n", i);
            printf("ecvt(\"%f\" sign) want %d got %d\n", t[i].f, t[i].sign, sign); 
            t_status = 1;  
        }
    }
    return t_status;
}