#define _IN_VIDEOMEM
#include <stdlib.h>
#include <kernel/tty.h>
#include <kernel/tty/videomem.h>
#include "videomem.h"

static volatile struct videomem_character *videomem = NULL;
static int videomem_cursor_x = 0;
static int videomem_cursor_y = 0;
int videomem_cursor_attr = 0;

static char videomem_color_code[32];
static unsigned char videomem_color_code_i;

static unsigned char videomem_color_palette[8] =
    {
        VIDEOMEM_COLOR_BLACK,
        VIDEOMEM_COLOR_RED,
        VIDEOMEM_COLOR_GREEN,
        VIDEOMEM_COLOR_YELLOW,
        VIDEOMEM_COLOR_BLUE,
        VIDEOMEM_COLOR_MAGENTA,
        VIDEOMEM_COLOR_CYAN,
        VIDEOMEM_COLOR_GRAY
    } ;

struct stage3_tty_driver TTY_VIDEOMEM =
    {
        videomem_init,
        videomem_set_attribute,
        videomem_move_cursor,
        videomem_clear_screen,
        videomem_putchar
    } ;

static void
videomem_init()
{
    videomem_cursor_attr = (VIDEOMEM_COLOR_BLACK << 4) | VIDEOMEM_COLOR_GRAY;
    videomem_clear_screen();
}

static int
videomem_set_attribute(unsigned long attribute, unsigned long value)
{
    attribute = value;
    value = attribute;
    return 0;
}

static void
videomem_move_cursor(unsigned char x, unsigned char y)
{
    videomem = VIDEO_MEM + (y * COLUMNS * sizeof(struct videomem_character))
                + x * sizeof(struct videomem_character);
    videomem_cursor_x = x;
    videomem_cursor_y = y;
}

static void
videomem_clear_screen()
{
    int i;
    
    videomem_move_cursor(0, 0);
    
    for (i = 0; i < COLUMNS * LINES; i++)
    {
        videomem[i].c = 0;
        videomem[i].a = videomem_cursor_attr;
    }
}

static unsigned char
videomem_color_code_valid(char *state)
{
    static char videomem_color_leftcode[] = LC;
    int i;
    
    for (i = 0; i < videomem_color_code_i && i < LC_LEN; i++)
        if (videomem_color_code[i] 
            != videomem_color_leftcode[i])
            return 0;
    
    if (videomem_color_code_i >= LC_LEN)
    {
        *state = 1;
        videomem_color_code_i  =0;
    }
        
    return 1;
}

static void
videomem_handle_color_code(int code)
{
    if (code >= ISO6429_BG_BLACK && code <= ISO6429_BG_GRAY)
    {
        videomem_cursor_attr &= 0x8F;
        videomem_cursor_attr |=
            videomem_color_palette[code - ISO6429_BG_BLACK] << 4;
    }
    
    if (code >= ISO6429_BLACK && code <= ISO6429_GRAY)
    {
        videomem_cursor_attr &= 0xF8;
        videomem_cursor_attr |=
            videomem_color_palette[code - ISO6429_BLACK];
    }
    
    if (code == ISO6429_DEFAULT)
        videomem_cursor_attr = (VIDEOMEM_COLOR_BLACK << 4) | VIDEOMEM_COLOR_GRAY;
        
    if (code == ISO6429_BRIGHT)
        videomem_cursor_attr |= 0x08;
        
    if (code == ISO6429_FLASH)
        videomem_cursor_attr |= 0x80;
}

void
videomem_scrolldown()
{
    int x, y;
    
    for (y = 0; y < LINES -1; y++)
    {
        for (x = 0; x < COLUMNS; x++)
        {
            videomem[x  + (y) * COLUMNS]
                = videomem[x  + (y+1) * COLUMNS];
        }
    }
    
    for (x = 0; x < COLUMNS; x++)
        videomem[x  + (LINES-1) * COLUMNS].c = 0;
}

static void 
videomem_putchar(int c)
{
    static char in_color_code = 0;
    
    // Inside an ISO6429 color code
    if (in_color_code)
    {      
        // End of code
        if (c == RC)
        {
            videomem_color_code[videomem_color_code_i] = 0;
            videomem_handle_color_code(atoi(videomem_color_code));
            in_color_code = 0;
            videomem_color_code_i = 0;
        }
        else if (c == ';')
        {
            videomem_color_code[videomem_color_code_i] = 0;
            videomem_handle_color_code(atoi(videomem_color_code));
            videomem_color_code_i = 0;
        }
        else if ((c >= '0') && (c <= '9'))
            videomem_color_code[videomem_color_code_i++] = c;
        
        return;
    }
    
    // Classic handling of characters
    switch (c)
    {            
        case '\n':
            videomem_cursor_x = 0;
            videomem_cursor_y++;
            
            
            if (videomem_cursor_y >= LINES)
            {
                videomem_scrolldown();
                videomem_cursor_y--;
            }
            break;
        
        case '\r':
            videomem_cursor_x = 0;
            break;
            
            
        case '\\':
        case 'e':
        case '[':
        case 'm':
            videomem_color_code[videomem_color_code_i++] = c;
            
            if (!videomem_color_code_valid(&in_color_code))
            {
                int i;
                
                for (i = 0; i < videomem_color_code_i; i++)
                    videomem_do_putc(videomem_color_code[i]);
                    
                in_color_code = 0;
                videomem_color_code_i = 0;
            }
            break;
            
        default:
            videomem_do_putc(c);
    }
}


static void
videomem_do_putc(int c)
{
    videomem[videomem_cursor_x  + videomem_cursor_y * COLUMNS].c 
        = c & 0xFF;
        
    videomem[videomem_cursor_x  + videomem_cursor_y * COLUMNS].a
        = videomem_cursor_attr;
        
    videomem_cursor_x++;
    
    if (videomem_cursor_x >= COLUMNS)
    {
        videomem_cursor_x = 0;
        videomem_cursor_y++;
    }
    
    if (videomem_cursor_y >= LINES)
    {
        videomem_scrolldown();
        videomem_cursor_y--;
    }
}
