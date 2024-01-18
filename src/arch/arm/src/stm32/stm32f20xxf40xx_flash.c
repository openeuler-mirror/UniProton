/****************************************************************************
 * arch/arm/src/stm32/stm32f20xxf40xx_flash.c
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 ****************************************************************************/

/* Provides standard flash access functions, to be used by the  flash mtd
 * driver.  The interface is defined in the include/nuttx/progmem.h
 *
 * Requirements during write/erase operations on FLASH:
 *  - HSI must be ON.
 *  - Low Power Modes are not permitted during write/erase
 */

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include <nuttx/arch.h>
#include <nuttx/mutex.h>

#include <stdbool.h>
#include <assert.h>
#include <errno.h>

#include "stm32_flash.h"
#include "stm32_rcc.h"
#include "stm32_waste.h"
#include "arm_internal.h"

/* Only for the STM32F[2|4]0xx family. */

#if defined(CONFIG_STM32_STM32F20XX) || defined (CONFIG_STM32_STM32F4XXX)

#if defined(CONFIG_STM32_FLASH_CONFIG_DEFAULT)
#  warning "Default Flash Configuration Used - See Override Flash Size Designator"
#endif

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define FLASH_KEY1         0x45670123
#define FLASH_KEY2         0xcdef89ab
#define FLASH_OPTKEY1      0x08192a3b
#define FLASH_OPTKEY2      0x4c5d6e7f
#define FLASH_ERASEDVALUE  0xff

/****************************************************************************
 * Private Data
 ****************************************************************************/

static mutex_t g_lock = NXMUTEX_INITIALIZER;

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static void flash_unlock(void)
{
  while (getreg32(STM32_FLASH_SR) & FLASH_SR_BSY)
    {
      stm32_waste();
    }

  if (getreg32(STM32_FLASH_CR) & FLASH_CR_LOCK)
    {
      /* Unlock sequence */

      putreg32(FLASH_KEY1, STM32_FLASH_KEYR);
      putreg32(FLASH_KEY2, STM32_FLASH_KEYR);
    }
}

static void flash_lock(void)
{
  modifyreg32(STM32_FLASH_CR, 0, FLASH_CR_LOCK);
}

#if defined(CONFIG_STM32_FLASH_WORKAROUND_DATA_CACHE_CORRUPTION_ON_RWW)
static void data_cache_disable(void)
{
  modifyreg32(STM32_FLASH_ACR, FLASH_ACR_DCEN, 0);
}

static void data_cache_enable(void)
{
  /* Reset data cache */

  modifyreg32(STM32_FLASH_ACR, 0, FLASH_ACR_DCRST);

  /* Enable data cache */

  modifyreg32(STM32_FLASH_ACR, 0, FLASH_ACR_DCEN);
}
#endif /* defined(CONFIG_STM32_FLASH_WORKAROUND_DATA_CACHE_CORRUPTION_ON_RWW) */

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int stm32_flash_unlock(void)
{
  int ret;

  ret = nxmutex_lock(&g_lock);
  if (ret < 0)
    {
      return ret;
    }

  flash_unlock();
  nxmutex_unlock(&g_lock);

  return ret;
}

int stm32_flash_lock(void)
{
  int ret;

  ret = nxmutex_lock(&g_lock);
  if (ret < 0)
    {
      return ret;
    }

  flash_lock();
  nxmutex_unlock(&g_lock);

  return ret;
}

/****************************************************************************
 * Name: stm32_flash_writeprotect
 *
 * Description:
 *   Enable or disable the write protection of a flash sector.
 *
 ****************************************************************************/

int stm32_flash_writeprotect(size_t page, bool enabled)
{
  uint32_t reg;
  uint32_t val;

  if (page >= STM32_FLASH_NPAGES)
    {
      return -EFAULT;
    }

  /* Select the register that contains the bit to be changed */

  if (page < 12)
    {
      reg = STM32_FLASH_OPTCR;
    }
#if defined(CONFIG_STM32_FLASH_CONFIG_I)
  else
    {
      reg = STM32_FLASH_OPTCR1;
      page -= 12;
    }
#else
  else
    {
      return -EFAULT;
    }
#endif

  /* Read the option status */

  val = getreg32(reg);

  /* Set or clear the protection */

  if (enabled)
    {
      val &= ~(1 << (16 + page));
    }
  else
    {
      val |=  (1 << (16 + page));
    }

  /* Unlock options */

  putreg32(FLASH_OPTKEY1, STM32_FLASH_OPTKEYR);
  putreg32(FLASH_OPTKEY2, STM32_FLASH_OPTKEYR);

  /* Write options */

  putreg32(val, reg);

  /* Trigger programming */

  modifyreg32(STM32_FLASH_OPTCR, 0, FLASH_OPTCR_OPTSTRT);

  /* Wait for completion */

  while (getreg32(STM32_FLASH_SR) & FLASH_SR_BSY)
    {
      stm32_waste();
    }

  /* Relock options */

  modifyreg32(STM32_FLASH_OPTCR, 0, FLASH_OPTCR_OPTLOCK);
  return 0;
}

