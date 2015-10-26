/* shadowVIC – Copyright (c) 2015 Sven Michael Klose <pixel@hugbox.org> */

#include <stdio.h>
#include <signal.h>

void debugger ()
{
    printf ("Debugger is not yet implemented – exiting with code 255.\n");
    exit (255);
}

void init_debugger ()
{
    signal (SIGINT, debugger);
}
