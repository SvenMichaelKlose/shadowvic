/* shadowVIC – Copyright (c) 2015 Sven Michael Klose <pixel@hugbox.org>
               Copyright (c) 2015 Robert Hurst */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>

#include "types.h"
#include "video.h"
#include "6502.h"
#include "6561.h"
#include "via.h"
#include "joystick.h"
#include "sync.h"
#include "debugger.h"
#include "prg-loader.h"
#include "shadowvic.h"

#include "chargen.c"
#include "kernal.c"
#include "basic.c"

#define SCREEN_UPDATE_INSTRUCTIONS  8000

struct vic20_config * config = NULL;
int do_exit = FALSE;

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
    if (config->frame_interceptor)
        config->frame_interceptor ();
}

void
screen_update ()
{
    vic_video (video_width (), video_height ());
    call_frame_interceptor ();
#ifndef SHADOWVIC_NO_VIDEO_COMMIT
    video_commit ();
#endif
    sync_frame ();
}

#define X_TEST      0
#define X_EXIT      1
#define X_UPDATE    2
#define X_DUMP      3

void
mos6502_jam ()
{
    byte ra;
    byte rx;
    byte ry;
    address from;
    address to;
    byte in = mos6502_fetch_byte ();

    switch (in) {
        case X_TEST:
            puts ((const char *) &m[pc]);
            while ((in = mos6502_fetch_byte ()));
            ra = mos6502_fetch_byte ();
            rx = mos6502_fetch_byte ();
            ry = mos6502_fetch_byte ();
            printf ("result: a: $%02hx x: $%02hx y: $%02hx – wanted: a: $%02hx x: $%02hx y: $%02hx\n", a, x, y, ra, rx, ry);
            if (a !=ra || x != rx || y != ry) {
                printf ("Emulator test result failed.\n");
                vic20_stop ();
            }
            break;
        case X_EXIT:
            printf ("Emulator escape code exit at $%04hx.\n", pc);
            mos6502_fetch_byte ();  /* Use as argument to exit(). */
            vic20_stop ();
        case X_UPDATE:
            screen_update ();
            break;
        case X_DUMP:
            from = mos6502_fetch_word ();
            to = mos6502_fetch_word ();
            printf ("Emulator escape code emory dump from $%04hx to $%04hx:\n", from, to);
            memory_dump (from, to);
            break;
        default:
            printf ("Illegal emulator escape code $%02hx at $%04hx.\n", in, pc);
            vic20_stop ();
    }
}

int recfile;
int playfile;

void
playback_write (int fd, const void * buf, size_t len)
{
    size_t r =  write (fd, buf, len);
    if (r < len)
        printf ("Cannot write to recording, wrote %d bytes instead of %d: %s\n", (int) r, (int) len, strerror (errno));
}

void
playback_read (int fd, void * buf, size_t len)
{
    size_t r = read (fd, buf, len);
    if (r < len)
        printf ("Cannot read from recording, read %d bytes instead of %d %s\n", (int) r, (int) len, strerror (errno));
}

