/* bender – Copyright (c) 2015 Sven Michael Klose <pixel@hugbox.org> */

#include <stdlib.h>
#include <stdio.h>

#include "types.h"
#include "6502.h"

#define AM_ACCU 1
#define AM_IMM  2
#define AM_ZP   4
#define AM_ZPX  8
#define AM_ZPY  16
#define AM_ABS  32
#define AM_ABSX 64
#define AM_ABSY 128
#define AM_IZPX 256
#define AM_IZPY 512
#define AM_INDI 1024
#define AM_BRANCH 2048

#define BYTE_AMS (AM_IMM | AM_ZP | AM_ZPX | AM_IZPX | AM_IZPY)
#define WORD_AMS (AM_ABS | AM_ABSX | AM_ABSY | AM_INDI)

#include "opcode-map.c"

struct operand_string {
    const char * str;
    int addrmode;
} operand_strings[] = {
    { "#", AM_IMM },
    { "(", AM_IZPX + AM_IZPY + AM_INDI },

    { ")", AM_IZPY },
    { ",", AM_ZPX + AM_IZPX + AM_IZPY + AM_ABSX + AM_ABSY },
    { "x", AM_ZPX + AM_IZPX + AM_ABSX },
    { "y", AM_ZPY + AM_IZPY + AM_ABSY },
    { ")", AM_IZPX + AM_INDI },
    { NULL, 0 }
};

const char *
print_operand_string (FILE * f, struct operand_string * s, int addrmode)
{
    if (s->addrmode & addrmode)
        fprintf (f, "%s", s->str);

    return s->str;
}

address
disassemble (FILE * f, address p, int print_linefeed)
{
    struct instruction * i = &opcode_map[m[p]];
    struct operand_string * o = operand_strings;
#ifdef STACKDUMP
    byte sp = s;
#endif

    fprintf (f, "%04hx: %s ", p, i->mnemonic);
    p++;

    print_operand_string (f, o++, i->addrmode);
    print_operand_string (f, o++, i->addrmode);

    if (i->addrmode & BYTE_AMS) {
        fprintf (f, "$%02hx", m[p]);
        p++;
    } else if (i->addrmode & WORD_AMS) {
        fprintf (f, "$%04hx", m[p] + (m[p + 1] << 8));
        p += 2;
    } else if (i->addrmode & AM_BRANCH) {
        fprintf (f, "$%04hx", p + 1 + (char) m[p]);
        p++;
    }

    while (print_operand_string (f, o++, i->addrmode));

#ifdef STACKDUMP
    while (sp < 0xff)
        fprintf (f, " $%02hx", m[++sp + 0x100]);
#endif

    if (print_linefeed)
        fprintf (f, "\n");

    return p;
}
