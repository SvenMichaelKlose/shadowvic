/* shadowVIC – Copyright (c) 2015 Sven Michael Klose <pixel@hugbox.org> */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>

#include "types.h"
#include "config.h"
#include "video.h"
#include "6502.h"
#include "6561.h"
#include "joystick.h"
#include "sync.h"
#include "vic-20.h"

#include "chargen.c"
#include "kernal.c"
#include "basic.c"

int do_exit = FALSE;
int do_use_paddle = FALSE;

void
vic20_stop ()
{
    do_exit = TRUE;
}

void (*frame_interception) (void);

void
vic20_intercept_frame (void (*f) ())
{
    frame_interception = f;
}

void
call_frame_interceptor ()
{
    if (frame_interception)
        frame_interception ();
}

void
screen_update ()
{
    vic_video (video_width (), video_height ());
    call_frame_interceptor ();
    video_commit ();
    sync_frame ();
}

#define X_TEST      0
#define X_EXIT      1
#define X_UPDATE    2

void
mos6502_jam ()
{
    byte ra;
    byte rx;
    byte ry;
    byte in = mos6502_fetch_byte ();

    switch (in) {
        case X_TEST:
            puts ((const char *) &m[pc]);
            while ((in = mos6502_fetch_byte ()));
            ra = mos6502_fetch_byte ();
            rx = mos6502_fetch_byte ();
            ry = mos6502_fetch_byte ();
            printf ("a: %d x: %d y: %d – a: %d x: %d y: %d\n", a, x, y, ra, rx, ry);
            if (a !=ra || x != rx || y != ry) {
                printf ("Emulator test failed.\n");
                vic20_stop ();
            }
            break;
        case X_EXIT:
            printf ("Emulator escape code exit at %d.\n", pc);
            mos6502_fetch_byte ();  /* Use as argument to exit(). */
            vic20_stop ();
        case X_UPDATE:
            screen_update ();
            break;
        default:
            printf ("Illegal emulator escape code %d at %d.\n", in, pc);
            vic20_stop ();
    }
}

int recfile;
int playfile;

void
playback_write (int fd, const void * buf, size_t len)
{
    int r =  write (fd, buf, len);
    if (r < 1)
        printf ("Cannot write to recording.\n");
}

void
playback_read (int fd, void * buf, size_t len)
{
    int r = read (fd, buf, len);
    if (r < 1)
        printf ("Cannot read from recording.\n");
}

void
record_joystick_status ()
{
    if (do_use_paddle) {
        playback_write (recfile, &m[0x9111], 1);
        playback_write (recfile, &m[0x9008], 1);
    } else {
        playback_write (recfile, &m[0x9111], 1);
        playback_write (recfile, &m[0x9120], 1);
    }
}

void
play_joystick_status ()
{
    if (do_use_paddle) {
        playback_read (playfile, &m[0x9111], 1);
        playback_read (playfile, &m[0x9008], 1);
    } else {
        playback_read (playfile, &m[0x9111], 1);
        playback_read (playfile, &m[0x9120], 1);
    }
}

void
vic20_record (int file)
{
    recfile = file;
}

void
vic20_play (int file)
{
    playfile = file;
}

void
emulate_joystick_via ()
{
    joystick_update ();
    if (do_use_paddle) {
        m[0x9111] = *joystick_buttons ? 0 : 16;
        m[0x9008] = 255 - ((joystick_hposition () + 32768) >> 8);
    } else {
        m[0x9111] = (*joystick_buttons ? 0 : 32) +
                    (joystick_left () ? 0 : 16) +
                    (joystick_down () ? 0 : 8) +
                    (joystick_up () ? 0 : 4);
        m[0x9120] = (joystick_right () ? 0 : 128);
    }
}

void
get_joystick_status ()
{
    if (playfile)
        play_joystick_status ();
    else
        emulate_joystick_via ();
    if (recfile)
        record_joystick_status ();
}

unsigned num_instructions = 0;

#define FRAME_IS_COMPLETE()   (!(num_instructions % SCREEN_UPDATE_INSTRUCTIONS))

void
update_rastercount ()
{
    if (FRAME_IS_COMPLETE())
        m[0x9004] = 0;
    else if (!(num_instructions % 23))
        m[0x9004]++;
}

void
rom_interrupt ()
{
    if (mos6502_interrupt_flag ())
        return;

    mos6502_interrupt (m[0xfffe] + (m[0xffff] << 8));
    mos6502_interrupt (m[0xfffa] + (m[0xfffb] << 8));
}

void
vic20_emulate (unsigned program_start)
{
    mos6502_reset ();
    pc = program_start;
    num_instructions = 0;

    while (!do_exit) {
        mos6502_emulate ();
        num_instructions++;
        update_rastercount ();
        if (FRAME_IS_COMPLETE()) {
#ifndef MANUAL_SCREEN_UPDATE
            screen_update ();
#endif
            get_joystick_status ();
            rom_interrupt ();
        }
    }
}

void
set_default_vic_register_values (int is_expanded)
{
    int i;

    for (i = 0; i < 16; i++)
        m[0x9000 + i] = m[0xede4 + i];
    if (!is_expanded)
        m[0x9002] = m[0x9002] | 128;
}

void
vic20_init (int is_expanded, int uses_paddle)
{
    do_exit = FALSE;
    do_use_paddle = uses_paddle;
    sync_set_fps (FRAMES_PER_SECOND);
    bzero (m, 65536);

    memcpy (&m[0x8000], &chargen, sizeof (chargen));
    memcpy (&m[0xc000], &basic, sizeof (basic));
    memcpy (&m[0xe000], &kernal, sizeof (kernal));
    set_default_vic_register_values (is_expanded);
}
