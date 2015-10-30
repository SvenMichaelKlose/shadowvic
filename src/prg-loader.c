/* shadowVIC – Copyright (c) 2015 Sven Michael Klose <pixel@hugbox.org> */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "types.h"
#include "6502.h"
#include "prg-loader.h"

void
load_prg (struct prg_info * i)
{
    int    fd;

    if ((fd = open (i->pathname, O_RDONLY)) < 0) {
        printf ("Could not open '%s': %s\n", i->pathname, strerror (errno));
        return;
    }
    if (i->is_relocated) {
        (void) read (fd, &i->load_addr, 2);
        i->load_addr = m[0x2b] + (m[0x2c] << 8);
        printf ("Relocating '%s' to %04hx.\n", i->pathname, i->load_addr);
    } else {
        (void) read (fd, &i->load_addr, 2);
        printf ("Loading '%s' to %04hx.\n", i->pathname, i->load_addr);
    }

    i->size = read (fd, &m[i->load_addr], 65536);
    printf ("Loaded %d bytes.\n", i->size);
    close (fd);
}
