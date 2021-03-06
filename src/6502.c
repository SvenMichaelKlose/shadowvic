/* bender – Copyright (c) 2015 Sven Michael Klose <pixel@hugbox.org>
            Copyright (c) 2015 Eric Hilaire
            Copyright (c) 2016 Gábor Lenárt */

#include <stdlib.h>
#include <stdio.h>

#include "types.h"
#include "6502.h"
#include "disassembler.h"
#include "debugger.h"

#define FLAG_C  1
#define FLAG_Z  2
#define FLAG_I  4
#ifndef CPU_2A03
#define FLAG_D  8
#endif
#define FLAG_B  16
#define FLAG_V  64
#define FLAG_N  128


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
writer writers[65536];

byte e_fetch_byte ()            { return m[pc++]; }
byte mos6502_fetch_byte ()      { return e_fetch_byte (); }
address e_fetch_word ()         { return e_fetch_byte () + (e_fetch_byte () << 8); }
address mos6502_fetch_word ()   { return e_fetch_word (); }

address e_get_word (address e)    { return m[e] + (m[e + 1] << 8); }
address e_get_zp_word (address e) { return m[e] + (m[(e + 1) & 0xff] << 8); }

void
ram_write (address addr, byte value)
{
    m[addr] = value;
}

void
e_writeback ()
{
    if (operand_is_accu) {
        a = r;
        operand_is_accu = FALSE;
    } else
        writers[operand] (operand, r);
}


/*
 * Addressing modes
 */

void
e_accu ()
{
    operand_is_accu = TRUE;
    r = a;
}

byte e_get_operand (address e) { return m[operand = e]; }
void e_imm ()  { r = e_fetch_byte (); }
void e_zp ()   { r = e_get_operand (e_fetch_byte ()); }
void e_zpx ()  { r = e_get_operand ((e_fetch_byte () + x) & 0xff); }
void e_zpy ()  { r = e_get_operand ((e_fetch_byte () + y) & 0xff); }
void e_abs ()  { r = e_get_operand (e_fetch_word ()); }
void e_absx () { r = e_get_operand (e_fetch_word () + x); }
void e_absy () { r = e_get_operand (e_fetch_word () + y); }
void e_izpx () { r = e_get_operand (e_get_zp_word ((e_fetch_byte () + x) & 0xff)); }
void e_izpy () { r = e_get_operand (e_get_zp_word (e_fetch_byte ()) + y); }
void e_indi () { operand = e_get_word (e_fetch_word ()); }

void
e_branch ()
{
    int e = e_fetch_byte ();
    r = pc + ((128 <= e) ? e - 256 : e);
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
    return (c ? FLAG_C : 0) +
           (z ? FLAG_Z : 0) +
           (i ? FLAG_I : 0) +
#ifndef CPU_2A03
           (d ? FLAG_D : 0) +
#endif
           32 +
           (v ? FLAG_V : 0) +
           (n ? FLAG_N : 0);
}

byte mos6502_flags () { return e_get_flags (); }

void
e_set_flags (byte f)
{
    c = z = i = d = v = n = 0;
    if (f & FLAG_C) c = 1;
    if (f & FLAG_Z) z = 1;
    if (f & FLAG_I) i = 1;
#ifndef CPU_2A03
    if (f & FLAG_D) d = 1;
#endif
    if (f & FLAG_V) v = 1;
    if (f & FLAG_N) n = 1;
}

void
e_arith_flags (byte e)
{
    n = e & 0x80;
    z = !e;
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
    address e;

#ifndef CPU_2A03
    if (d) {
	    address e2 = (a & 0xF0) + (r & 0xF0);
        e = (a & 0x0F) + (r & 0x0F) + (c ? 1 : 0);
        if (e > 9) { e2 += 0x10; e += 6;
    }
    v = (~(a ^ r) & (a ^ e) & 0x80);
    if (e2 > 0x90)
        e2 += 0x60;
    c = (e2 > 0xFF);
    e_arith_flags (a = (e & 0x0F) + (e2 & 0xF0));
    } else {
#endif
        e = a + r + (c ? 1 : 0);
        //v = (a ^ r) & (a ^ e) & 0x80;
	v = (!((a ^ r) & 0x80) && ((a ^ e) & 0x80));
        c = 0xff < e ? 1 : 0;
	e_arith_flags (a = e);
#ifndef CPU_2A03
    }
#endif
}

