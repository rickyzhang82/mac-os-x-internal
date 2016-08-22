// nomalloc_printf.h
   
#ifndef _NOMALLOC_PRINTF_H
#define _NOMALLOC_PRINTF_H
   
#include <stdarg.h>
   
extern void _simple_vdprintf(int, const char *, va_list);
   
inline void
nomalloc_printf(const char *format, ...)
{
    va_list ap;
   
    va_start(ap, format);
    _simple_vdprintf(STDOUT_FILENO, format, ap);
    va_end(ap);
}
   
#endif
