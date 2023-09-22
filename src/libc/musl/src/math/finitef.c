#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <math.h>

int finitef(float x)
{
	return isfinite(x);
}
