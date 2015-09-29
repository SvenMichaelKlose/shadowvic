/* bender – Copyright (c) 2015 Sven Michael Klose <pixel@hugbox.org> */

#include <stdlib.h>
#include <stdio.h>

#include "types.h"
#include "6502.h"

#define AM_ACCU 1
#define AM_IMM  2
#define AM_ZP   4
#define AM_ZPX  8
#define AM_ABS  16
#define AM_ABSX 32
#define AM_ABSY 64
#define AM_IZPX 128
#define AM_IZPY 256
#define AM_INDI 512
#define AM_BRANCH 1024

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
    { "y", AM_IZPY + AM_ABSY },
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

void
disassemble (FILE * f, address pc)
{
    struct instruction * i = &opcode_map[m[pc]];
    struct operand_string * s = operand_strings;

    fprintf (f, "$%04hx: %s ", pc, i->mnemonic);

    print_operand_string (f, s++, i->addrmode);
    print_operand_string (f, s++, i->addrmode);

    if (i->addrmode & BYTE_AMS)
        fprintf (f, "$%02hx", m[pc + 1]);
    else if (i->addrmode & WORD_AMS)
        fprintf (f, "$%04hx", m[pc + 1] + (m[pc + 2] << 8));
    else if (i->addrmode & AM_BRANCH)
        fprintf (f, "$%04hx", pc + 2 + (char) m[pc + 1]);

    while (print_operand_string (f, s++, i->addrmode));

    fprintf (f, "\n");
}
