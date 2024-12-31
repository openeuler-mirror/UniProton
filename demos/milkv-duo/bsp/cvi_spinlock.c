// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2022. All rights reserved.
 *
 * File Name: cvi_spinlock.c
 * Description:
 */

#include <stdint.h>
#include "csr.h"
#include "csi_rv64_gcc.h"
#include "core_rv64.h"
#include "arch_time.h"
#include "top_reg.h"
#include "cvi_spinlock.h"
#include "delay.h"
#include "mmio.h"
#include "uart.h"

static unsigned long reg_base = SPINLOCK_REG_BASE;

static unsigned char lockCount[SPIN_MAX+1] = {0};

void cvi_spinlock_init()
{
	uart_printf("[%s] succeess\n" , __func__);
}

void cvi_spinlock_deinit(void)
{
}

void spinlock_base(unsigned long mb_base)
{
	reg_base = mb_base;
}

static int hw_spin_trylock(hw_raw_spinlock_t *lock)
{
    writew(lock->locks, reg_base + sizeof(int) * lock->hw_field);
    if (readw(reg_base + sizeof(int) * lock->hw_field) == lock->locks)
        return MAILBOX_LOCK_SUCCESS;
    return MAILBOX_LOCK_FAILED;
}

int hw_spin_lock(hw_raw_spinlock_t *lock)
{
    u64 i;
    u64 loops = 1000000;
    hw_raw_spinlock_t _lock = {.hw_field = lock->hw_field, .locks = lock->locks};
    if (lock->hw_field >= SPIN_LINUX_RTOS)
    {
        if (lockCount[lock->hw_field] == 0) 
        {
            lockCount[lock->hw_field]++;
        }
        _lock.locks = (lockCount[lock->hw_field] << 8);
        lockCount[lock->hw_field]++;
    }
    else 
    {
        unsigned long systime = GetSysTime();
        /* lock ID can not be 0, so set it to 1 at least */
        if ((systime & 0xFFFF) == 0)
            systime = 1;
        lock->locks = (unsigned short) (systime & 0xFFFF);
    }
    for (i = 0; i < loops; i++) 
    {
        if (hw_spin_trylock(&_lock) == MAILBOX_LOCK_SUCCESS)
        {
            lock->locks = _lock.locks;
            return MAILBOX_LOCK_SUCCESS;
        }
	udelay(1);
    }
    uart_printf("__spin_lock_debug fail\n");
    return MAILBOX_LOCK_FAILED;
}

int _hw_raw_spin_lock_irqsave(hw_raw_spinlock_t *lock)
{
	int flag = 0;
	// save and disable irq
	flag = (__get_MSTATUS() & 8);
	__disable_irq();
	// lock
	if (hw_spin_lock(lock) == MAILBOX_LOCK_FAILED) 
        {
	    // if spinlock failed , restore irq
            if (flag)
            {
                __enable_irq();
            }
            uart_printf("spin lock fail! reg_val=0x%x, lock->locks=0x%x\n", readw(reg_base + sizeof(int) * lock->hw_field), lock->locks);
            return MAILBOX_LOCK_FAILED;
        }
        return flag;
}

void _hw_raw_spin_unlock_irqrestore(hw_raw_spinlock_t *lock, int flag)
{
	// unlock
	if (readw(reg_base + sizeof(int) * lock->hw_field) == lock->locks) 
        {
            writew(lock->locks, reg_base + sizeof(int) * lock->hw_field);
            // restore irq
            if (flag)
            {
                __enable_irq();
            }
        } 
        else 
        {
            uart_printf("spin unlock fail! reg_val=0x%x, lock->locks=0x%x\n",readw(reg_base + sizeof(int) * lock->hw_field), lock->locks);
	}
}