#if defined(CONFIG_FS_FAT) && defined(OS_OPTION_STM32_FLASH)
static const size_t page_sizes[STM32_FLASH_NPAGES] = STM32_FLASH_SIZES;

size_t up_progmem_pagesize(size_t page)
{
  if (page * OS_OPTION_FATFS_PAGE_SIZE > STM32_FLASH_SIZE - page_sizes[STM32_FLASH_NPAGES - 1]) {
    return 0;
  }

  return OS_OPTION_FATFS_PAGE_SIZE;
}
#else
size_t up_progmem_pagesize(size_t page)
{
  static const size_t page_sizes[STM32_FLASH_NPAGES] = STM32_FLASH_SIZES;

  if (page >= sizeof(page_sizes) / sizeof(*page_sizes))
    {
      return 0;
    }
  else
    {
      return page_sizes[page];
    }
}
#endif
size_t up_progmem_erasesize(size_t block)
{
  return up_progmem_pagesize(block);
}

extern size_t up_progmem_neraseblocks(void);
ssize_t up_progmem_getpage(size_t addr)
{
  size_t page_end = 0;
  size_t i;

  if (addr >= STM32_FLASH_BASE)
    {
      addr -= STM32_FLASH_BASE;
    }
#if defined(CONFIG_FS_FAT) && defined(OS_OPTION_STM32_FLASH)
  if (addr >= STM32_FLASH_SIZE - page_sizes[STM32_FLASH_NPAGES - 1])
#else
  if (addr >= STM32_FLASH_SIZE)
#endif
    {
      return -EFAULT;
    }

#if defined(CONFIG_FS_FAT) && defined(OS_OPTION_STM32_FLASH)
  size_t all_blocks = up_progmem_neraseblocks();
  for (i = 0; i < all_blocks; ++i)
#else
  for (i = 0; i < STM32_FLASH_NPAGES; ++i)
#endif
    {
      page_end += up_progmem_pagesize(i);
      if (page_end > addr)
        {
          return i;
        }
    }

  return -EFAULT;
}

size_t up_progmem_getaddress(size_t page)
{
  size_t base_address = STM32_FLASH_BASE;
  size_t i;
#if defined(CONFIG_FS_FAT) && defined(OS_OPTION_STM32_FLASH)
  if (page * OS_OPTION_FATFS_PAGE_SIZE > STM32_FLASH_SIZE - page_sizes[STM32_FLASH_NPAGES - 1])
#else
  if (page >= STM32_FLASH_NPAGES)
#endif
    {
      return SIZE_MAX;
    }

  for (i = 0; i < page; ++i)
    {
      base_address += up_progmem_pagesize(i);
    }

  return base_address;
}

#if defined(CONFIG_FS_FAT) && defined(OS_OPTION_STM32_FLASH)
size_t up_progmem_neraseblocks(void)
{
  return ((STM32_FLASH_SIZE - page_sizes[STM32_FLASH_NPAGES - 1]) / OS_OPTION_FATFS_PAGE_SIZE);
}
#else
size_t up_progmem_neraseblocks(void)
{
  return STM32_FLASH_NPAGES;
}
#endif

bool up_progmem_isuniform(void)
{
#ifdef STM32_FLASH_PAGESIZE
  return true;
#else
  return false;
#endif
}

ssize_t up_progmem_ispageerased(size_t page)
{
  size_t addr;
  size_t count;
  size_t bwritten = 0;

#if defined(CONFIG_FS_FAT) && defined(OS_OPTION_STM32_FLASH)
  if (page * OS_OPTION_FATFS_PAGE_SIZE > STM32_FLASH_SIZE)
#else
  if (page >= STM32_FLASH_NPAGES)
#endif
    {
      return -EFAULT;
    }

  /* Verify */

  for (addr = up_progmem_getaddress(page), count = up_progmem_pagesize(page);
       count; count--, addr++)
    {
      if (getreg8(addr) != FLASH_ERASEDVALUE)
        {
          bwritten++;
        }
    }

  return bwritten;
}

