#include "prt_dynamic_module.h"
int dlclose(void *p)
{
	OsDynModuleUnload(p);
    return 0;
}
