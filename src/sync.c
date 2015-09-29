/* shadowVIC – Copyright (c) 2015 Sven Michael Klose <pixel@hugbox.org> */

#include <sys/time.h>
#include <time.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "types.h"
#include "config.h"
#include "sync.h"

#define FRAME_INTERVAL(fps)  (1000000 / fps)

int frames_per_second;
struct timeval last_frame_time;

void
sync_set_fps (int fps)
{
    frames_per_second = fps;
}

void
sync_frame (void)
{
    struct timespec pause = {0, 0};
    struct timeval current_time;
    int diff;

    if (gettimeofday (&current_time, NULL))
        printf ("Cannot get current time: %s\n", strerror (errno));
    diff = current_time.tv_usec - last_frame_time.tv_usec;
    if (diff > 0 && diff < FRAME_INTERVAL(frames_per_second)) {
        pause.tv_nsec = (FRAME_INTERVAL(frames_per_second) - diff) * 1000;
        nanosleep (&pause, NULL);
    }
    if (gettimeofday (&last_frame_time, NULL))
        printf ("Cannot get frame time: %s\n", strerror (errno));
}
