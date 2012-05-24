#include <stdlib.h>
#include <kernel/tty.h>

struct stage3_tty_driver *tty_handler = NULL;
 
void tty_set_handler(struct stage3_tty_driver *handler)
{
    tty_handler = handler;
    tty_handler->init();
}

int
tty_set_attribute(unsigned long attribute, unsigned long value)
{
    return tty_handler->set_attribute(attribute, value);
}

void
tty_move_cursor(unsigned char x, unsigned char y)
{
    tty_handler->move_cursor(x, y);
}

void
tty_clear_screen()
{
    tty_handler->clear();
}

void 
tty_putchar(int c)
{
    tty_handler->putchar(c);
}
