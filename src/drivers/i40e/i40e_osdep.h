/*
 * Copyright (c) 2024-2024 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Create: 2024-1-24
 * Description: I40e网卡功能适配
 */

#ifndef _I40E_OSDEP_H_
#define _I40E_OSDEP_H_
#include <linux/types.h>
// #include<linux/error.h>
#include "errno.h"
#include <linux/module.h>
// #include<linux/pci.h>
#include <linux/delay.h>
#ifndef u8
typedef unsigned char    u8;
#endif
// typedef char   s8;
#ifndef u16
typedef unsigned short   u16;
#endif
#ifndef s16
typedef short  s16;
#endif
#ifndef u32
typedef unsigned int  u32;
#endif
#ifndef s32
typedef int  s32;
#endif
#ifndef u64
// typedef unsigned long  u64;
#endif
#define TRUE 1
#define FALSE 0
#define INLINE inline

#define DEBUGOUT(S)                printk(KERN_EMERG S )
#define DEBUGOUT1(S,A)             printk(KERN_EMERG S ,A)
#define DEBUGOUT2(S,A,B)           printk(KERN_EMERG S ,A,B)
#define DEBUGOUT3(S,A,B,C)         printk(KERN_EMERG S ,A,B,C)
#define DEBUGOUT7(S,A,B,C,D,E,F,G)      printk(KERN_EMERG S ,A,B,C,D,E,F,G)

#define DEBUGFUNC(S)                printk(KERN_EMERG S)

#define i40e_usec_delay(t) udelay(t)
#define i40e_msec_delay(t) mdelay(t)
#define i40e_memset(a, b, c, d) memset((a), (b), (c))
#define i40e_memcpy(a, b, c, d) memcpy((a), (b), (c))

#define i40e_debug(h, m, s, args...) printk(KERN_EMERG s ,##args)

#define CPU_TO_LE16(o) cpu_to_le16(o)
#define CPU_TO_LE32(s) cpu_to_le32(s)
#define CPU_TO_LE64(h) cpu_to_le64(h)
#define LE16_TO_CPU(a) le16_to_cpu(a)
#define LE32_TO_CPU(c) le32_to_cpu(c)
#define LE64_TO_CPU(k) le64_to_cpu(k)

#define ALIGN(x,a) (((x)+(a)-1)&~((a)-1))
// ------------------------------------------
// #include <pthread.h>
// typedef pthread_mutex_t mutex; // *** 危险，暂时先这么屏蔽掉 有可能会有问题
// ------------------------------------------
/* SW spinlock */
struct i40e_spinlock {
	//struct mutex spinlock; // *** 危险，暂时先这么屏蔽掉 有可能会有问题
	u64 a;
	u64 b;
	u64 c;
	u64 d;
};

struct i40e_dma_mem {
	void   *va;
	u64     pa;
	u64   size;
};

struct i40e_virt_mem {
	void *va;
	u32 size;
};

#define wr32(a, reg, value) writel((value), ((a)->hw_addr + (reg)))
#define rd32(a, reg)        readl((a)->hw_addr + (reg))

#define wr64(a, reg, value) writeq((value), ((a)->hw_addr + (reg)))
#define rd64(a, reg)        readq((a)->hw_addr + (reg))
#define i40e_flush(a)       readl((a)->hw_addr + I40E_GLGEN_STAT)
#define i40evf_flush(a)     readl((a)->hw_addr + I40E_VFGEN_RSTAT)

void ether_addr_copy(u8 *dst, const u8 *src);
// ------------------------------------------
#define __le16 u16
#define __le32 u32
#define __le64 u64
#define __be16 u16
#define __be32 u32
#define __be64 u64
// ------------------------------------------
#if 0


#include <sys/types.h>
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/endian.h>
#include <sys/mbuf.h>
#include <sys/protosw.h>
#include <sys/socket.h>
#include <sys/malloc.h>
#include <sys/kernel.h>
#include <sys/bus.h>
#include <machine/bus.h>
#include <sys/rman.h>
#include <machine/resource.h>
#include <vm/vm.h>
#include <vm/pmap.h>
#include <machine/clock.h>
#include <dev/pci/pcivar.h>
#include <dev/pci/pcireg.h>

#define ASSERT(x) if(!(x)) panic("IXL: x")

#define i40e_usec_delay(x) DELAY(x)
#define i40e_msec_delay(x) DELAY(1000*(x))

#define DBG 0 
#define MSGOUT(S, A, B)     printf(S "\n", A, B)
#define DEBUGFUNC(F)        DEBUGOUT(F);
#if DBG
	#define DEBUGOUT(S)         printf(S "\n")
	#define DEBUGOUT1(S,A)      printf(S "\n",A)
	#define DEBUGOUT2(S,A,B)    printf(S "\n",A,B)
	#define DEBUGOUT3(S,A,B,C)  printf(S "\n",A,B,C)
	#define DEBUGOUT7(S,A,B,C,D,E,F,G)  printf(S "\n",A,B,C,D,E,F,G)
#else
	#define DEBUGOUT(S)
	#define DEBUGOUT1(S,A)
	#define DEBUGOUT2(S,A,B)
	#define DEBUGOUT3(S,A,B,C)
	#define DEBUGOUT6(S,A,B,C,D,E,F)
	#define DEBUGOUT7(S,A,B,C,D,E,F,G)
#endif

