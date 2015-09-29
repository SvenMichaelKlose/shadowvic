/* shadowVIC – Copyright (c) 2015 Sven Michael Klose <pixel@hugbox.org> */

#ifndef SHADOWVIC_VIC
#define SHADOWVIC_VIC

extern void vic20_intercept_frame (void (*f) ());
extern void vic20_init (int is_expanded, int uses_paddle);
extern void vic20_record (int file);
extern void vic20_play (int file);
extern void vic20_stop (void);
extern void vic20_emulate (unsigned program_start);

#endif /* #ifndef SHADOWVIC_VIC */