void
record_joystick_status ()
{
    if (config->use_paddles) {
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
    if (config->use_paddles) {
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
    if (config->use_paddles) {
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
#define HALF_FRAME_IS_COMPLETE()   (!(num_instructions % (SCREEN_UPDATE_INSTRUCTIONS / 2)))

void
update_rastercount ()
{
    if (FRAME_IS_COMPLETE())
        m[0x9004] = 0;
    else if (!(num_instructions % 23))
        m[0x9004]++;
}

void
irq ()
{
    if (!mos6502_interrupt_flag ())
        mos6502_interrupt (m[0xfffe] + (m[0xffff] << 8));
}

#define VIA_FLAG_NMI_ENABLED    64
#define NMI_IS_ENABLED()        (enabled_interrupts & VIA_FLAG_NMI_ENABLED)

void
nmi ()
{
    if (NMI_IS_ENABLED())
        mos6502_interrupt (m[0xfffa] + (m[0xfffb] << 8));
}

void
vic20_init (unsigned program_start)
{
    mos6502_reset ();
    pc = program_start;
    num_instructions = 0;
}

void
vic20_step ()
{
    mos6502_emulate ();
    num_instructions++;
    update_rastercount ();
    if (FRAME_IS_COMPLETE()) {
        if (!config->manual_screen_updates)
            screen_update ();
        get_joystick_status ();
        irq ();
    }
    if (HALF_FRAME_IS_COMPLETE())
        nmi ();
}

void
vic20_run ()
{
    while (!do_exit)
        vic20_step ();
}

void
vic20_emulate (unsigned program_start)
{
    vic20_init (program_start);
    vic20_run ();
}

byte booter[] = {
    0x20, 0x8d, 0xfd,  /* jsr ramtas */
    0x20, 0x52, 0xfd,  /* jsr restor */
    0x20, 0xf9, 0xfd,  /* jsr ioinit */
    0x20, 0x18, 0xe5,  /* jsr cint1 */
    0x20, 0x5b, 0xe4,  /* jsr initv */
    0x20, 0xa4, 0xe3,  /* jsr initcz */
    0x22, 0x01         /* return from emulation */
};

byte basic_starter[] = {
    0x20, 0x59, 0xc6,   /* Reset execution to start. clear variables and flush stack. */
    0x20, 0x33, 0xc5,   /* jsr $c533 */
    0x4c, 0xae, 0xc7    /* Goto BASIC. */
};

void
vic20_run_prg (char * pathname)
{
    int i;
    address end_addr;
    struct prg_info pi = {
        .pathname       = pathname,
        .is_relocated   = TRUE
    };

    printf ("Booting BASIC…\n");
    vic20_init (m[0xfffc] + (m[0xfffd] << 8));
    memcpy (&m[0xa000], booter, sizeof (booter));
    pc = 0xa000;
    vic20_run ();
    do_exit = FALSE;

    load_prg (&pi);
    printf ("Returned load address is %04hx.\n", pi.load_addr);
    printf ("Returned size is %04hx.\n", pi.size);
    end_addr = pi.load_addr + pi.size;
    printf ("BASIC end is %04hx.\n", end_addr);
    m[pi.load_addr - 1] = 0;    /* Just to save other people time. */
    m[0x2b] = pi.load_addr & 0xff;
    m[0x2c] = pi.load_addr >> 8;
    m[0x2d] = end_addr & 0xff;
    m[0x2e] = end_addr >> 8;
    memcpy (&m[0xa000], basic_starter, sizeof (basic_starter));
    pc = 0xa000;
    vic20_run ();
}

void
set_default_vic_register_values ()
{
    int i;

    for (i = 0; i < 16; i++)
        m[0x9000 + i] = m[0xede4 + i];
    if (!config->memory_expansion)
        m[0x9002] = m[0x9002] | 128;
}

void
init_vectors ()
{
    /* $eb18 is an register-restoring interrupt return in ROM. */
    m[0x314] = m[0x316] = m[0x318] = 0x18;
    m[0x315] = m[0x317] = m[0x319] = 0xeb;
    /* NMI: $eb1d is an RTI. */
    m[0x318] = 0x1d;
    m[0x319] = 0xeb;
}

void
rom_write (address addr, byte value)
{
    (void) addr;
    (void) value;
}

void
vic20_eject_ram (address addr, size_t size)
{
    int i;

    for (i = 0; i < size; i++)
        writers[addr + i] = rom_write;
}


void
make_rom (address addr, byte * data, size_t size)
{
    vic20_eject_ram (addr, size);
    memcpy (&m[addr], data, size);
}

void
vic20_open (struct vic20_config * cfg)
{
    config = cfg;
    do_exit = FALSE;
    sync_set_fps (config->frames_per_second);
    mos6502_init ();

    bzero (m, 65536);
    make_rom (0x8000, chargen, sizeof (chargen));
    make_rom (0xc000, basic, sizeof (basic));
    make_rom (0xe000, kernal, sizeof (kernal));
    if (!cfg->memory_expansion_3k)
        vic20_eject_ram (0x400, 0xc00);
    if (cfg->memory_expansion < 3)
        vic20_eject_ram (0x6000, 0x2000);
    if (cfg->memory_expansion < 2)
        vic20_eject_ram (0x4000, 0x2000);
    if (cfg->memory_expansion < 1)
        vic20_eject_ram (0x2000, 0x2000);
    init_vectors ();
    set_default_vic_register_values ();
}

void
vic20_close ()
{
}
