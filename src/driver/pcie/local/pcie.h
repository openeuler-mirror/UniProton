
#ifndef __PCIE_H__
#define __PCIE_H__

#include "prt_typedef.h"

typedef union {
    struct {
        U32 function : 3;
        U32 device : 5;
        U32 bus : 8;
    } bdf;
    U16 value;
} BDF_U;

#define BDF_GET_FROM_DD(dd) (BDF_U)((U16)(dd))

#define BAR_NUM 6
typedef struct tagPCI_DEVICE {
    U16 bdf;
    uintptr_t bar_phy[BAR_NUM];
    uintptr_t bar_virt[BAR_NUM];
    U32 bar_size[BAR_NUM];
    void *pci_data;
    struct tagPCI_DEVICE *prev;
    struct tagPCI_DEVICE *next;
} pci_device_t;

#endif