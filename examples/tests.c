/* shadowVIC – Copyright (c) 2015 Sven Michael Klose <pixel@hugbox.org> */

#include <stdio.h>
#include <string.h>

#include "types.h"
#include "config.h"
#include "6502.h"
#include "shadowvic.h"
#include "joystick.h"
#include "video.h"
#include "sync.h"

#include "tests.bin.c"

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

    printf ("shadowVIC 6502 CPU emulation test\n");
    joystick_open ();
    video_open ();
    video_map ();
    vic20_open (&config);
    memcpy (&m[0x1000], tests_image, sizeof (tests_image));
    vic20_emulate (0x1000);
    vic20_close ();
    video_close ();
    joystick_close ();
    printf ("Tests passed.\n");

    return 0;
}
