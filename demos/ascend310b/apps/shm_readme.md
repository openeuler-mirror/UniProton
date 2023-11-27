> 本共享内存基于`fusiondock`的虚拟化底座，提供给上层VM进行信息交互。当前开启两块共享内存区域，物理地址和内存区块大小固定。VM之间通过IPI中断进行交互，用以一个VM通知另外一个VM进行访问。

[TOC]

# 共享内存的使用

## 共享内存的地址和大小

`fusiondock`在物理地址上注册了两块共享内存区域。

第一块区域的起始物理地址为0x2A800000，长度当前为0x400000，根据需要可定制。

第二块区域的起始物理地址为0x2AC00000，长度当前为0x400000，根据需要可定制。

使用两块共享内存，可以实现全双工的首发操作。一块共享内存用于VM0写入，VM1读取，一块用于VM0读取，VM1写入。权限当前为两个VM均开放读写，具体使用方式由用户端自行定义。

## 共享内存基本使用

虚拟化底座本身完成了对共享内存的分配和映射，在VM上面可以直接访问该内存区域。`UniProton`侧可以通过物理地址的方式直接访问，Open Euler-rt`侧则需要通过文件系统功能访问物理内存。两类VM的操作方式demo详见后文描述。

理论上讲，用户可以根据自身需要，直接操作内存实现业务功能。不过为了尽量减少一些问题，底座提供了一套用于共享内存访问和操作的头文件、和功能函数定义。

## 公共头文件和功能函数

由文件`shm_pub.h`和`shm_pub.c`来定义。

主要的数据结构为`shm_info_s`，即用来访问共享内存区域的数据结构。该结构体指针直接指向共享内存的地址，从而访问该共享内存区域。

数据结构如下：

```c
typedef struct shm_info_t{
    unsigned int max_size;          // 数据区最大可写入长度
    unsigned int used_size;         // 本次传输数据长度
    int op_type;                    // 该内存当前响应的操作状态
    int resevered;
    char data[0];                   // 数据区
} shm_info_s;
```

开放的接口如下：

```c
void shm_ipi_init(void); // 用来使能IPI中断功能的相关函数，调用的具体函数的功能实现依赖不同VM的操作系统各自实现。
int shm_send_ipi(int vmid); // 用来给某个VM发送IPI中断
int shm_write(void *src, unsigned int len, int data_type, shm_info_s *shm, int recId); // 写共享内存接口，用来往共享内存区域写入数据并发送IPI中断给特定的VM进行通知
int shm_read(shm_info_s *shm, void *data, unsigned int len); // 读共享内存接口，用来从共享内存区域读数据到私有内有区域。
```

### 调用接口示例

为了方便数据的访问的安全，提供了通用的接口，进行共享内存的数据读写并发送IPI中断通知到另外一个VM。该接口会自行校验数据长度和共享内存长度等信息，并进行同一块区域收发的简单自旋锁等待防止访问冲突。该自旋锁可以关闭避免操作不当引起无限等待卡死。

通过函数`shm_write`可以将本地内存内容写入共享内存，`shm_read`可以将共享内存的数据读取到本地内存。如果若将一个整数写入共享内存，并在vm1读取，可以采用如下方法。

```c
// VM0写入，VM0该共享内存地址映射为0x2AC00000
int i = 999;
shm_info_s *shm_write = (shm_info_s *)0x2AC00000;
shm_write(&i, sizeof(i), 0, shm_write, 1); // 第三个参数当前用来与读取方对应数据的处理方式，即0号方式；第五个参数通知给VM1

