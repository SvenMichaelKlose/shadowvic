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


int debugger_break = FALSE;


char *
skip_whitespace (char * p)
{
    while (*p && *p < ' ')
        p++;
    return p;
}

void
memory_dump (address from, address to)
{
    address p = from;
    address l;
    byte c;
    int i;

    while (p < to) {
        printf ("%04hx:", p);
        l = p;
        for (i = 0; i < 16; i++) {
            if (p >= to)
                break;
            if (i == 8)
                printf (" ");
            printf (" %02hx", m[p++]);
        }
        printf ("  ");
        p = l;
        for (i = 0; i < 16; i++) {
            if (p >= to)
                break;
            c = m[p++];
            if (c < 32 || c > 126)
                c = '.';
            putc (c, stdout);
        }
        printf ("\n");
    }
}


address next_disassembly_address = 0;

int
disassembly (char * p)
{
    int i = DISASSEMBLY_LENGTH;
    if (*p)
        next_disassembly_address = strtol (p, NULL, 16);

    while (i--)
        next_disassembly_address = disassemble (stdout, next_disassembly_address);

    return FALSE;
}


int
step_instruction ()
{
    return debugger_break = TRUE;
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
    printf ("Registers: A: %02hx X: %02hx Y: %02hx PC: %04hx ", a, x, y, pc);
    dump_flags ();
    printf ("\n");

    return FALSE;
}


address next_memory_dump_address = 0;

int
dump_memory (char * p)
{
    address from = next_memory_dump_address;
    if (*p)
        from = strtol (p, NULL, 16);

    next_memory_dump_address = from + 128;
    memory_dump (from, next_memory_dump_address);

    return FALSE;
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

    return FALSE;
}


int
exit_shadowvic (char * p)
{
    (void) p;

    printf ("Exiting with code 255 – bye! :)\n");
    exit (255);

    return FALSE;
}


int
print_help ()
{
    printf ("Command overview:\n\n");
    printf ("Ctrl+d  Continue program.\n");
    printf ("d addr  Disassemble %d items at 'addr'.\n", DISASSEMBLY_LENGTH);
    printf ("s       Step a single instruction.\n");
    printf ("r       Show register values and flags.\n");
    printf ("m addr  Dump 128 bytes of memory at 'addr'.\n");
    printf ("bt      Stack dump (will get backtrace functionality later).\n");
    printf ("q       Quit shadowVIC.\n");
    printf ("h       Print this help text.\n");

    return FALSE;
}


struct command {
    char * name;
    int (*handler) (char * p);
} commands[] = {
    { "d",  disassembly },
    { "s",  step_instruction },
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
    sprintf (prompt, "%04hx> ", next_disassembly_address);
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

    (void) disassemble (stdout, pc);

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
    if (!debugger_break)
        return;

    debugger_break = FALSE;
    debugger ();
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
