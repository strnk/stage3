#ifndef _TTY_VIDEOMEM_H_
#define _TTY_VIDEOMEM_H_

#define COLUMNS     80
#define LINES       25

#define VIDEO_MEM   (void*)0xB8000

#define VIDEOMEM_COLOR_BLACK        0
#define VIDEOMEM_COLOR_BLUE         1
#define VIDEOMEM_COLOR_GREEN        2
#define VIDEOMEM_COLOR_CYAN         3
#define VIDEOMEM_COLOR_RED          4
#define VIDEOMEM_COLOR_MAGENTA      5
#define VIDEOMEM_COLOR_YELLOW       6
#define VIDEOMEM_COLOR_GRAY         7

struct videomem_character {
    char    c;
    char    a;
};

static void
videomem_init();

static int
videomem_set_attribute(unsigned long attribute, unsigned long value);

static void
videomem_move_cursor(unsigned char x, unsigned char y);

static void
videomem_clear_screen();

static void 
videomem_putchar(int c);

static void
videomem_do_putc(int c);

static unsigned char
videomem_color_code_valid(char *state);

#endif // _TTY_VIDEOMEM_H_
