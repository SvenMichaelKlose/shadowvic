/* shadowVIC – Copyright (c) 2015 Sven Michael Klose <pixel@hugbox.org> */

#include "types.h"
#include "6502.h"
#include "via.h"

byte enabled_interrupts = 0;

void
via_interrupt_enable (address addr, byte value)
{
    int do_enable = value & 128;

    if (do_enable)
        enabled_interrupts |= value & 0x7f;
    else
        enabled_interrupts &= 0x7f ^ value;
}

void
via_init ()
{
    writers[0x911e] = via_interrupt_enable;
}
