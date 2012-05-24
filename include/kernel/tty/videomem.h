#ifndef _KERNEL_TTY_VIDEOMEM_H_
#define _KERNEL_TTY_VIDEOMEM_H_

#define LC  "\\e["
#define LC_LEN 3
#define RC  'm'

/* iso6429 commands */
#define ISO6429_DEFAULT     0

#define ISO6429_BRIGHT      1
#define ISO6429_UNDERLINE   4
#define ISO6429_FLASH       5

#define ISO6429_BLACK       30
#define ISO6429_RED         31
#define ISO6429_GREEN       32
#define ISO6429_YELLOW      33
#define ISO6429_BLUE        34
#define ISO6429_PURPLE      35
#define ISO6429_CYAN        36
#define ISO6429_GRAY        37
#define ISO6429_GREY        37

#define ISO6429_BG_BLACK    40
#define ISO6429_BG_RED      41
#define ISO6429_BG_GREEN    42
#define ISO6429_BG_YELLOW   43
#define ISO6429_BG_BLUE     44
#define ISO6429_BG_PURPLE   45
#define ISO6429_BG_CYAN     46
#define ISO6429_BG_GRAY     47
#define ISO6429_BG_GREY     47

#define STRCOLOR(x) #x
#define COLOR(x) LC STRCOLOR(x) "m"
#define LCOLOR(x) LC "1;" STRCOLOR(x) "m"

#ifndef _IN_VIDEOMEM
extern struct stage3_tty_driver TTY_VIDEOMEM;
#endif


#endif // _KERNEL_TTY_VIDEOMEM_H_
