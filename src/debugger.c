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

char *
skip_whitespace (char * p)
{
    while (*p && *p < ' ')
        p++;
    return p;
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
    printf ("q       Quit shadowVIC.\n");
    printf ("h       Print this help text.\n");
}

struct command {
    char * name;
    void (*handler) (char * p);
} commands[] = {
    { "q",  exit_shadowvic },
    { "h",  print_help },
    { NULL, NULL }
};

int
invoke_command (char * p)
{
    struct command * c = commands;

    while (c->name) {
        if (strncmp (p, c->name, strlen (c->name))) {
            c++;
            continue;
        }
        c->handler (p);
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
