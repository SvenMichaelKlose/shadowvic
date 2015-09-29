/* shadowVIC – Copyright (c) 2015 Sven Michael Klose <pixel@hugbox.org> */

#include <stdio.h>

#include "types.h"
#include "config.h"
#include "6502.h"
#include "vic-20.h"
#include "joystick.h"
#include "video.h"
#include "sync.h"

void
open_io ()
{
    joystick_open ();
    video_open ();
    video_map ();
}

void
close_io ()
{
    video_close ();
    joystick_close ();
}

int
main (int argc, char * argv[])
{
    printf ("shadowVIC – https://github.com/SvenMichaelKlose/shadowvic/");

    open_io ();
    vic20_init (FALSE, FALSE);
    vic20_emulate (m[0xfffc] + (m[0xfffd] << 8));
    close_io ();

    return 0;
}
