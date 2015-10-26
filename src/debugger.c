/* shadowVIC – Copyright (c) 2015 Sven Michael Klose <pixel@hugbox.org> */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

#include "types.h"
#include "6502.h"
#include "disassembler.h"
#include "debugger.h"

#ifndef HISTORY
#define HISTORY     "shadowvic-debugger.history"
#endif

#ifndef DEBUGGER_WELCOME
#define DEBUGGER_WELCOME "\nWelcome to the shadowVIC debugger.\n"
#endif

#ifndef DISASSEMBLY_LENGTH
#define DISASSEMBLY_LENGTH  16
#endif

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

void
disassembly (char * p)
{
    int i = DISASSEMBLY_LENGTH;
    address from = next_disassembly_address;
    if (*p)
        from = strtol (p, NULL, 16);

    while (i--)
        next_disassembly_address = disassemble (stdout, next_disassembly_address);
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

void
dump_registers (char * p)
{
    printf ("Registers: A: %02hx X: %02hx Y: %02hx PC: %04hx ", a, x, y, pc);
    dump_flags ();
    printf ("\n");
}


address next_memory_dump_address = 0;

void
dump_memory (char * p)
{
    address from = next_memory_dump_address;
    if (*p)
        from = strtol (p, NULL, 16);

    next_memory_dump_address = from + 128;
    memory_dump (from, next_memory_dump_address);
}


void
backtrace (char * p)
{
    address sp = s;
    (void) p;

    printf ("Stack: ");
    while (sp < 0xff)
        printf (" $%02hx", m[++sp + 0x100]);
}


void
exit_shadowvic (char * p)
{
    (void) p;

    printf ("Exiting with code 255 – bye! :)\n");
    exit (255);
}


void
print_help ()
{
    printf ("Command overview:\n");
    printf ("d addr  Disassemble %d items at 'addr'.\n", DISASSEMBLY_LENGTH);
    printf ("r       Show register values and flags.\n");
    printf ("m addr  Dump 128 bytes of memory at 'addr'.\n");
    printf ("bt      Stack dump (will get backtrace functionality later).\n");
    printf ("q       Quit shadowVIC.\n");
    printf ("h       Print this help text.\n");
}


struct command {
    char * name;
    void (*handler) (char * p);
} commands[] = {
    { "d",  disassembly },
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
        c->handler (skip_whitespace (p + l));
        return -1;
    }
    return 0;
}

int debugger_was_running = FALSE;

void
debugger_welcome_message ()
{
    if (!debugger_was_running) {
        printf (DEBUGGER_WELCOME);
        debugger_was_running = TRUE;
    } else
        printf ("\n");
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

    while ((line = read_command ()) != NULL) {
        p = skip_whitespace (line);
        if (!*p)
            continue;
        linenoiseHistoryAdd (p);
        linenoiseHistorySave (HISTORY);

        if (!invoke_command (p)) {
            printf ("Unrecognized command. ");
            print_help ();
        }
        
        free (line);
    }
}

void
init_debugger ()
{
    signal (SIGINT, debugger);
}