void
e_sbc ()
{
#ifndef CPU_2A03
    if (d) {
	    address e = a - (r & 0x0F) - (c ? 0 : 1);
	    if ((e & 0x0F) > (a & 0x0F))
            e -= 6;
	    e -= (r & 0xF0);
	    if ((e & 0xF0) > (a & 0xF0))
            e -= 0x60;
	    v = (!(e > a));
	    c = (!(e > a));
	    e_arith_flags (a = e);
    } else {
#endif
        r ^= 0xff;
        e_adc ();
#ifndef CPU_2A03
   }
#endif
}


/*
 * Logical
 */

void e_and () { e_arith_flags (a &= r); }
void e_ora () { e_arith_flags (a |= r); }
void e_eor () { e_arith_flags (a ^= r); }


/*
 * Shifting
 */

void
e_asl ()
{
    c = r & 0x80;
    e_arith_flags (r <<= 1);
}

void
e_lsr ()
{
    c = r & 0x01;
    e_arith_flags (r >>= 1);
}

void
e_rol ()
{
    byte old_c = c;
    c = r & 0x80;
    e_arith_flags (r = (r << 1) | (old_c ? 1 : 0));
}

void
e_ror ()
{
    byte old_c = c;
    c = r & 0x01;
    e_arith_flags (r = (r >> 1) | (old_c ? 128 : 0));
}


/*
 * Comparing
 */

void
e_bit ()
{
    e_arith_flags (a & r);
    n = r & 0x80;
    v = r & 0x40;
}

void
e_cmp_shared (byte o)
{
    int diff = o - r;
    e_arith_flags (diff);
    c = 0 <= diff;
}

void e_cmp () { e_cmp_shared (a); }
void e_cpx () { e_cmp_shared (x); }
void e_cpy () { e_cmp_shared (y); }


/*
 * Stack
 */

void e_push (byte o) { m[s-- + 0x100] = o; }
byte e_pop ()        { return m[++s + 0x100]; }

void e_pha () { e_push (a); }
void e_pla () { e_arith_flags(a = e_pop ()); }
void e_php () { e_push (e_get_flags () | FLAG_B); }
void e_plp () { e_set_flags (e_pop ()); }


/*
 * Transfer
 */

void e_tax () { e_arith_flags (x = a); }
void e_txa () { e_arith_flags (a = x); }
void e_tay () { e_arith_flags (y = a); }
void e_tya () { e_arith_flags (a = y); }
void e_txs () { s = x; }
void e_tsx () { e_arith_flags (x = s); }


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
e_rts ()
{
    pc = (e_pop () + (e_pop () << 8)) + 1;
}

void
e_jsr ()
{
    pc--;
    e_push_pc ();
    pc++;
    e_jmp ();
}


/*
 * Conditional
 */

void e_cond (byte o) { if (o) pc = r; }
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
    e_push (e_get_flags () | FLAG_B);
    i = 1;
    pc = e_get_word (0xfffe);
}

void
e_rti ()
{
    e_plp ();
    pc = e_pop () + (e_pop () << 8);
}


/*
 * Miscellaneous
 */

void e_nop () {}

void
e_ill ()
{
    printf ("Illegal opcode %02hx at %04hx.\n", opcode, pc - 1);
#ifdef WITHOUT_DEBUGGER
    exit (255);
#else
    (void) debugger ();
#endif
}

#include "6502-instructions.c"

void
mos6502_reset ()
{
    e_set_flags (4);
    s = 0xff;
    pc = m[0xfffc] + (m[0xfffd] << 8);
}

void (*debugger_hook) () = NULL;

void
mos6502_set_debugger_hook (void (*fun) ())
{
    debugger_hook = fun;
}

void
mos6502_emulate ()
{
#ifdef DISASSEMBLE
    disassemble (stdout, pc, 1);
#endif
    opcode = e_fetch_byte ();
    instructions[opcode] ();
    if (debugger_hook)
        debugger_hook ();
}

void
mos6502_interrupt (address new_pc)
{
    e_push_pc ();
    e_push (e_get_flags () & ~FLAG_B);
    i = 1;
    pc = new_pc;
}

int mos6502_interrupt_flag () { return i; }

void
mos6502_init ()
{
    int i;

    for (i = 0; i < 0x10000; i++)
        writers[i] = ram_write;
}
