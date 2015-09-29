/* Copyright (c) 2015 Sven Michael Klose <pixel@hugbox.org> */

#include <linux/joystick.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "joystick.h"
#include "vic-20.h"

int joystick_num_buttons;
int joystick_num_axis;
char * joystick_buttons;
int * joystick_axis;

int joystick_fd;

void
joystick_open ()
{
    char name[128];

    joystick_fd = open ("/dev/input/js0", O_RDONLY | O_NONBLOCK);
    if (joystick_fd < 0) {
        printf ("No joystick.\n");
        return;
    }
    if (ioctl (joystick_fd, JSIOCGNAME(sizeof (name)), name) < 0)
        strncpy (name, "unknown", sizeof (name));
    ioctl (joystick_fd, JSIOCGBUTTONS, &joystick_num_buttons);
    joystick_buttons = (char *) calloc (joystick_num_buttons, sizeof (char));
    ioctl (joystick_fd, JSIOCGAXES, &joystick_num_axis);
    joystick_axis = (int *) calloc (joystick_num_axis, sizeof (int));
    printf ("Joystick: \"%s\", %d buttons, %d axis\n", name, joystick_num_buttons, joystick_num_axis);
}

void
joystick_close ()
{
    free (joystick_buttons);
    free (joystick_axis);
    close (joystick_fd);
}

void
joystick_reset ()
{
    int i;

    for (i = 0; i < joystick_num_buttons; i++)
        joystick_buttons[i] = 0;
}

void
joystick_update ()
{
    struct js_event e;

    while (read (joystick_fd, &e, sizeof (e)) > 0) {
        if ((e.type & ~JS_EVENT_INIT) == JS_EVENT_BUTTON) {
            if (e.value)
                *joystick_buttons |= (1 << e.number);
            else
                *joystick_buttons &= ~(1 << e.number);
        }
        if ((e.type & ~JS_EVENT_INIT) == JS_EVENT_AXIS)
            joystick_axis[e.number] = e.value;
    }
}

int joystick_left ()
{
    return joystick_axis[0] < 0;
}

int joystick_right ()
{
    return joystick_axis[0] > 0;
}

int joystick_up ()
{
    return joystick_axis[1] < 0;
}

int joystick_down ()
{
    return joystick_axis[1] > 0;
}

int joystick_hposition ()
{
    return joystick_axis[0];
}
