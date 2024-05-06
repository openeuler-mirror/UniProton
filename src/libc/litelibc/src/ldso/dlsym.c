#include "prt_dynamic_module.h"
void *dlsym(void *restrict p, const char *restrict s)
{
	return OsDynModuleFind(p, s);
}
