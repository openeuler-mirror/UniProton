/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_JIFFIES_H
#define _LINUX_JIFFIES_H

#include <linux/typecheck.h>
#include <asm/div64.h>

extern unsigned long volatile jiffies;
extern unsigned int get_HZ(void);

#define HZ get_HZ()

/*
 *    These inlines deal with timer wrapping correctly. You are 
 *    strongly encouraged to use them
 *    1. Because people otherwise forget
 *    2. Because if the timer wrap changes in future you won't have to
 *       alter your driver code.
 *
 * time_after(a,b) returns true if the time a is after time b.
 *
 * Do this with "<0" and ">=0" to only test the sign of the result. A
 * good compiler would generate better code (and a really good compiler
 * wouldn't care). Gcc is currently neither.
 */
#define time_after(a,b)        \
    (typecheck(unsigned long, a) && \
     typecheck(unsigned long, b) && \
     ((long)((b) - (a)) < 0))

#endif
