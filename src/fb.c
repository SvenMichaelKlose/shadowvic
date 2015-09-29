/* shadowVIC – Copyright (c) 2015 Sven Michael Klose <pixel@hugbox.org> */

#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include "types.h"
#include "video.h"

byte * original_pixels;
byte * pixels;

struct fb_fix_screeninfo finfo;
struct fb_var_screeninfo vinfo;
struct fb_var_screeninfo oinfo;
int fbfd = 0;

void
fb_error (char * what)
{
    printf ("Can't %s.\n", what);
    exit (1);
}

void
fbi_error (char * what)
{
    printf ("Cannot %s framebuffer info.\n", what);
    exit (1);
}

void
fb_read_fixed (void)
{
    if (ioctl (fbfd, FBIOGET_FSCREENINFO, &finfo))
        fbi_error ("read fixed");
    video_bytes_per_pixel = vinfo.bits_per_pixel / 8;
    printf ("Display: %dx%d pixels, %d bpp\n",
            vinfo.xres, vinfo.yres, vinfo.bits_per_pixel);
}

void
video_open ()
{
    fbfd = open ("/dev/fb0", O_RDWR);
    if (fbfd == -1)
        fb_error ("open framebuffer device");

    if (ioctl (fbfd, FBIOGET_VSCREENINFO, &vinfo))
        fbi_error ("read variable");
    memcpy (&oinfo, &vinfo, sizeof (struct fb_var_screeninfo));

    fb_read_fixed ();
}

void
video_set_mode (unsigned w, unsigned h, unsigned bpp)
{
    vinfo.bits_per_pixel = bpp;
    vinfo.grayscale = 0;
    vinfo.xres = vinfo.xres_virtual = w;
    vinfo.yres = vinfo.yres_virtual = h;
    if (ioctl (fbfd, FBIOPUT_VSCREENINFO, &vinfo))
        fbi_error ("set variable");
    fb_read_fixed ();
}

unsigned
video_size ()
{
    return finfo.smem_len;
}

byte *
video_map ()
{
    original_pixels = (byte *) mmap (0, video_size (), PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);
    if (original_pixels == (byte *) -1)
        printf ("mmap() failed.\n");
    pixels = (byte *) malloc (video_size ());
    return pixels;
}

void
video_set_map (byte * new_pixels)
{
    pixels = new_pixels;
}

void
video_commit ()
{
    int arg = 9;
    ioctl (fbfd, FBIO_WAITFORVSYNC, &arg);
    memcpy (original_pixels, pixels, video_size ());
}

void
video_close ()
{
    munmap (original_pixels, video_size ());
    free (pixels);
    if (ioctl (fbfd, FBIOPUT_VSCREENINFO, &oinfo))
        fbi_error ("restore variable");
    close (fbfd);
}

unsigned
video_line_length ()
{
    return finfo.line_length;
}

unsigned
video_width ()
{
    return vinfo.xres;
}

unsigned
video_height ()
{
    return vinfo.yres;
}

unsigned
video_bpp ()
{
    return vinfo.bits_per_pixel;
}

void video_update () {}
