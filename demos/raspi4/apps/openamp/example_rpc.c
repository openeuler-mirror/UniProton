#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>


#include "openamp/open_amp.h"

#define RPMSG_RPC_SERVICE_NAME "rpmsg-rpc"
#define ADDR                    0xFE
#define DEST_ADDR               0xFF

static struct rpmsg_endpoint ept;

extern void rpmsg_set_default_ept(struct rpmsg_endpoint *ept);

extern int rpmsg_client_cb(struct rpmsg_endpoint *ept,
                    void *data, size_t len,
                    uint32_t src, void *priv);

extern int rpc_test();

int rpmsg_endpoint_init(struct rpmsg_device *rdev)
{
    int err;

    err = rpmsg_create_ept(&ept, rdev, RPMSG_RPC_SERVICE_NAME,
                           ADDR, DEST_ADDR,
                           rpmsg_client_cb, rpmsg_destroy_ept);
    if (err) {
        return err;
    }

    rpmsg_set_default_ept(&ept);
    return err;
}

#define REDEF_O_CREAT   0000100
#define REDEF_O_EXCL    0000200
#define REDEF_O_RDONLY  0000000
#define REDEF_O_WRONLY  0000001
#define REDEF_O_RDWR    0000002
#define REDEF_O_APPEND  0002000
#define REDEF_O_ACCMODE 0000003

static int write_str()
{
    char *fname = "/tmp/remote.file";
    char *str = "Hello!";
    int fd = 0;
    int ret = 0;
    off_t off = 0;
    char rbuff[100];

    fd = open(fname, REDEF_O_CREAT | REDEF_O_RDWR | REDEF_O_APPEND,
           S_IRUSR | S_IWUSR);
    if (fd < 0) {
        PRT_Printf("open file '%s' fail, ret: %d\r\n", fname, fd);
        return fd;
    }

    ret = write(fd, str, strlen(str));
    if (ret < 0) {
        PRT_Printf("write fail, ret: %d\r\n", ret);
        goto close_file;
    }

close_file:
    close(fd);
    return ret >= 0 ? 0 : -1;
}

void example_init()
{
    write_str();
}
