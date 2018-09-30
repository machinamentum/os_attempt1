
#include "string.h"
#include "print.h"

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