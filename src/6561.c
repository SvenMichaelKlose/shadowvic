/* shadowVIC – Copyright (c) 2015 Sven Michael Klose <pixel@hugbox.org> */

/*********************************/
/* MOS Technology 6561 emulation */
/*********************************/

#include <stdlib.h>
#include <stdio.h>

#include "types.h"
#include "6502.h"
#include "video.h"
#include "6561.h"

unsigned display_width;
unsigned display_height;
unsigned pixel_width;
unsigned pixel_height;

unsigned screen_columns;
unsigned screen_rows;

#define SCREEN_WIDTH        (screen_columns * 8)
#define SCREEN_HEIGHT       (screen_rows * 8)

#define PHYS_SCREEN_WIDTH   (SCREEN_WIDTH * pixel_width)
#define PHYS_SCREEN_HEIGHT  (SCREEN_HEIGHT * pixel_height)
#define HORIZONTAL_BORDER ((display_height - PHYS_SCREEN_HEIGHT) / 2)
#define VERTICAL_BORDER ((display_width - PHYS_SCREEN_WIDTH) / 2)

#include "palette.c"

address
vic_address (byte bits10to12)
{
    return ((bits10to12 & 7) | ((bits10to12 & 8) ? 0 : 32)) << 10;
}

address
vic_charset ()
{
    return vic_address (m[0x9005] & 15);
}

address
vic_address_bit_9 ()
{
    return  (m[0x9002] & 128) << 2;
}

address
vic_screen ()
{
    return  vic_address (m[0x9005] >> 4) | vic_address_bit_9 ();
}

address
vic_colors ()
{
    return 0x9400 | vic_address_bit_9 ();
}

byte
vic_get (unsigned base, unsigned x, unsigned y)
{
    return m[base + (y * screen_columns) + x];
}

byte vic_charcode (unsigned x, unsigned y)
{
    return vic_get (vic_screen (), x, y);
}

byte vic_color (unsigned x, unsigned y)
{
    return vic_get (vic_colors (), x, y);
}

byte *
vic_char_address (byte x)
{
    return &m[vic_charset () + x * 8];
}

byte background_color;
byte border_color;
byte auxiliary_color;

byte
vic_color_index (byte x, byte c)
{
    byte is_reverse = m[0x900f] & 16;
    switch (x) {
        case 0: return is_reverse ? c : background_color;
        case 1: return border_color;
        case 2: return is_reverse ? background_color : c;
    }
    return auxiliary_color;
}

rgb565_t
vic_color_rgb (byte x, byte c)
{
    return palette[vic_color_index (x, c)];
}

void
vic_draw_char (unsigned x, unsigned y)
{
    rgb565_t c = vic_color (x, y);
    byte m = c & 8;
    byte l;
    unsigned yc;
    unsigned xc;
    int px = x * 8 * pixel_width + VERTICAL_BORDER;
    int py = y * 8 * pixel_height + HORIZONTAL_BORDER;
    byte * p = vic_char_address (vic_charcode (x, y));

    c = c & 7;

    for (yc = 0; yc < 8; yc++) {
        l = *p++;
        if (m)
            for (xc = 0; xc < 4; xc++, l <<= 2)
                draw_rectangle (px + xc * 2 * pixel_width, py + yc * pixel_height, pixel_width * 2, pixel_height, vic_color_rgb ((l & 0xc0) >> 6, c));
        else
            for (xc = 0; xc < 8; xc++, l <<= 1)
                draw_rectangle (px + xc * pixel_width, py + yc * pixel_height, pixel_width, pixel_height, vic_color_rgb (((l & 0x80) >> 7) * 2, c));
    }
}

void
vic_draw_screen ()
{
    unsigned y;
    unsigned x;

    for (y = 0; y < screen_rows; y++)
        for (x = 0; x < screen_columns; x++)
            vic_draw_char (x, y);
}

void
vic_draw_border ()
{
    rgb565_t c = palette[border_color];

    draw_rectangle (0, 0, PHYS_SCREEN_WIDTH + VERTICAL_BORDER * 2, HORIZONTAL_BORDER, c);
    draw_rectangle (0, HORIZONTAL_BORDER + PHYS_SCREEN_HEIGHT, PHYS_SCREEN_WIDTH + VERTICAL_BORDER * 2, HORIZONTAL_BORDER, c);
    draw_rectangle (0, HORIZONTAL_BORDER, VERTICAL_BORDER, PHYS_SCREEN_HEIGHT, c);
    draw_rectangle (VERTICAL_BORDER + PHYS_SCREEN_WIDTH, HORIZONTAL_BORDER, VERTICAL_BORDER, PHYS_SCREEN_HEIGHT, c);
}

void
vic_get_colors ()
{
    background_color = m[0x900f] >> 4;
    border_color = m[0x900f] & 7;
    auxiliary_color = m[0x900e] >> 4;
}

void
vic_set_proportions (unsigned disp_width, unsigned disp_height)
{
    display_width = disp_width;
    display_height = disp_height;
    pixel_height = display_height / SCREEN_HEIGHT;
    pixel_width = pixel_height * 6 / 3;
    if (pixel_width * SCREEN_WIDTH > display_width) {
        pixel_width = display_width / SCREEN_WIDTH;
        pixel_height = pixel_width * 3 / 6;
    }
}

void
vic_video (unsigned disp_width, unsigned disp_height)
{
    screen_columns = m[0x9002] & 127;
    screen_rows = (m[0x9003] >> 1) & 63;
    if (!screen_columns)
        screen_columns = 22;
    if (!screen_rows)
        screen_rows = 23;
    vic_set_proportions (disp_width, disp_height);
    vic_get_colors ();
    vic_draw_border ();
    vic_draw_screen ();
#ifdef DEBUG_VIC
    printf ("VIC screen: %04hx\n", vic_screen ());
    printf ("VIC colors: %04hx\n", vic_colors ());
    printf ("VIC chars:  %04hx\n", vic_charset ());
#endif
}
