#include <stdlib.h>

int 
atoi(const char *str)
{
    int val = 0, digit;
    char c;

    while ((c = *str++) != 0) {

        if (c >= '0' && c <= '9')
            digit = (int)(c - '0');
        else
            break;

        val = (val * 10) + digit;
    }

    return val;
}
