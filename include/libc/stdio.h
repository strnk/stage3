#ifndef _LIBC_STDIO_H_
#define _LIBC_STDIO_H_

#include <sys/cdefs.h>
#include <stdarg.h>
#include <stdlib.h>

__BEGIN_DECLS

int
putchar(int c);

int
puts(const char *str);

int
vsnprintf(char *str, size_t len, const char *format, va_list ap);

int 
snprintf(char * buff, size_t len, const char *format, ...)
  __attribute__ ((format (printf, 3, 4)));
  
int 
printf(const char *format, ...)
  __attribute__ ((format (printf, 1, 2)));

__END_DECLS

#endif // _LIBC_STDIO_H_
