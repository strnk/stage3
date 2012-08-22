#ifndef _LIBC_STDLIB_H_
#define _LIBC_STDLIB_H_

#include <stddef.h>

#ifdef __cplusplus
#define NULL 0
#else
#define NULL (void*)0
#endif

int
atoi(const char *str);

#endif // _LIBC_STDLIB_H_
