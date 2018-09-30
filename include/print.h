
#ifndef PRINT_H
#define PRINT_H

#include "kernel.h"

#include <stdarg.h>

typedef int (*putchar_callback)(void *payload, u8 c);

void print_valist_callback(String fmt, va_list a_list, void *payload, putchar_callback putc_cb);

#endif