#if defined(CONFIG_FS_FAT) && defined(OS_OPTION_STM32_FLASH)
extern ssize_t up_progmem_eraseblock_original(size_t block);

static void get_origin_beg_end_pages(size_t original_block, size_t *begin_block, size_t *end_block)
{
  size_t bblock = 0;
  size_t eblock = 0;
  size_t i = 0;
  while (i <= original_block) {
    bblock = eblock;
    eblock += page_sizes[i] / OS_OPTION_FATFS_PAGE_SIZE;
    i++;
  }

  *begin_block = bblock;
  *end_block = eblock;
}

extern ssize_t up_progmem_write(size_t addr, const void *buf, size_t count);

static ssize_t copy_block(size_t src_begin, size_t src_end, size_t dest_begin, size_t dest_end)
{
  if (dest_end == 0) {
    dest_end = dest_begin + (src_end - src_begin);
  }
  if ((dest_end - dest_begin) < (src_end - src_begin)) {
    return 0;
  }
  if (dest_begin >= src_begin && dest_begin < src_end) {
    return 0;
  }
  if (dest_end >= src_begin && dest_end < src_end) {
    return 0;
  }
  if (src_begin >= dest_begin && src_begin < dest_end) {
    return 0;
  }
  if (src_end > dest_begin && src_end < dest_end) {
    return 0;
  }

  uintptr_t src_beg_addr = STM32_FLASH_BASE + (src_begin * OS_OPTION_FATFS_PAGE_SIZE);
  uintptr_t dest_beg_addr = STM32_FLASH_BASE + (dest_begin * OS_OPTION_FATFS_PAGE_SIZE);
  size_t copy_size = (src_end - src_begin) * OS_OPTION_FATFS_PAGE_SIZE;
  size_t ret = up_progmem_write(dest_beg_addr, (void *)src_beg_addr, copy_size);
  if (ret != copy_size) {
    return ret;
  }

  return copy_size;
}

ssize_t up_progmem_eraseblock(size_t block)
{
  size_t original_block;
  size_t block_size = 0;
  if (block * OS_OPTION_FATFS_PAGE_SIZE > STM32_FLASH_SIZE - page_sizes[STM32_FLASH_NPAGES - 1]) {
    return -EFAULT;
  }

  for (original_block = 0; original_block <  sizeof(page_sizes) / sizeof(*page_sizes); original_block++) {
    block_size += page_sizes[original_block];
    if (block_size > block * OS_OPTION_FATFS_PAGE_SIZE) {
      break;
    }
  }

  size_t erase_size = up_progmem_eraseblock_original(STM32_FLASH_NPAGES - 1);
  if (erase_size != page_sizes[STM32_FLASH_NPAGES - 1]) {
    return -EIO; /* failure */
  }
  size_t begin_block = 0;
  size_t end_block = 0;
  size_t bblock = 0;
  size_t eblock = 0;
  get_origin_beg_end_pages(original_block, &begin_block, &end_block);
  get_origin_beg_end_pages(STM32_FLASH_NPAGES - 1, &bblock, &eblock);

  // 拷贝源扇区数据到备份扇区
  size_t copy_size = (end_block - begin_block) * OS_OPTION_FATFS_PAGE_SIZE;
  if (copy_block(begin_block, end_block, bblock, eblock) != copy_size) {
    return -EIO; /* failure */
  }

  erase_size = up_progmem_eraseblock_original(original_block);
  if (erase_size != page_sizes[original_block]) {
    return -EIO; /* failure */
  }
  // 将备份数据拷贝回去，刨去需要擦除的数据
  copy_size = (end_block - begin_block - 1) * OS_OPTION_FATFS_PAGE_SIZE;
  eblock = bblock + (end_block - begin_block);
  if (block == begin_block) {
    if (copy_block(bblock + 1, eblock, begin_block + 1, 0) != copy_size) {
      return -EIO; /* failure */
    }
  } else if (block == end_block - 1) {
    if (copy_block(bblock, eblock - 1, begin_block, 0) != copy_size) {
      return -EIO; /* failure */
    }
  } else {
    copy_size = (block - begin_block) * OS_OPTION_FATFS_PAGE_SIZE;
    eblock = bblock + (block - begin_block);
    if (copy_block(bblock, eblock, begin_block, 0) != copy_size) {
      return -EIO; /* failure */
    }
    bblock = eblock + 1;
    eblock = bblock + (end_block - (block + 1));
    copy_size = (end_block - (block + 1)) * OS_OPTION_FATFS_PAGE_SIZE;
    if (copy_block(bblock, eblock, block + 1, 0) != copy_size) {
      return -EIO; /* failure */
    }
  }

  return up_progmem_pagesize(block);
}

