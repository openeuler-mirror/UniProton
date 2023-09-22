#include <string.h>
#include "inode/inode.h"

int inode_nextname_test()
{
    int countErr = 0;
    char *testPath1 = "/home/test/name";
    char *testPath2 = "//usr1/./name/";
    char *testPath3 = "/usr/home////zzy/../name";

    char *tmp = inode_nextname(testPath1);
    if (strcmp("home/test/name", tmp) != 0) {
        countErr++;
    }
    tmp = inode_nextname(tmp);
    if (strcmp("test/name", tmp) != 0) {
        countErr++;
    }
    tmp = inode_nextname(tmp);
    if (strcmp("name", tmp) != 0) {
        countErr++;
    }
    tmp = inode_nextname(tmp);
    if (*tmp != '\0') {
        countErr++;
    }

    tmp = inode_nextname(testPath2);
    if (strcmp("usr1/./name/", tmp) != 0) {
        countErr++;
    }
    tmp = inode_nextname(tmp);
    if (strcmp("name/", tmp) != 0) {
        countErr++;
    }
    tmp = inode_nextname(tmp);
    if (*tmp != '\0') {
        countErr++;
    }

    tmp = inode_nextname(testPath3);
    if (strcmp("usr/home////zzy/../name", tmp) != 0) {
        countErr++;
    }
    tmp = inode_nextname(tmp);
    if (strcmp("home////zzy/../name", tmp) != 0) {
        countErr++;
    }
    tmp = inode_nextname(tmp);
    if (strcmp("zzy/../name", tmp) != 0) {
        countErr++;
    }
    tmp = inode_nextname(tmp);
    if (strcmp("../name", tmp) != 0) {
        countErr++;
    }
    tmp = inode_nextname(tmp);
    if (strcmp("name", tmp) != 0) {
        countErr++;
    }
    tmp = inode_nextname(tmp);
    if (*tmp != '\0') {
        countErr++;
    }

    if (countErr != 0) {
        return 1;
    }
    return 0;
}