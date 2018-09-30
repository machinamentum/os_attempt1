#ifndef STRING_H
#define STRING_H

#include "kernel.h"


void advance(String *s, s64 amount);

struct String_Builder {
    String data;
    s64 allocated;
};

int append(String_Builder *builder, String s);

int string_builder_putchar(void *payload, u8 c);

String sprint(String fmt, ...);

s64 find_char(String *s, u8 needle);

#endif


