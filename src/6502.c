/* bender – Copyright (c) 2015 Sven Michael Klose <pixel@hugbox.org> */

#include <stdlib.h>
#include <stdio.h>

#include "types.h"
#include "6502.h"
#include "disassembler.h"

/*
 * Registers
 */

address pc;
byte a;
byte x;
byte y;
byte s;

byte n;
byte v;
byte b;
byte d;
byte i;
byte z;
byte c;

address operand;
address r;
byte operand_is_accu = FALSE;
byte opcode;


/*
 * Memory
 */

byte m[65536];

byte e_fetch_byte () { return m[pc++]; }
byte mos6502_fetch_byte () { return e_fetch_byte (); }

address
e_fetch_word ()
{
    byte l = e_fetch_byte ();
    byte h = e_fetch_byte ();
    return l + (h << 8);
}

address
e_get_word (address x)
{
    return m[x] + (m[x + 1] << 8);
}


/*
 * Operands
 */

byte
e_get_operand (address a)
{
    operand = a;
    return m[operand];
}

void
e_accu ()
{
    operand_is_accu = TRUE;
    r = a;
}

void e_imm ()  { r = e_fetch_byte (); }
void e_zp ()   { r = e_get_operand (e_fetch_byte ()); }
void e_zpx ()  { r = e_get_operand (e_fetch_byte () + x); }
void e_abs ()  { r = e_get_operand (e_fetch_word ()); }
void e_absx () { r = e_get_operand (e_fetch_word () + x); }
void e_absy () { r = e_get_operand (e_fetch_word () + y); }
void e_izpx () { r = e_get_operand (e_get_word (e_fetch_byte () + x)); }
void e_izpy () { r = e_get_operand (e_get_word (e_fetch_byte ()) + y); }
void e_indi () { operand = e_get_word (e_fetch_word ()); }

void
e_branch ()
{
    int x = e_fetch_byte ();
    r = pc + ((128 <= x) ? x - 256 : x);
}

void
e_writeback ()
{
    if (operand >= 0xc000 || (operand >= 0x8000 && operand < 0x9000))
        return;
    if (operand_is_accu) {
        a = r;
        operand_is_accu = FALSE;
    } else
        m[operand] = r;
}


/*
 * Flags
 */

void e_sec () { c = 1; }
void e_clc () { c = 0; }
void e_sei () { i = 1; }
void e_cli () { i = 0; }
void e_sed () { d = 1; }
void e_cld () { d = 0; }
void e_clv () { v = 0; }

byte
e_get_flags ()
{
    return (c ? 1 : 0) +
           (z ? 2 : 0) +
           (i ? 4 : 0) +
           (d ? 8 : 0) +
           (b ? 16 : 0) +
           32 +
           (v ? 64 : 0) +
           (n ? 128 : 0);
}

void
e_set_flags (byte f)
{
    c = z = i = d = v = n = 0;
    if (f & 1) c = 1;
    if (f & 2) z = 1;
    if (f & 4) i = 1;
    if (f & 8) d = 1;
    if (f & 16) b = 1;
    if (f & 64) v = 1;
    if (f & 128) n = 1;
}

void
e_arith_flags (byte x)
{
    n = x & 0x80;
    z = !x;
}


/*
 * Load/store
 */

void e_lda () { e_arith_flags (a = r); }
void e_ldx () { e_arith_flags (x = r); }
void e_ldy () { e_arith_flags (y = r); }

void e_sta () { r = a; }
void e_stx () { r = x; }
void e_sty () { r = y; }


/*
 * Arithmetics
 */

void e_dec () { e_arith_flags (--r); }
void e_dex () { e_arith_flags (--x); }
void e_dey () { e_arith_flags (--y); }

void e_inc () { e_arith_flags (++r); }
void e_inx () { e_arith_flags (++x); }
void e_iny () { e_arith_flags (++y); }

void
e_adc ()
{
    address x = a + r + (c ? 1 : 0);
    v = (a ^ r) & (a ^ x) & 0x80;
    c = 0xff < x ? 1 : 0;
    a = x;
    e_arith_flags (a);
}

