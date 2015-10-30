/* shadowVIC – Copyright (c) 2015 Sven Michael Klose <pixel@hugbox.org> */

#ifndef SHADOWVIC_VIC
#define SHADOWVIC_VIC

struct vic20_config {
    int is_expanded;                /* Have full memory? */
    int use_paddles;                /* Paddles instead of joystick? (inpractical) */
    int frames_per_second;          /* PAL 50, NTSC 60 */
    int manual_screen_updates;      /* No automatic screen updates? */
    void (*frame_interceptor) ();   /* Called just before screen is shown. */
};

extern void vic20_open (struct vic20_config *);
extern void vic20_eject_ram (address addr, size_t size);
extern void vic20_record (int file);
extern void vic20_play (int file);
extern void vic20_stop (void);
extern void vic20_emulate (unsigned program_start);
extern void vic20_run_prg (char * path);
extern void vic20_close ();

#endif /* #ifndef SHADOWVIC_VIC */