static ssize_t original_block_ispageerased(size_t block)
{
  size_t begin_block = 0;
  size_t end_block = 0;

  get_origin_beg_end_pages(block, &begin_block, &end_block);
  for ( ; begin_block < end_block; begin_block++) {
    if (up_progmem_ispageerased(begin_block) != 0) {
      return -EIO; /* failure */
    }
  }

  return 0; /* success */
}

ssize_t up_progmem_eraseblock_original(size_t block)
#else
ssize_t up_progmem_eraseblock(size_t block)
#endif
{
  if (block >= STM32_FLASH_NPAGES)
    {
      return -EFAULT;
    }

  nxmutex_lock(&g_lock);

  /* Get flash ready and begin erasing single block */

  flash_unlock();

  modifyreg32(STM32_FLASH_CR, 0, FLASH_CR_SER);
  modifyreg32(STM32_FLASH_CR, FLASH_CR_SNB_MASK, FLASH_CR_SNB(block));
  modifyreg32(STM32_FLASH_CR, 0, FLASH_CR_STRT);

  while (getreg32(STM32_FLASH_SR) & FLASH_SR_BSY)
    {
      stm32_waste();
    }

  modifyreg32(STM32_FLASH_CR, FLASH_CR_SER, 0);
  nxmutex_unlock(&g_lock);

  /* Verify */
#if defined(CONFIG_FS_FAT) && defined(OS_OPTION_STM32_FLASH)
  if (original_block_ispageerased(block) == 0)
    {
      return page_sizes[block];
    }
#else
  if (up_progmem_ispageerased(block) == 0)
    {
      return up_progmem_pagesize(block); /* success */
    }
#endif
  else
    {
      return -EIO; /* failure */
    }
}

ssize_t up_progmem_write(size_t addr, const void *buf, size_t count)
{
  uint16_t *hword = (uint16_t *)buf;
  size_t written = count;

  /* STM32 requires half-word access */

  if (count & 1)
    {
      return -EINVAL;
    }

  /* Check for valid address range */

  if (addr >= STM32_FLASH_BASE)
    {
      addr -= STM32_FLASH_BASE;
    }

  if ((addr + count) > STM32_FLASH_SIZE)
    {
      return -EFAULT;
    }

  nxmutex_lock(&g_lock);

  /* Get flash ready and begin flashing */

  flash_unlock();

#if defined(CONFIG_STM32_FLASH_WORKAROUND_DATA_CACHE_CORRUPTION_ON_RWW)
  data_cache_disable();
#endif

  modifyreg32(STM32_FLASH_CR, 0, FLASH_CR_PG);

  /* TODO: implement up_progmem_write() to support other sizes than 16-bits */

  modifyreg32(STM32_FLASH_CR, FLASH_CR_PSIZE_MASK, FLASH_CR_PSIZE_X16);

  for (addr += STM32_FLASH_BASE; count; count -= 2, hword++, addr += 2)
    {
      /* Write half-word and wait to complete */

      putreg16(*hword, addr);

      while (getreg32(STM32_FLASH_SR) & FLASH_SR_BSY)
        {
          stm32_waste();
        }

      /* Verify */

      if (getreg32(STM32_FLASH_SR) & FLASH_CR_SER)
        {
          modifyreg32(STM32_FLASH_CR, FLASH_CR_PG, 0);
          nxmutex_unlock(&g_lock);
          return -EROFS;
        }

      if (getreg16(addr) != *hword)
        {
          modifyreg32(STM32_FLASH_CR, FLASH_CR_PG, 0);
          nxmutex_unlock(&g_lock);
          return -EIO;
        }
    }

  modifyreg32(STM32_FLASH_CR, FLASH_CR_PG, 0);

#if defined(CONFIG_STM32_FLASH_WORKAROUND_DATA_CACHE_CORRUPTION_ON_RWW)
  data_cache_enable();
#endif

  nxmutex_unlock(&g_lock);
  return written;
}

uint8_t up_progmem_erasestate(void)
{
  return FLASH_ERASEDVALUE;
}

#endif /* defined(CONFIG_STM32_STM32F20XX) || defined(CONFIG_STM32_STM32F4XXX) */
