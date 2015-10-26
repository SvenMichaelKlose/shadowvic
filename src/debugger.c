/* shadowVIC – Copyright (c) 2015 Sven Michael Klose <pixel@hugbox.org> */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

#include "types.h"
#include "6502.h"

#ifndef HISTORY
#define HISTORY     "shadowvic-debugger.history"
#endif

#ifndef DEBUGGER_WELCOME
#define DEBUGGER_WELCOME "\nWelcome to the shadowVIC debugger.\n"
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

    printf ("Memory dump from $%04hx to $%04hx:\n", from, to);
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
    printf ("m addr  Dump 128 bytes of memory at 'addr'.\n");
    printf ("q       Quit shadowVIC.\n");
    printf ("h       Print this help text.\n");
}

struct command {
    char * name;
    void (*handler) (char * p);
} commands[] = {
    { "m",  dump_memory },
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

void
debugger ()
{
    char * line;
    char * p;

    printf (DEBUGGER_WELCOME);
    linenoiseHistoryLoad (HISTORY);

    while ((line = linenoise ("> ")) != NULL) {
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
