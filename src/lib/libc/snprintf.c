#include <stdio.h>

int 
snprintf(char * buff, size_t len, const char *format, ...)
{
    va_list ap;

    va_start(ap, format);
    len = vsnprintf(buff, len, format, ap);
    va_end(ap);

    return len;
}
