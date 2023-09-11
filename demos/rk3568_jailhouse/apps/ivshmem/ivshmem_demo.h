#ifndef IVSHMEM_DEMO_H
#define IVSHMEM_DEMO_H

#define VENDORID			0x110a
#define DEVICEID			0x4106

#define PCI_CFG_VENDOR_ID	0x000
#define PCI_CFG_DEVICE_ID	0x002
#define PCI_CFG_COMMAND		0x004
#define PCI_CMD_IO		(1 << 0)
#define PCI_CMD_MEM		(1 << 1)
#define PCI_CMD_MASTER		(1 << 2)
#define PCI_CMD_INTX_OFF	(1 << 10)
#define PCI_CFG_STATUS		0x006
#define PCI_STS_INT		(1 << 3)
#define PCI_STS_CAPS		(1 << 4)
#define PCI_CFG_BAR		0x010
#define PCI_BAR_64BIT		0x4
#define PCI_CFG_CAP_PTR		0x034

#define PCI_ID_ANY		0xffff

#define PCI_DEV_CLASS_OTHER	0xff

#define PCI_CAP_MSI		0x05
#define PCI_CAP_VENDOR		0x09
#define PCI_CAP_MSIX		0x11

#define MSIX_CTRL_ENABLE	0x8000
#define MSIX_CTRL_FMASK		0x4000

#define PCI_REG_ADDR_PORT	0xcf8
#define PCI_REG_DATA_PORT	0xcfc

#define PCI_CONE		(1 << 31)

#define COMM_REGION_BASE	0x80000000
#define HUGE_PAGE_SIZE		(2 * 1024 * 1024ULL)
#define HUGE_PAGE_MASK		(~(HUGE_PAGE_SIZE - 1))

#define ICC_IAR1_EL1		SYSREG_32(0, c12, c12, 0)
#define ICC_EOIR1_EL1		SYSREG_32(0, c12, c12, 1)
#define ICC_PMR_EL1		SYSREG_32(0, c4, c6, 0)
#define ICC_CTLR_EL1		SYSREG_32(0, c12, c12, 4)
#define ICC_IGRPEN1_EL1		SYSREG_32(0, c12, c12, 7)

#define ICC_IGRPEN1_EN		0x1

#define MAIR_ATTR_SHIFT(__n)	((__n) << 3)
#define MAIR_ATTR(__n, __attr)	((__attr) << MAIR_ATTR_SHIFT(__n))
#define MAIR_ATTR_WBRWA		0xff
#define MAIR_ATTR_DEVICE	0x00    /* nGnRnE */

/* Common definitions for page table structure in long descriptor format */
#define LONG_DESC_BLOCK 0x1
#define LONG_DESC_TABLE 0x3

#define LATTR_CONT		(1 << 12)
#define LATTR_AF		(1 << 10)
#define LATTR_INNER_SHAREABLE	(3 << 8)
#define LATTR_MAIR(n)		(((n) & 0x3) << 2)

#define LATTR_AP(n)		(((n) & 0x3) << 6)
#define LATTR_AP_RW_EL1		LATTR_AP(0x0)

#define PGD_INDEX(addr)		((addr) >> 30)
#define PMD_INDEX(addr)		(((addr) >> 21) & 0x1ff)

#define PCI_DEV_CLASS_OTHER	0xff
#define IVSHMEM_CFG_STATE_TAB_SZ	0x04
#define IVSHMEM_CFG_RW_SECTION_SZ	0x08
#define IVSHMEM_CFG_OUT_SECTION_SZ	0x10
#define IVSHMEM_CFG_ADDRESS		0x18
#define JAILHOUSE_SHMEM_PROTO_UNDEFINED	0x0000

#define BAR_BASE			0xff000000
#define MAX_VECTORS	4
#define MAX_DEVICE_COUNT 0x10000

#define BYTES_NUMBER_ONE 1
#define BYTES_NUMBER_TWO 2
#define BYTES_NUMBER_FOUR 4
#define IN_SECTION_ADDRESS_TWO 0x2000
#define IN_SECTION_ADDRESS_THREE 0x4000
#define MAP_RANGE_SIZE 0x100000
#define TEST_SHMEME_INT 134

struct JailhouseConsole {
    U64 address;
    U32 size;
    U16 type;
    U16 flags;
    U32 divider;
    U32 gateNr;
    U64 clockReg;
} __attribute__((packed));

#define COMM_REGION_GENERIC_HEADER					\
    /** Communication region magic JHCOMM */			\
    char signature[6];						\
    /** Communication region ABI revision */			\
    U16 revision;							\
    /** Cell state, initialized by hypervisor, updated by cell. */	\
    volatile U32 cellState;					\
    /** Message code sent from hypervisor to cell. */		\
    volatile U32 msgToCell;					\
    /** Reply code sent from cell to hypervisor. */			\
    volatile U32 replyFromCell;					\
    /** Holds static flags, see JAILHOUSE_COMM_FLAG_*. */		\
    U32 flags;							\
    /** Debug console that may be accessed by the inmate. */	\
    struct JailhouseConsole console;				\
    /** Base address of PCI memory mapped config. */		\
    U64 pciMmconfigBase;

struct JailhouseCommRegion {
    COMM_REGION_GENERIC_HEADER;
    U8 gicVersion;
    U8 padding[7];
    U64 gicdBase;
    U64 giccBase;
    U64 gicrBase;
    U32 vpciIrqBase;
} __attribute__((packed));

struct IvshmRegs {
    U32 id;
    U32 maxPeers;
    U32 intControl;
    U32 doorbell;
    U32 state;
};

struct IvshmemDeviceData {
    U16 bdf;
    struct IvshmRegs *registers;
    U32 *stateTable;
    U32 stateTableSize;
    U32 *rwSection;
    U64 rwSectionSize;
    U32 *inSections;
    U32 *outSection;
    U64 outSectionSize;
    U32 *msixTable;
    U32 id;
    int msixCap;
};

#define commRegion	((struct JailhouseCommRegion *)COMM_REGION_BASE)

enum MapType { MAP_CACHED, MAP_UNCACHED };

#define JAILHOUSE_INMATE_MEM_PAGE_DIR_LEN	512  // diff
#define PAGE_SIZE	(4 * 1024ULL)

static U64 __attribute__((aligned(4096)))
    pageDirectory[JAILHOUSE_INMATE_MEM_PAGE_DIR_LEN];

#define DSB(domain)	asm volatile("dsb " #domain ::: "memory")

#define DEFAULT_IRQ_BASE	(commRegion->vpciIrqBase + 32)

int PciInit(void);
int PciDeviceFind(U16 vendor, U16 device, U16 start_bdf);
int PciCapFind(U16 bdf, U16 cap);
void IrqSend(struct IvshmemDeviceData *d);
int DeviceInit(struct IvshmemDeviceData *d);
void MmioWrite32(void *address, U32 value);
void PrintShmem(struct IvshmemDeviceData *d);
U32 MmioRead32(void *address);
void ShmemHandler(U32 intNum);
U32 TestShmemStart(void);

#endif
