#ifndef STRING_H
#define STRING_H

#include "kernel.h"

struct String;

String temp_string(char *c_string);

struct String {
    u8 *data;
    s64 length;
    
    String(){}
    String(char *str) {
        *this = temp_string(str);
    }
};

void advance(String *s, s64 amount);

struct String_Builder {
    String data;
    s64 allocated;
};

int append(String_Builder *builder, String s);

int string_builder_putchar(void *payload, u8 c);

String sprint(String fmt, ...);

s64 find_char(String *s, u8 needle);

bool strings_match(String a, String b);

#endif

#ifdef STRING_IMPLEMENTATION

#include "print.h"
#include "heap.h"


void advance(String *s, s64 amount) {
    s->data += amount;
    s->length -= amount;
}

int string_builder_putchar(void *payload, u8 c) {
    String_Builder *builder = reinterpret_cast<String_Builder *>(payload);
    
    if (builder->data.length + 1 >= builder->allocated) {
        u8 *new_data = reinterpret_cast<u8 *>(heap_alloc(builder->allocated + 32));
        builder->allocated += 32;
        // @TOOD check for nullptr returned from heap_alloc
        if (builder->data.data) {
            memcpy(new_data, builder->data.data, builder->data.length);
            heap_free(builder->data.data);
        }
        builder->data.data = new_data;
    }
    
    builder->data.data[builder->data.length++] = c;
    return 0; // @TODO error codes
}

String sprint(String fmt, ...) {
    va_list a_list;
    va_start(a_list, fmt);
    String_Builder builder;
    zero_memory(&builder, sizeof(String_Builder));
    print_valist_callback(fmt, a_list, &builder, string_builder_putchar);
    va_end(a_list);
    return builder.data;
}

// @FixMe this doesnt support UTF8 but all Strings should be considered a UTF8 string
s64 find_char(String *s, u8 needle) {
    for (s64 i = 0; i < s->length; ++i) {
        u8 c = s->data[i];
        if (c == needle) return i;
    }
    
    return -1;
}


int append(String_Builder *builder, String s) {
    for (s64 i = 0; i < s.length; ++i) {
        u8 c = s.data[i];
        string_builder_putchar(builder, c);
    }
    
    return 0;
}

bool strings_match(String a, String b) {
    if ((a.data == b.data) && (a.length == b.length)) return true;
    
    if (!a.data || !b.data) return  false; // the case where the length matches is caught above
    
    if (a.length != b.length) return false;
    
    for (s64 i = 0; i < a.length; ++i) {
        if (a.data[i] != b.data[i]) return false;
    }
    
    return true;
}

#endif // STRING_IMPLEMENTATION



