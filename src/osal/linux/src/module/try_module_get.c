#include <linux/module.h>
#include <linux/types.h>

bool try_module_get(struct module *module)
{
    return true;
}