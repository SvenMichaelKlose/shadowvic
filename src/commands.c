/* shadowVIC – Copyright (c) 2015 Sven Michael Klose <pixel@hugbox.org> */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "types.h"
#include "commands.h"

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

int
parse_command (struct command * commands, char * p)
{
    size_t l;

    while (commands->name) {
        l = strlen (commands->name);
        if (strncmp (p, commands->name, l)) {
            commands++;
            continue;
        }
        return commands->handler (skip_whitespace (p + l));
    }

    printf ("Unrecognized command. ");
    return COMMAND_NOT_RECOGNIZED;
}
