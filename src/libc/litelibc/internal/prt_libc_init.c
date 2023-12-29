#include <stdlib.h>

/* From newlib init.c */
extern void (*__preinit_array_start []) (void) __attribute__((__weak__));
extern void (*__preinit_array_end []) (void) __attribute__((__weak__));
extern void (*__init_array_start []) (void) __attribute__((__weak__));
extern void (*__init_array_end []) (void) __attribute__((__weak__));

extern void _init (void);

/* Iterate over all the init routines.  */
void __attribute__((__weak__)) __libc_init_array (void)
{
    size_t count;
    size_t i;

    count = __preinit_array_end - __preinit_array_start;
    for (i = 0; i < count; i++)
        __preinit_array_start[i] ();

    _init ();

    count = __init_array_end - __init_array_start;
    for (i = 0; i < count; i++)
        __init_array_start[i] ();
}