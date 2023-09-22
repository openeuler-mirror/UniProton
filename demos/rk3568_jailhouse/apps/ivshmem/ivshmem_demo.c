#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "securec.h"
#include "prt_config.h"
#include "prt_config_internal.h"
#include "prt_task.h"
#include "prt_typedef.h"
#include "prt_sys.h"
#include "prt_tick.h"
#include "ivshmem_demo.h"
#include "prt_mem.h"
#include "cpu_config.h"
#include "ivshmem.h"

extern int irqCounter;
extern struct IvshmemDeviceData dev;

static inline U8 MmioRead8(void *address)
{
    return *(volatile U8 *)address;
}

static inline U16 MmioRead16(void *address)
{
    return *(volatile U16 *)address;
}

U32 MmioRead32(void *address)
{
    return *(volatile U32 *)address;
}

static inline void SynchronizationBarrier(void)
{
    DSB(ish);
}

static inline void MmioWrite8(void *address, U8 value)
{
    *(volatile U8 *)address = value;
}

static inline void MmioWrite16(void *address, U16 value)
{
    *(volatile U16 *)address = value;
}

void MmioWrite32(void *address, U32 value)
{
    *(volatile U32 *)address = value;
}

void *Zalloc(unsigned long size, unsigned long align)
{
    void *base = PRT_MemAlloc(0, 0, size);
    if (base == NULL) {
        return NULL;
    }
    memset_s(base, size, 0, size);

    return base;
}

int MapRange(void *start, unsigned long size, enum MapType mapType)
{
    U64 addr, pmdEntry;
    unsigned pgdIndex;
    U64 *pmd;

    addr = (unsigned long)start;

    size += (addr & ~HUGE_PAGE_MASK) + HUGE_PAGE_SIZE - 1;
    size &= HUGE_PAGE_MASK;

    while (size) {
        pgdIndex = PGD_INDEX(addr);
        if (!(pageDirectory[pgdIndex] & LONG_DESC_TABLE)) {
            pmd = Zalloc(PAGE_SIZE, PAGE_SIZE);
            if (pmd == NULL) {
                return -1;
            }
            /* ensure the page table walker will see the zeroes */
            SynchronizationBarrier();

            pageDirectory[pgdIndex] = (unsigned long)pmd | LONG_DESC_TABLE;
        } else {
            pmd = (U64*)(unsigned long)(pageDirectory[pgdIndex] & ~LONG_DESC_TABLE);
        }

        pmdEntry = addr & HUGE_PAGE_MASK;
        pmdEntry |= LATTR_AF | LATTR_INNER_SHAREABLE | LATTR_AP_RW_EL1 | LONG_DESC_BLOCK;
        if (mapType == MAP_CACHED) {
            pmdEntry |= LATTR_MAIR(0);
        } else {
            pmdEntry |= LATTR_MAIR(1);
        }

        pmd[PMD_INDEX(addr)] = pmdEntry;
        size -= HUGE_PAGE_SIZE;
        addr += HUGE_PAGE_SIZE;
    }

    /*
     * As long es we only add entries and do not modify entries, a
     * synchronization barrier is enough to propagate changes. Otherwise we
     * need to flush the TLB.
     */
    SynchronizationBarrier();
    return OS_OK;
}

int PciInit(void)
{
    void *mmcfg = (void *)(unsigned long)commRegion->pciMmconfigBase;
    int ret;

    if (mmcfg) {
        ret = MapRange(mmcfg, MAP_RANGE_SIZE, MAP_UNCACHED);
        if (ret != OS_OK) {
            return -1;
        }
    }
    return OS_OK;
}

static void *PciGetDeviceCfgBase(U16 bdf)
{
    void *mmcfg = (void *)(unsigned long)commRegion->pciMmconfigBase;

    return mmcfg + ((unsigned long)bdf << 12);
}

U32 PciCfgRead(U16 bdf, unsigned int addr, unsigned int size)
{
    void *cfgAddr = PciGetDeviceCfgBase(bdf) + addr;

    switch (size) {
        case BYTES_NUMBER_ONE:
            return MmioRead8(cfgAddr);
        case BYTES_NUMBER_TWO:
            return MmioRead16(cfgAddr);
        case BYTES_NUMBER_FOUR:
            return MmioRead32(cfgAddr);
        default:
            return -1;
    }
}

U64 PciCfgRead64(U16 bdf, unsigned int addr)
{
    return PciCfgRead(bdf, addr, BYTES_NUMBER_FOUR) | ((U64)PciCfgRead(bdf, addr + 4, BYTES_NUMBER_FOUR) << 32);
}

void PciCfgWrite(U16 bdf, unsigned int addr, U32 value, unsigned int size)
{
    void *cfgAddr = PciGetDeviceCfgBase(bdf) + addr;

    switch (size) {
        case BYTES_NUMBER_ONE:
            MmioWrite8(cfgAddr, value);
            break;
        case BYTES_NUMBER_TWO:
            MmioWrite16(cfgAddr, value);
            break;
        case BYTES_NUMBER_FOUR:
            MmioWrite32(cfgAddr, value);
            break;
        default:
            return;
    }
}

int PciDeviceFind(U16 vendor, U16 device, U16 startBdf)
{
    unsigned int bdf;
    U16 id;

    for (bdf = startBdf; bdf < MAX_DEVICE_COUNT; bdf++) {
        id = PciCfgRead(bdf, PCI_CFG_VENDOR_ID, 2);
        if (id == PCI_ID_ANY || (vendor != PCI_ID_ANY && vendor != id)) {
            continue;
        }
 
        if (device == PCI_ID_ANY || PciCfgRead(bdf, PCI_CFG_DEVICE_ID, 2) == device) {
            return bdf;
        }
    }
    return -1;
}