void
e_sbc ()
{
    r = r ^ 0xff;
    e_adc ();
}


/*
 * Logical
 */

void
e_and ()
{
    a = a & r;
    e_arith_flags (a);
}

void
e_ora ()
{
    a = a | r;
    e_arith_flags (a);
}

void
e_eor ()
{
    a = a ^ r;
    e_arith_flags (a);
}


/*
 * Shift
 */

void
e_asl ()
{
    c = r & 0x80;
    r = r << 1;
    e_arith_flags (r);
}

void
e_lsr ()
{
    c = r & 0x01;
    r = r >> 1;
    e_arith_flags (r);
}

void
e_rol ()
{
    byte old_c = c;
    c = r & 0x80;
    r = (r << 1) | (old_c ? 1 : 0);
    e_arith_flags (r);
}

void
e_ror ()
{
    byte old_c = c;
    c = r & 0x01;
    r = (r >> 1) | (old_c ? 128 : 0);
    e_arith_flags (r);
}


/*
 * Compare
 */

void
e_bit ()
{
    v = r & 0x40;
    e_arith_flags (r);
}

void
e_cmp_shared (byte a)
{
    int diff = a - r;
    n = diff & 0x80;
    z = !(byte) diff;
    c = 0 <= diff;
}

void e_cmp () { e_cmp_shared (a); }
void e_cpx () { e_cmp_shared (x); }
void e_cpy () { e_cmp_shared (y); }


/*
 * Stack
 */

void
e_push (byte x)
{
    m[s + 0x100] = x;
    s--;
}

byte
e_pop ()
{
    s++;
    return m[s + 0x100];
}

void e_pha () { e_push (a); }
void e_pla () { a = e_pop (); }
void e_php () { e_push (e_get_flags ()); }
void e_plp () { e_set_flags (e_pop ()); }


/*
 * Transfer
 */

void e_tax () { x = a; }
void e_txa () { a = x; }
void e_tay () { y = a; }
void e_tya () { a = y; }
void e_txs () { s = x; }
void e_tsx () { x = s; }


/*
 * Jumps & calls
 */

void e_jmp () { pc = operand; }

void
e_push_pc ()
{
    e_push (pc >> 8);
    e_push (pc & 0xff);
}

void
e_pop_pc ()
{
    pc = e_pop () + (e_pop () << 8);
}

void
e_jsr ()
{
    e_push_pc ();
    pc = operand;
}

void e_rts () { e_pop_pc (); }


/*
 * Conditional
 */

void e_cond (byte x) { if (x) pc = r; }
void e_bne () { e_cond (!z); }
void e_beq () { e_cond (z); }
void e_bcc () { e_cond (!c); }
void e_bcs () { e_cond (c); }
void e_bpl () { e_cond (!n); }
void e_bmi () { e_cond (n); }
void e_bvc () { e_cond (!v); }
void e_bvs () { e_cond (v); }


/*
 * Interrupts
 */

void
e_brk ()
{
    pc++;
    e_push_pc ();
    e_php ();
    b = 1;
    pc = e_get_word (0xfffe);
}

void
e_rti ()
{
    e_plp ();
    e_pop_pc ();
}


/*
 * Miscellaneous
 */

void e_nop () {}

void
e_ill ()
{
    printf ("Illegal opcode %d at %d.\n", opcode, pc - 1);
    exit (255);
}

#include "6502-instructions.c"

void
mos6502_reset ()
{
    e_set_flags (4);
    s = 0xff;
    pc = m[0xfffc] + (m[0xfffd] << 8);
}

void
mos6502_emulate ()
{
#ifdef DISASSEMBLE
    disassemble (stdout, pc);
#endif
    opcode = e_fetch_byte ();
    instructions[opcode] ();
}

void
mos6502_interrupt (address new_pc)
{
    e_push_pc ();
    e_php ();
    pc = new_pc;
}

int mos6502_interrupt_flag () { return i; }
