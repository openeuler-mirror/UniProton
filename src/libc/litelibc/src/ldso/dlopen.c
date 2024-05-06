#include "prt_dynamic_module.h"
void *dlopen(const char *file, int mode)
{
	return OsDynModuleLoad(file, mode);
}
