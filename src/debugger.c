/* shadowVIC – Copyright (c) 2015 Sven Michael Klose <pixel@hugbox.org> */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

#ifndef HISTORY
#define HISTORY     "shadowvic-debugger.history"
#endif

#ifndef DEBUGGER_WELCOME
#define DEBUGGER_WELCOME "\nWelcome to the shadowVIC debugger.\n"
#endif

void
print_help ()
{
    printf ("Command overview:\n");
    printf ("q       Quit shadowVIC.\n");
    printf ("h       Print this help text.\n");
}

char *
skip_whitespace (char * p)
{
    while (*p && *p < ' ')
        p++;
    return p;
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

        if (!strcmp (p, "q")) {
            printf ("Bye! :)\n");
            exit (255);
        } else if (!strcmp (p, "h")) {
            print_help ();
        } else {
            printf ("Unrecognized commend. ");
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
