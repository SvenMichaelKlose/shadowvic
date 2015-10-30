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

void
open_picovic ()
{
    struct vic20_config config = {
        .is_expanded = FALSE,
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
    printf (" +32   Add RAM from 2000-7fff and from a000-bfff.:\n");
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

int do_add_3k = FALSE;
int do_add_8k = FALSE;
int do_add_16k = FALSE;
int do_add_24k = FALSE;
int do_add_32k = FALSE;

int argument_errors = FALSE;

void
error_multiple_expansion (char * which)
{
    printf ("+%sK expansion has already been specified.\n", which);
    argument_errors = TRUE;
}

void
check_other_expansions ()
{
    if (do_add_8k)
        error_multiple_expansion ("8");
    if (do_add_16k)
        error_multiple_expansion ("16");
    if (do_add_24k)
        error_multiple_expansion ("24");
    if (do_add_32k)
        error_multiple_expansion ("32");
}

int add_3k (char * p) { do_add_3k = TRUE; return 0; }
int add_8k (char * p) { check_other_expansions (); do_add_8k = TRUE; return 0; }
int add_16k (char * p) { check_other_expansions (); do_add_16k = TRUE; return 0; }
int add_24k (char * p) { check_other_expansions (); do_add_24k = TRUE; return 0; }
int add_32k (char * p) { check_other_expansions (); do_add_32k = TRUE; return 0; }

struct command argument_commands[] = {
    { "-h", print_picovic_help },
    { "+3", add_3k },
    { "+8", add_8k },
    { "+16", add_16k },
    { "+24", add_24k },
    { "+32", add_32k },
    { NULL, NULL }
};

void
configure_vic ()
{
    if (argument_errors) {
        close_picovic ();
        exit (255);
    }
    if (!do_add_3k)
        vic20_eject_ram (0x0400, 0x0c00);
    if (do_add_8k) {
        vic20_eject_ram (0x4000, 0x4000);
        return;
    } else if (do_add_16k) {
        vic20_eject_ram (0x6000, 0x2000);
        return;
    } else if (do_add_24k) {
        vic20_eject_ram (0xa000, 0x2000);
        return;
    }

    vic20_eject_ram (0x2000, 0x6000);
}

void
exec_arguments (int argc, char * argv[])
{
    int i;
    for (i = 1; i < argc; i++) {
        printf ("Executing argument '%s'…\n", argv[i]);
        if (parse_command (argument_commands, argv[i]) == COMMAND_NOT_RECOGNIZED) {
            printf ("Booting from ROM and starting program '%s'…\n", argv[i]);
            configure_vic ();
            vic20_run_prg (argv[i]);
            return;
        }
    }
    printf ("Booting from ROM…\n");
    configure_vic ();
    vic20_emulate (m[0xfffc] + (m[0xfffd] << 8));
}

int
main (int argc, char * argv[])
{

    printf ("picoVIC – https://github.com/SvenMichaelKlose/shadowvic/\n");

    open_picovic ();
    exec_arguments (argc, argv);
    close_picovic ();

    return 0;
}
