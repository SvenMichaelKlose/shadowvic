/* shadowVIC – Copyright (c) 2015 Sven Michael Klose <pixel@hugbox.org> */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

#include "types.h"
#include "6502.h"
#include "disassembler.h"
#include "debugger.h"
#include "linenoise.h"


#ifndef HISTORY
#define HISTORY     "shadowvic-debugger.history"
#endif

#ifndef DEBUGGER_WELCOME
#define DEBUGGER_WELCOME "\nWelcome to the shadowVIC debugger.\n"
#endif

#ifndef DISASSEMBLY_LENGTH
#define DISASSEMBLY_LENGTH  16
#endif

#define STAY_IN_DEBUGGER        FALSE
#define RETURN_FROM_DEBUGGER    TRUE

int debugger_break = FALSE;
int debugger_return_address = -1;
int debugger_until_address = -1;


char *
skip_whitespace (char * p)
{
    while (*p && *p < ' ')
        p++;
    return p;
}

address
parse_address (char * p)
{
    return strtol (p, NULL, 16);
}

void
memory_dump_hex (address from, address to)
{
    int i;

    for (i = 0; i < 16; i++) {
        if (from >= to)
            break;
        if (i == 8)
            printf (" ");
        printf (" %02hx", m[from++]);
    }
}

address
memory_dump_ascii (address from, address to)
{
    int i;
    byte c;

    for (i = 0; i < 16; i++) {
        if (from >= to)
            break;
        c = m[from++];
        if (c < 32 || c > 126)
            c = '.';
        putc (c, stdout);
    }

    return from;
}

void
memory_dump (address from, address to)
{
    while (from < to) {
        printf ("%04hx:", from);
        memory_dump_hex (from, to);
        printf ("  ");
        from = memory_dump_ascii (from, to);
        printf ("\n");
    }
}


address next_disassembly_address = 0;

int
disassembly (char * p)
{
    int i = DISASSEMBLY_LENGTH;
    if (*p)
        next_disassembly_address = parse_address (p);

    while (i--)
        next_disassembly_address = disassemble (stdout, next_disassembly_address, DO_PRINT_LINEFEED);

    return STAY_IN_DEBUGGER;
}


int
step_instruction ()
{
    return debugger_break = TRUE;
}


#define OPCODE_BRK  0x00
#define OPCODE_JSR  0x20

int
next_instruction ()
{
    byte opcode = m[pc];

    if (opcode != OPCODE_BRK && opcode != OPCODE_JSR)
        return step_instruction ();
    debugger_return_address = s;
    mos6502_emulate ();

    return RETURN_FROM_DEBUGGER;
}


int
execute_until (char * p)
{
    if (!*p) {
        printf ("Address expected where to halt execution.\n");
        return STAY_IN_DEBUGGER;
    }

    debugger_until_address = parse_address (p);

    return RETURN_FROM_DEBUGGER;
}


int
execute_from (char * p)
{
    if (!*p) {
        printf ("Address expected where to resume execution.\n");
        return STAY_IN_DEBUGGER;
    }

    pc = parse_address (p);

    return RETURN_FROM_DEBUGGER;
}


void
dump_flags ()
{
    const char * flags = "NV.BDIZC";
    int i;
    byte f = mos6502_flags ();

    for (i = 0; i < 8; i++) {
        putc (f & 128 ? flags[i] : '-', stdout);
        f <<= 1;
    }
}

int
dump_registers (char * p)
{
    printf ("A: %02hx X: %02hx Y: %02hx S: %02hx ", a, x, y, s);
    dump_flags ();
    printf ("\n");

    return STAY_IN_DEBUGGER;
}


address next_memory_dump_address = 0;

int
dump_memory (char * p)
{
    address from = next_memory_dump_address;
    if (*p)
        from = parse_address (p);

    next_memory_dump_address = from + 128;
    memory_dump (from, next_memory_dump_address);

    return STAY_IN_DEBUGGER;
}


int
backtrace (char * p)
{
    address sp = s;
    (void) p;

    printf ("Stack: ");
    while (sp < 0xff)
        printf (" $%02hx", m[++sp + 0x100]);
    printf ("\n");

    return STAY_IN_DEBUGGER;
}


int
exit_shadowvic (char * p)
{
    (void) p;

    printf ("Exiting with code 255 – bye! :)\n");
    exit (255);

    return STAY_IN_DEBUGGER;
}


int
print_help ()
{
    printf ("Command overview:\n\n");
    printf ("Ctrl+d  Continue program.\n");
    printf ("s       Step a single instruction.\n");
    printf ("n       Step to next instruction, don't step into BRK or JSR.\n");
    printf ("u addr  Continue program until 'addr' is reached.\n");
    printf ("g addr  Resume program at 'addr'.\n");
    printf ("d addr  Disassemble %d items at 'addr'.\n", DISASSEMBLY_LENGTH);
    printf ("m addr  Dump 128 bytes of memory at 'addr'.\n");
    printf ("r       Show register values and flags.\n");
    printf ("bt      Stack dump (will get backtrace functionality later).\n");
    printf ("q       Quit shadowVIC.\n");
    printf ("h       Print this help text.\n");

    return STAY_IN_DEBUGGER;
}


struct command {
    char * name;
    int (*handler) (char * p);
} commands[] = {
    { "d",  disassembly },
    { "s",  step_instruction },
    { "n",  next_instruction },
    { "u",  execute_until },
    { "g",  execute_from },
    { "r",  dump_registers },
    { "m",  dump_memory },
    { "bt", backtrace },
    { "q",  exit_shadowvic },
    { "h",  print_help },
    { NULL, NULL }
};

int
invoke_command (char * p)
{
    struct command * c = commands;
    size_t l;

    while (c->name) {
        l = strlen (c->name);
        if (strncmp (p, c->name, l)) {
            c++;
            continue;
        }
        return c->handler (skip_whitespace (p + l));
    }

    printf ("Unrecognized command. ");
    return print_help ();
}

int debugger_was_running = FALSE;

void
debugger_welcome_message ()
{
    if (!debugger_was_running) {
        printf (DEBUGGER_WELCOME);
        debugger_was_running = TRUE;
    }
}

void make_prompt (char * prompt)
{
    sprintf (prompt, "%04hx> ", pc);
}

char *
read_command ()
{
    char prompt[256];

    make_prompt (prompt);
    return linenoise (prompt);
}

void
debugger ()
{
    char * line;
    char * p;

    debugger_welcome_message ();
    linenoiseHistoryLoad (HISTORY);
    next_disassembly_address = pc;

    (void) disassemble (stdout, pc, DONT_PRINT_LINEFEED);
    printf ("\t\t");
    (void) dump_registers (NULL);

    while ((line = read_command ()) != NULL) {
        p = skip_whitespace (line);
        if (!*p)
            continue;
        linenoiseHistoryAdd (p);
        linenoiseHistorySave (HISTORY);

        if (invoke_command (p))
            return;
        
        free (line);
    }
}

void
debugger_ctrl ()
{
    if (debugger_break) {
        debugger_break = FALSE;
        debugger ();
    }
    if (debugger_return_address == s) {
        debugger_return_address = -1;
        debugger ();
    }
    if (debugger_until_address == pc) {
        debugger_until_address = -1;
        debugger ();
    }
}

void
user_break (int dummy)
{
    (void) dummy;

    debugger_break = TRUE;
}

void
init_debugger ()
{
    struct sigaction action = {
        .sa_handler = user_break,
        .sa_flags = 0
    };

    mos6502_set_debugger_hook (debugger_ctrl);
    sigaction (SIGINT, &action, NULL);
}
