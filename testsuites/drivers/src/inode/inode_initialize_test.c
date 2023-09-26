#include "inode/inode.h"

int inode_initialize_test()
{
    if (g_root_inode != NULL) {
        return -1;
    }
    inode_initialize();
    if (g_root_inode == NULL) {
        return -1;
    }

    return 0;
}