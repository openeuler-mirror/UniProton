#include "inode/inode.h"

int inode_addref_test() {
    struct inode testInode = { 0 };
    int ret = inode_addref(&testInode);
    if (testInode.i_crefs != 1) {
        return 1;
    }

    return 0;
}