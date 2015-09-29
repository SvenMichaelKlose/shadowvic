/* Copyright (c) 2015 Sven Michael Klose <pixel@hugbox.org> */

#define RGB565(r, g, b) (((r / 8) << 11) + ((g / 4) << 5) + (b / 8))

extern byte * pixels;
extern unsigned video_bytes_per_pixel;
typedef unsigned short rgb565_t;

inline void draw_pixel (unsigned x, unsigned y, rgb565_t);
inline void draw_rectangle (unsigned x, unsigned y, unsigned w, unsigned h, rgb565_t);
void draw_char (byte * char_lines, unsigned x, unsigned y, unsigned pixel_width, unsigned pixel_height, rgb565_t fg, rgb565_t bg);
void draw_string (char * s, byte * charset, unsigned x, unsigned y, unsigned pixel_width, unsigned pixel_height, rgb565_t fg, rgb565_t bg);
void draw_string_center (char * s, byte * charset, unsigned y, unsigned w, unsigned h, rgb565_t fg, rgb565_t bg);
void draw_string_center_middle (char * s, byte * charset, unsigned w, unsigned h, rgb565_t fg, rgb565_t bg);
void video_clear ();

extern void     video_open (void);
extern void     video_set_mode (unsigned w, unsigned h, unsigned bpp);
extern byte *   video_map (void);
extern void     video_set_map (byte *);
extern void     video_commit (void);
extern void     video_close (void);
extern unsigned video_size (void);
extern unsigned video_line_length (void);
extern unsigned video_width (void);
extern unsigned video_height (void);
extern unsigned video_bpp (void);
extern void     video_update (void);
