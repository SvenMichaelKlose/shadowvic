/* shadowVIC – Copyright (c) 2015 Sven Michael Klose <pixel@hugbox.org> */

#include <stdlib.h>
#include <stdio.h>

#include "types.h"
#include "config.h"
#include "6502.h"
#include "shadowvic.h"
#include "joystick.h"
#include "video.h"
#include "sync.h"
#include "debugger.h"
#include "commands.h"

int have_memory_expansion_3k = FALSE;
int have_memory_expansion = 0;
int argument_errors = FALSE;

void
open_picovic ()
{
    struct vic20_config config = {
        .memory_expansion = have_memory_expansion,
        .memory_expansion_3k = have_memory_expansion_3k,
        .use_paddles = FALSE,
        .manual_screen_updates = FALSE,
        .frames_per_second = 50,
        .frame_interceptor = NULL
    };

    joystick_open ();
    video_open ();
    video_map ();
    vic20_open (&config);
    init_debugger ();
}

void
close_picovic ()
{
    vic20_close ();
    video_close ();
    joystick_close ();
}

int
print_picovic_help (char * p)
{
    (void) p;

    printf ("Usage: picovic [+3] [+8|+16|+24|+32] [PRG file]\n");
    printf ("\n");
    printf (" +3    Add RAM from 0400-0fff.:\n");
    printf ("\n");
    printf ("These arguments cannot be combined:\n");
    printf (" +8    Add RAM from 2000-3fff.:\n");
    printf (" +16   Add RAM from 2000-5fff.:\n");
    printf (" +24   Add RAM from 2000-7fff.:\n");
    printf ("\n");
    printf ("Unrecognized strings are interpreted as the path name\n");
    printf ("of a program which is started after BASIC has been\n");
    printf ("booted.\n");
    printf ("\n");
    printf ("After program start Ctrl+C launches the debugger.\n");
    printf ("Enter 'h' in it to get more help there.\n");

    close_picovic ();
    exit (0);

    return 0;
}

void
check_other_expansions ()
{
    if (!have_memory_expansion)
        return;
    printf ("Memory expansion has already been specified.\n");
    argument_errors = TRUE;
}

int add_3k (char * p) { have_memory_expansion_3k = TRUE; return 0; }
int add_8k (char * p) { check_other_expansions (); have_memory_expansion = 1; return 0; }
int add_16k (char * p) { check_other_expansions (); have_memory_expansion = 2; return 0; }
int add_24k (char * p) { check_other_expansions (); have_memory_expansion = 3; return 0; }

struct command argument_commands[] = {
    { "-h", print_picovic_help },
    { "+3", add_3k },
    { "+8", add_8k },
    { "+16", add_16k },
    { "+24", add_24k },
    { NULL, NULL }
};

void
exit_on_argument_errors ()
{
    if (!argument_errors)
        return;
    close_picovic ();
    exit (255);
}

void
exec_arguments (int argc, char * argv[])
{
    int i;
    for (i = 1; i < argc; i++) {
        printf ("Executing argument '%s'…\n", argv[i]);
        if (parse_command (argument_commands, argv[i]) == COMMAND_NOT_RECOGNIZED) {
            printf ("Booting from ROM and starting program '%s'…\n", argv[i]);
            exit_on_argument_errors ();
            open_picovic ();
            vic20_run_prg (argv[i]);
            return;
        }
    }
    printf ("Booting from ROM…\n");
    exit_on_argument_errors ();
    open_picovic ();
    vic20_emulate (m[0xfffc] + (m[0xfffd] << 8));
}

int
main (int argc, char * argv[])
{

    printf ("picoVIC – https://github.com/SvenMichaelKlose/shadowvic/\n");

    exec_arguments (argc, argv);
    close_picovic ();

    return 0;
}
