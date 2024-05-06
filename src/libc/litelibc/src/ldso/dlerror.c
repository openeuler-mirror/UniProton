#include "prt_dynamic_module.h"
char  *dlerror(void)
{
    return OsDynModuleGetError();
}
