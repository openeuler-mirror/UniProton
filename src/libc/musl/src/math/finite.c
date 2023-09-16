#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <math.h>

int finite(double x)
{
	return isfinite(x);
}