// VM1读取，VM1该共享内存地址映射为0x2AC00000
char buf[0x20]; // buf长度由业务自行把握。如果超出要读取数据的长度会报错
shm_info_s *shm_read = (shm_info_s *)0x2AC00000;
if (shm_read(shm_read, buf, sizeof(buf) < 0) {
    /* 读取异常 */
    return -1;
}
if (shm_read->resevered == 0) {
    int i = *(int *)buf;
}
```


## `UniProton`端共享内存的操作

参考代码`uniproton_shm_demo.c`和`uniproton_shmIpi.c`，实现依赖公共文件`shm_pub.h`和`shm_pub.c`。

### 访问方式

由于`UniProton`未使用2阶段页表翻译，可以直接访问IPA。因此由`fusiondock`注册给`UniProton`的共享内存地址可以直接在`UniProton`使用。

```c
#define SHM_RD_ADDR 0x2A800000
#define SHM_WR_ADDR 0x2AC00000
```

创建共享内存的访问结构体`shm_info_s`直接指向该内存地址，即可进行共享内存的访问。

```c
shm_info_s *shm_rd = (shm_info_s *)SHM_RD_ADDR;
shm_info_s *shm_wr = (shm_info_s *)SHM_WR_ADDR;
```

数据的读写可以采用上述接口调用的方式进行。

```c
    /* 整数发送测试 */
    int tmp = 8080808;
    for (int i = 0; i < 10; i++) {
        shm_write(&tmp, sizeof(tmp), 0, shm_wr, IPI_TARGET);
        PRT_Printf("[UniProton] write to shm int: %d\n", tmp--);
        PRT_TaskDelay(OS_TICK_PER_SECOND);
    }

    /* 字符串发送测试 */
    char str[] = "Hello openEuler";
    shm_write(str, sizeof(str), 1, shm_wr, IPI_TARGET);
    PRT_Printf("[UniProton] write to shm string: %s\n", str);
    PRT_TaskDelay(OS_TICK_PER_SECOND);

...
    char buf[0x2100] = {0};
    switch (shm_rd->resevered) {
        case 0:
            /* 整数读取测试 */
            shm_read(shm_rd, buf, sizeof(buf));
            PRT_Printf("[UniProton]read from shm: %d\n", *(int *)buf);
            break;
        case 1:
            /* 字符串读取测试 */
            shm_read(shm_rd, buf, sizeof(buf));
            PRT_Printf("[UniProton]read len: 0x%lx, st: %s\n", shm_rd->used_size, buf);
            break;
        case ...
    }
```

除了按照上述使用提供的接口进行共享内存读写之外，也可以直接通过修改结构体不同字段的值来进行直接的控制。操作方式可以参考封装的接口的实现。

```c
/* 手动测试 */
shm_wr->used_size = sizeof(unsigned long long);
shm_wr->resevered = 2;
shm_wr->op_type = SHM_OP_READY_TO_READ;
asm volatile ("mrs %0, CNTPCT_EL0\n" : "=r" (t));
*(unsigned long long *)shm_wr->data = t;
shm_send_ipi(IPI_TARGET);
```



## openEuler-rt端共享内存的操作

参考代码`linux_shm_demo.c`、`linux_shmIpi.c`，实现依赖公共文件`shm_pub.h`和`shm_pub.c`。

### 驱动安装

当前提供了`mcs_km.ko`，首先需要通过`insmod`命令安装该驱动模块。

### 访问方式

`openEuler-rt`需要通过设备文件的方式来访问到物理内存。具体操作方式如下：

```c
#define SHM_WR_ADDR 0x2A800000
#define SHM_RD_ADDR 0x2AC00000

int fd = open ("/dev/mem", O_RDWR | O_SYNC);
if (fd == -1) {
    printf("open /dev/mem failed\n");
    return -1;
}

/* 获取写入共享内存区域的访问指针 */
unsigned char *w_mem = mmap (NULL, MEM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, SHM_WR_ADDR);
if (w_mem == 0) {
    printf("Null pointer!\n");
    close(fd);
    return -1;
} else {
    printf("mmap write mem success!\n");
}

/* 获取读取共享内存区域的访问指针 */
unsigned char *r_mem = mmap (NULL, MEM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, SHM_RD_ADDR);
if (r_mem == 0) {
    printf("Null pointer!\n");
    close(fd);
    return -1;
} else {
    printf("mmap read mem success!\n");
}
```

通过上述文件系统和`mmap`的方式，即可获得两块共享内存的访问指针。

通过这两个访问指针，强转为共享内存信息结构体，即可使用公共接口实现共享内存访问，与`UniProton`侧类似，不再赘述。

```c
shm_info_s *shm = (shm_info_s *)arg;
/* 整数发送测试 */
for (int i = 0; i < 5; i++) {
    shm_write(&i, sizeof(i), 0, shm, IPI_TARGET);
    printf("[linux] write to shm int: %d\n", i);
    sleep(1);
}
```

