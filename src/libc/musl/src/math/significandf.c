#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <math.h>

float significandf(float x)
{
	return scalbnf(x, -ilogbf(x));
}