#define UNREFERENCED_XPARAMETER
#define UNREFERENCED_PARAMETER(_p)
#define UNREFERENCED_1PARAMETER(_p)
#define UNREFERENCED_2PARAMETER(_p, _q)
#define UNREFERENCED_3PARAMETER(_p, _q, _r)
#define UNREFERENCED_4PARAMETER(_p, _q, _r, _s)

#define STATIC	static
#define INLINE  inline

#define FALSE               0
#define false               0 /* shared code requires this */
#define TRUE                1
#define true                1
#define CMD_MEM_WRT_INVALIDATE          0x0010  /* BIT_4 */
#define PCI_COMMAND_REGISTER            PCIR_COMMAND
#define ARRAY_SIZE(a)		(sizeof(a) / sizeof((a)[0]))

#define i40e_memset(a, b, c, d)  memset((a), (b), (c))
#define i40e_memcpy(a, b, c, d)  memcpy((a), (b), (c))

#define CPU_TO_LE16(o)	htole16(o)
#define CPU_TO_LE32(s)	htole32(s)
#define CPU_TO_LE64(h)	htole64(h)
#define LE16_TO_CPU(a)	le16toh(a)
#define LE32_TO_CPU(c)	le32toh(c)
#define LE64_TO_CPU(k)	le64toh(k)

#define I40E_NTOHS(a)	ntohs(a)
#define I40E_NTOHL(a)	ntohl(a)
#define I40E_HTONS(a)	htons(a)
#define I40E_HTONL(a)	htonl(a)

#define FIELD_SIZEOF(x, y) (sizeof(((x*)0)->y))

typedef uint8_t		u8;
typedef int8_t		s8;
typedef uint16_t	u16;
typedef int16_t		s16;
typedef uint32_t	u32;
typedef int32_t		s32;
typedef uint64_t	u64;

/* long string relief */
typedef enum i40e_status_code i40e_status;

#define __le16  u16
#define __le32  u32
#define __le64  u64
#define __be16  u16
#define __be32  u32
#define __be64  u64

/* SW spinlock */
struct i40e_spinlock {
        struct mtx mutex;
};

#define le16_to_cpu 

#if defined(__amd64__) || defined(i386)
static __inline
void prefetch(void *x)
{
	__asm volatile("prefetcht0 %0" :: "m" (*(unsigned long *)x));
}
#else
#define	prefetch(x)
#endif

struct i40e_osdep {
	bus_space_tag_t		mem_bus_space_tag;
	bus_space_handle_t	mem_bus_space_handle;
	bus_size_t		mem_bus_space_size;
	uint32_t		flush_reg;
	struct device		*dev;
};

struct i40e_dma_mem {
	void			*va;
	u64			pa;
	bus_dma_tag_t		tag;
	bus_dmamap_t		map;
	bus_dma_segment_t   seg;
	bus_size_t              size;
	int nseg;
	int   flags;
};

struct i40e_hw; /* forward decl */
u16 i40e_read_pci_cfg(struct i40e_hw *, u32);
void i40e_write_pci_cfg(struct i40e_hw *, u32, u16);

#define i40e_debug(h, m, s, ...) i40e_debug_d(h, m, s, ##__VA_ARGS__)
extern void i40e_debug_d(void *hw, u32 mask, char *fmt_str, ...);


struct i40e_virt_mem {
	void *va;
	u32 size;
};

/*
** This hardware supports either 16 or 32 byte rx descriptors;
** the driver only uses the 32 byte kind.
*/
#define i40e_rx_desc i40e_32byte_rx_desc

static __inline uint32_t
rd32_osdep(struct i40e_osdep *osdep, uint32_t reg)
{

	KASSERT(reg < osdep->mem_bus_space_size,
	    ("ixl: register offset %#jx too large (max is %#jx",
	    (uintmax_t)reg, (uintmax_t)osdep->mem_bus_space_size));

	return (bus_space_read_4(osdep->mem_bus_space_tag,
	    osdep->mem_bus_space_handle, reg));
}

static __inline void
wr32_osdep(struct i40e_osdep *osdep, uint32_t reg, uint32_t value)
{

	KASSERT(reg < osdep->mem_bus_space_size,
	    ("ixl: register offset %#jx too large (max is %#jx)",
	    (uintmax_t)reg, (uintmax_t)osdep->mem_bus_space_size));

	bus_space_write_4(osdep->mem_bus_space_tag,
	    osdep->mem_bus_space_handle, reg, value);
}

static __inline void
ixl_flush_osdep(struct i40e_osdep *osdep)
{
	rd32_osdep(osdep, osdep->flush_reg);
}

#define rd32(a, reg)		rd32_osdep((a)->back, (reg))
#define wr32(a, reg, value)	wr32_osdep((a)->back, (reg), (value))

#define rd64(a, reg) (\
   bus_space_read_8( ((struct i40e_osdep *)(a)->back)->mem_bus_space_tag, \
                     ((struct i40e_osdep *)(a)->back)->mem_bus_space_handle, \
                     reg))

#define wr64(a, reg, value) (\
   bus_space_write_8( ((struct i40e_osdep *)(a)->back)->mem_bus_space_tag, \
                     ((struct i40e_osdep *)(a)->back)->mem_bus_space_handle, \
                     reg, value))

#define ixl_flush(a)		ixl_flush_osdep((a)->back)
#endif

#endif /* _I40E_OSDEP_H_ */
