/* shadowVIC – Copyright (c) 2015 Sven Michael Klose <pixel@hugbox.org> */

#include <stdio.h>

#include "types.h"
#include "config.h"
#include "6502.h"
#include "shadowvic.h"
#include "joystick.h"
#include "video.h"
#include "sync.h"
#include "debugger.h"

#define FALSE   0
#define TRUE    1

int
main (int argc, char * argv[])
{
    struct vic20_config config = {
        .is_expanded = FALSE,
        .use_paddles = FALSE,
        .manual_screen_updates = FALSE,
        .frames_per_second = 50,
        .frame_interceptor = NULL
    };

    printf ("picoVIC – https://github.com/SvenMichaelKlose/shadowvic/\n");

    joystick_open ();
    video_open ();
    video_map ();
    vic20_open (&config);
    init_debugger ();
    vic20_emulate (m[0xfffc] + (m[0xfffd] << 8));
    vic20_close ();
    video_close ();
    joystick_close ();

    return 0;
}
