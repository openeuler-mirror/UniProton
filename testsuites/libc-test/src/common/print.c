#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include "test.h"

volatile int t_status = 0;

int t_printf(const char *s, ...)
{
	va_list ap;
	char buf[512] = {0};
	int n;

	t_status = 1;
	va_start(ap, s);
	n = vsnprintf(buf, sizeof buf, s, ap);
	va_end(ap);
	if (n < 0) {
		buf[0] = '\0';
	} else if (n >= sizeof buf) {
		n = sizeof buf;
		buf[n - 1] = '\0';
		buf[n - 2] = '\n';
		buf[n - 3] = '.';
		buf[n - 4] = '.';
		buf[n - 5] = '.';
	}
	printf(buf);
	return 0;
}
