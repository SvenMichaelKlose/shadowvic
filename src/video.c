/* Copyright (c) 2015 Sven Michael Klose <pixel@hugbox.org> */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "types.h"
#include "video.h"

unsigned video_bytes_per_pixel;

inline void
draw_pixel (unsigned x, unsigned y, rgb565_t color)
{
    unsigned ofs = x * video_bytes_per_pixel + y * video_line_length ();

#ifdef HAVE_32_BPP
    if (video_bytes_per_pixel == 4) {
        * (byte *) &pixels[ofs + 0] = color << 3;
        * (byte *) &pixels[ofs + 1] = color >> 5 << 2;
        * (byte *) &pixels[ofs + 2] = color >> 11 << 3;
        * (byte *) &pixels[ofs + 3] = 255;
    } else
#endif
        * (address *) &pixels[ofs] = color;
}

inline void
draw_rectangle (unsigned x, unsigned y,
                unsigned w, unsigned h,
                rgb565_t color)
{
    unsigned line_length = video_line_length ();
    unsigned ofs = x * video_bytes_per_pixel + y * line_length;
    unsigned xc;
    unsigned yc;

    for (yc = 0; yc < h; yc++) {
        for (xc = 0; xc < w; xc++)
            draw_pixel (x + xc, y + yc, color);
        ofs += line_length;
    }
}

void
draw_char (byte * c,
           unsigned x, unsigned y,
           unsigned w, unsigned h,
           rgb565_t fg, rgb565_t bg)
{
    unsigned xc;
    unsigned yc;
    byte l;

    for (yc = 0; yc < 8; yc++) {
        l = *c++;
        for (xc = 0; xc < 8; xc++, l <<= 1)
            draw_rectangle (x + xc * w, y + yc * h, w, h, l & 0x80 ? fg : bg);
    }
}

void
draw_string (char * s, byte * charset,
             unsigned x, unsigned y,
             unsigned w, unsigned h,
             rgb565_t fg, rgb565_t bg)
{
   char c;

    while (!!(c = *s++)) {
        if (c > 'Z')
            c -= 'a' - 1;
        draw_char (&charset[c * 8], x, y, w, h, fg, bg);
        x += 8 * w;
    }
}

void
draw_string_center (char * s, byte * charset,
                    unsigned y,
                    unsigned w, unsigned h,
                    rgb565_t fg, rgb565_t bg)
{
    draw_string (s, charset, video_width () / 2 - strlen (s) * 4 * w, y, w, h, fg, bg);
}

void
draw_string_center_middle (char * s, byte * charset,
                           unsigned w, unsigned h,
                           rgb565_t fg, rgb565_t bg)
{
    draw_string_center (s, charset, video_height () / 2 - 4 * h, w, h, fg, bg);
}

void
video_clear ()
{
    bzero (pixels, video_size ());
}
