/* bender – Copyright (c) 2015 Sven Michael Klose <pixel@hugbox.org> */

#ifndef DISASSEMBLER_H
#define DISASSEMBLER_H

#define DO_PRINT_LINEFEED       TRUE
#define DONT_PRINT_LINEFEED     FALSE
extern address disassemble (FILE*, address, int print_linefeed);

#endif /* #ifndef DISASSEMBLER_H */
