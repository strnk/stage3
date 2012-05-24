#ifndef _LIBC_STDARG_H_
#define _LIBC_STDARG_H_

/*  Definitions from GCC stdarg.h
 *
 */
 
#define __GNUC_VA_LIST 

typedef void *__gnuc_va_list;
typedef __gnuc_va_list va_list;

#define __va_rounded_size(TYPE) \
  (((sizeof (TYPE) + sizeof (int) - 1) / sizeof (int)) * sizeof (int))
  
#define va_start(AP, LASTARG) \
  (AP = ((__gnuc_va_list) __builtin_next_arg (LASTARG)))
  
#define va_end(AP) \
  ((void)0)
  
#define va_arg(AP, TYPE) \
  (AP = (__gnuc_va_list) ((char *) (AP) + __va_rounded_size (TYPE)),  \
   *((TYPE *) (void *) ((char *) (AP) - __va_rounded_size (TYPE))))
   
#define __va_copy(dest, src) \
  (dest) = (src)
  
#endif
