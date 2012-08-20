#include <stdio.h>

#define PUTCHAR(c) \
    do { \
        if (result < len-1) \
            *str++ = (c);  \
            result++; \
    } while (0)
    
int 
vsnprintf(char *str, size_t len, const char *format, va_list ap)
{
    size_t i, result;

    if (!str || !format || (len < 0))
        return -1;
        
    result = 0;
    for(i = 0; format[i] != 0; i++)
    {
        switch (format[i])
        {
            case '%':
                i++;
                
                switch(format[i])
                {
                    case '%':
                    {
                        PUTCHAR('%');
                        break;
                    }
                    
                    case 'l':
                    {
                        i++;
                        
                        switch (format[i])
                        {
                            case 'i':
                            case 'd':
                            {                       
                                long int integer = va_arg(ap, long int);
                                int cpt2 = 0;
                                char str_int[32];

                                if (integer<0)
                                    PUTCHAR('-');

                                do {
                                    long int m10 = integer%10;
                                    m10 = (m10 < 0)? -m10:m10;
                                    str_int[cpt2++] = (char)('0'+ m10);
                                    integer=integer/10;
                                } while (integer != 0);

                                for(cpt2 = cpt2 - 1 ; cpt2 >= 0 ; cpt2--)
                                    PUTCHAR(str_int[cpt2]);

                                break;
                            }
                            
                            case 'x':
                            {
                                unsigned long int hexa = va_arg(ap, long int);
                                unsigned long int nb;
                                int i;
                                
                                for (i=0 ; i < 16 ; i++)
                                {
                                    nb = (unsigned long int)(hexa << (i*4));
                                    nb = (nb >> 60) & 0xf;

                                    if (nb < 10)
                                        PUTCHAR('0'+nb);
                                    else
                                        PUTCHAR('a'+(nb-10));
                                }
                                
                                break;
                            }
                            
                            default:
                                PUTCHAR('%');
                                PUTCHAR('l');
                                PUTCHAR(format[i]);
                        }
                        
                        break;
                    }
                    
                    case 'i':
                    case 'd':
                    {
                        int integer = va_arg(ap,int);
                        int cpt2 = 0;
                        char str_int[16];

                        if (integer<0)
                            PUTCHAR('-');

                        do {
                            int m10 = integer%10;
                            m10 = (m10 < 0)? -m10:m10;
                            str_int[cpt2++] = (char)('0'+ m10);
                            integer=integer/10;
                        } while (integer != 0);

                        for(cpt2 = cpt2 - 1 ; cpt2 >= 0 ; cpt2--)
                        PUTCHAR(str_int[cpt2]);

                        break;
                    }

                    case 'c':
                    {
                        int value = va_arg(ap, int);
                        PUTCHAR((char)value);
                        
                        break;
                    }

                    case 's':
                    {
                        char *string = va_arg(ap,char *);
                        if (!string)
                            string = "(null)";
                            
                        for( ; *string != '\0' ; string++)
                            PUTCHAR(*string);
                            
                        break;
                    }

                    case 'x':
                    {
                        unsigned int hexa = va_arg(ap,int);
                        unsigned int nb;
                        int i, had_nonzero = 0;
                        
                        for (i=0 ; i < 8 ; i++)
                        {
                            nb = (unsigned int)(hexa << (i*4));
                            nb = (nb >> 28) & 0xf;
                            
                            // Skip the leading zeros
                            if (nb == 0)
                            {
                                if (had_nonzero)
                                    PUTCHAR('0');
                            }
                            else
                            {
                                had_nonzero = 1;
                                if (nb < 10)
                                    PUTCHAR('0'+nb);
                                else
                                    PUTCHAR('a'+(nb-10));
                            }
                        }
                        
                        if (! had_nonzero)
                            PUTCHAR('0');
                            
                        break;
                    }
                    
                    case 'p':
                    {
                        unsigned long int hexa = va_arg(ap, long int);
                        unsigned long int nb;
                        int i;
                        
                        for (i=0 ; i < 16 ; i++)
                        {
                            nb = (unsigned long int)(hexa << (i*4));
                            nb = (nb >> 60) & 0xf;

                            if (nb < 10)
                                PUTCHAR('0'+nb);
                            else
                                PUTCHAR('a'+(nb-10));
                        }
                        
                        break;
                    }

                    default:
                        PUTCHAR('%');
                        PUTCHAR(format[i]);
                } // switch (format[i])
                break; // case '%'

            default:
                PUTCHAR(format[i]);
        } // switch (format[i])
    }

    *str = '\0';
    return result;
}
