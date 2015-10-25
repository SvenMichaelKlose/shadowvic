/* bender – Copyright (c) 2015 Sven Michael Klose <pixel@hugbox.org> */

extern address pc;
extern byte a;
extern byte x;
extern byte y;
extern byte s;

extern byte m[65536];

extern byte mos6502_fetch_byte (void);
extern address mos6502_fetch_word (void);
extern void mos6502_jam (void);

extern void mos6502_reset (void);
extern void mos6502_emulate (void);
extern void mos6502_interrupt (address);
extern int mos6502_interrupt_flag ();
