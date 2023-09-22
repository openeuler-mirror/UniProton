#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <math.h>

double significand(double x)
{
	return scalbn(x, -ilogb(x));
}