int PciCapFind(U16 bdf, U16 cap)
{
    U8 pos = PCI_CFG_CAP_PTR - 1;

    if (!(PciCfgRead(bdf, PCI_CFG_STATUS, BYTES_NUMBER_TWO) & PCI_STS_CAPS)) {
        return -1;
    }

    while (1) {
        pos = PciCfgRead(bdf, pos + 1, BYTES_NUMBER_ONE);
        if (pos == 0) {
            return -1;
        }
        if (PciCfgRead(bdf, pos, BYTES_NUMBER_ONE) == cap) {
            return pos;
        }
    }
    return -1;
}

void PrintShmem(struct IvshmemDeviceData *d)
{
    PRT_Printf("state[0] = %d\n", d->stateTable[0]);
    PRT_Printf("state[1] = %d\n", d->stateTable[1]);
    PRT_Printf("state[2] = %d\n", d->stateTable[2]);
    PRT_Printf("rw[0] = %d\n", d->rwSection[0]);
    PRT_Printf("rw[1] = %d\n", d->rwSection[1]);
    PRT_Printf("rw[2] = %d\n", d->rwSection[2]);
    PRT_Printf("in@0x0000 = %d\n", d->inSections[0/4]);
    PRT_Printf("in@0x2000 = %d\n", d->inSections[IN_SECTION_ADDRESS_TWO/4]);
    PRT_Printf("in@0x4000 = %d\n", d->inSections[IN_SECTION_ADDRESS_THREE/4]);
}

void IrqSend(struct IvshmemDeviceData *d)
{
    unsigned int target = 0;
    U32 intNum = 0;

    PRT_Printf("\nIVSHMEM: sending IRQ %d to peer %d\n", intNum, target);
    MmioWrite32(&d->registers->doorbell, intNum | (target << 16));
}

int DeviceInit(struct IvshmemDeviceData *d)
{
    unsigned long baseAddr, addr, size;
    int vndrCap, n, ret;
    U32 maxPeers;

    vndrCap = PciCapFind(d->bdf, PCI_CAP_VENDOR);
    if (vndrCap < 0) {
        PRT_Printf("IVSHMEM ERROR: missing vendor capability\n");
        return;
    }
    d->registers = (struct IvshmRegs *)BAR_BASE;
    PciCfgWrite(d->bdf, PCI_CFG_BAR, (unsigned long)d->registers, BYTES_NUMBER_FOUR);
    PRT_Printf("IVSHMEM: bar0 is at %p\n", d->registers);

    d->msixTable = (U32 *)(BAR_BASE + PAGE_SIZE);
    PciCfgWrite(d->bdf, PCI_CFG_BAR + 4,
                (unsigned long)d->msixTable, BYTES_NUMBER_FOUR);
    PRT_Printf("IVSHMEM: bar1 is at %p\n", d->msixTable);

    PciCfgWrite(d->bdf, PCI_CFG_COMMAND,
                (PCI_CMD_MEM | PCI_CMD_MASTER), BYTES_NUMBER_TWO);

    ret = MapRange((void *)BAR_BASE, 2 * PAGE_SIZE, MAP_UNCACHED);
    if (ret != OS_OK) {
        return -1;
    }

    d->id = MmioRead32(&d->registers->id);
    PRT_Printf("IVSHMEM: ID is %d\n", d->id);

    d->stateTableSize =
        PciCfgRead(d->bdf, vndrCap + IVSHMEM_CFG_STATE_TAB_SZ, BYTES_NUMBER_FOUR);
    d->rwSectionSize =
        PciCfgRead64(d->bdf, vndrCap + IVSHMEM_CFG_RW_SECTION_SZ);
    d->outSectionSize =
        PciCfgRead64(d->bdf, vndrCap + IVSHMEM_CFG_OUT_SECTION_SZ);
    baseAddr = PciCfgRead64(d->bdf, vndrCap + IVSHMEM_CFG_ADDRESS);

    addr = baseAddr;
    d->stateTable = (U32 *)addr;

    addr += d->stateTableSize;
    d->rwSection = (U32 *)addr;

    addr += d->rwSectionSize;
    d->inSections = (U32 *)addr;

    addr += d->id * d->outSectionSize;
    d->outSection = (U32 *)addr;

    PRT_Printf("IVSHMEM: state table is at %p\n", d->stateTable);
    PRT_Printf("IVSHMEM: R/W section is at %p\n", d->rwSection);
    PRT_Printf("IVSHMEM: input sections start at %p\n", d->inSections);
    PRT_Printf("IVSHMEM: output section is at %p\n", d->outSection);

    size = d->stateTableSize + d->rwSectionSize +
        maxPeers * d->outSectionSize;
    ret = MapRange((void *)baseAddr, size, MAP_CACHED);
    if (ret != OS_OK) {
        return -1;
    }

    d->msixCap = PciCapFind(d->bdf, PCI_CAP_MSIX);

    return OS_OK;
}

void ShmemHandler(U32 intNum)
{
    irqCounter += 1;
    PRT_Printf("\n ShmemHandler: got interrupt %d (#%d)\n", intNum, irqCounter);
    PrintShmem(&dev);
}

U32 TestShmemStart(void)
{
    U32 ret;
    ret = PRT_HwiSetAttr(TEST_SHMEME_INT, 10, OS_HWI_MODE_ENGROSS);
    if (ret != OS_OK) {
        return ret;
    }

    ret = PRT_HwiCreate(TEST_SHMEME_INT, (HwiProcFunc)ShmemHandler, 0);
    if (ret != OS_OK) {
        return ret;
    }

    ret = PRT_HwiEnable(TEST_SHMEME_INT);
    if (ret != OS_OK) {
        return ret;
    }
    return OS_OK;
}