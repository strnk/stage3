#include <stdio.h>

#define BUFFER_SIZE 256

/* TODO: cleaner version ... :-)
 */
int 
printf(const char *format, ...)
{
    int len;
    char buff[BUFFER_SIZE];
    char *p;
    va_list ap;

    va_start(ap, format);
    len = vsnprintf(buff, BUFFER_SIZE, format, ap);
    va_end(ap);
    
    for (p = buff; *p != 0; p++)
        putchar(*p);

    return len;
}
