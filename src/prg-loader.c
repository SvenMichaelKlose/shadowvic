/* shadowVIC – Copyright (c) 2015 Sven Michael Klose <pixel@hugbox.org> */

#include <stdio.h>

#include "types.h"
#include "6502.h"

void
load_prg (char * name)
{
    address load_addr;

    FILE * f = fopen (name, "r");
    (void) fread (&load_addr, 2, 1, f);
    (void) fread (&m[load_addr], 65536, 1, f);
    fclose (f);
}
