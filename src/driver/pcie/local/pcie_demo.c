
#include "pcie.h"

char g_vga_driver_name[] = "huawei_vga";

#define PCI_VENDOR_ID 0x19e5
#define PCI_DEVICE_ID 0x1711

U32 pcie_scan_device(U16 device_id, U16 vendor_id);

#define error -1
#define ok 0
void test(void)
{
    BDF_U bdf;
    U32 dd = pcie_scan_device(PCI_VENDOR_ID, PCI_DEVICE_ID);
    if (dd == -1) {
        return error;
    }

    bdf = BDF_GET_FROM_DD(dd);


}





