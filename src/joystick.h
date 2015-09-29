/* Copyright (c) 2015 Sven Michael Klose <pixel@hugbox.org> */

#ifndef JOYSTICK_H
#define JOYSTICK_H

extern int joystick_num_buttons;
extern int joystick_num_axis;
extern char * joystick_buttons;
extern int * joystick_axis;

extern void joystick_open (void);
extern void joystick_update (void);
extern void joystick_close (void);
extern void joystick_reset (void);
extern int joystick_up (void);
extern int joystick_down (void);
extern int joystick_left (void);
extern int joystick_right (void);
extern int joystick_hposition (void);

#endif /* #ifndef JOYSTICK_H */
