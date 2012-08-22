#ifndef _KERNEL_TTY_H_
#define _KERNEL_TTY_H_

#include <sys/cdefs.h>

__BEGIN_DECLS

typedef void (*TTY_INIT)(void);
typedef int (*TTY_SETATTR)(unsigned long, unsigned long);
typedef void (*TTY_MOVE)(unsigned char, unsigned char);
typedef void (*TTY_CLEAR)(void);
typedef void (*TTY_PUTC)(int);

struct stage3_tty_driver
{
    TTY_INIT        init;
    TTY_SETATTR     set_attribute;
    TTY_MOVE        move_cursor;
    TTY_CLEAR       clear;
    TTY_PUTC        putchar;
};


void 
tty_set_handler(struct stage3_tty_driver *handler);

int
tty_set_attribute(unsigned long attribute, unsigned long value);

void
tty_move_cursor(unsigned char x, unsigned char y);

void
tty_clear_screen();

void 
tty_putchar(int c);

__END_DECLS

#endif // _KERNEL_TTY_H_
