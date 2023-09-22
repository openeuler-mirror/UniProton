#include <linux/uaccess.h>
#include <string.h>

#include "prt_typedef.h"
#include "prt_cpu_external.h"

unsigned long copy_from_user(void *to, const void *from, unsigned long n)
{
    uintptr_t intSave;
    intSave = OsIntLock();
    memcpy(to, from, (size_t)n);
    OsIntRestore(intSave);
    return 0;
}
