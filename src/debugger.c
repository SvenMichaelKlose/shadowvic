/* shadowVIC – Copyright (c) 2015 Sven Michael Klose <pixel@hugbox.org> */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

#ifndef HISTORY
#define HISTORY     "shadowvic-debugger.history"
#endif

void
print_help ()
{
    printf ("Command overview:\n");
    printf ("q       Quit shadowVIC.\n");
    printf ("h       Print this help text.\n");
}

void
debugger ()
{
    char * line;
    char * p;

    printf ("\nWelcome to the shadowVIC debugger.\n");
    linenoiseHistoryLoad (HISTORY);

    while ((line = linenoise ("> ")) != NULL) {
        p = line;

        /* Skip white space. */
        while (*p && *p < ' ')
            p++;

        /* Ignore empty lines. */
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
