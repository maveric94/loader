#ifndef PRINTF_H
#define PRINTF_H

#include "stdarg.h"
#include "types.h"

int printf (const char *format, ...)
	__attribute__ ((format (printf, 1, 2)));
int vprintf (const char *format, va_list ap);
int snprintf (char *str, size_t size, const char *format, ...)
	__attribute__ ((format (printf, 3, 4)));
int vsnprintf (char *str, size_t size, const char *format, va_list ap);

#endif