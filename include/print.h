
#ifndef PRINT_H
#define PRINT_H

#include "kernel.h"

#include <stdarg.h>

typedef int (*putchar_callback)(void *payload, u8 c);

void print_valist_callback(String fmt, va_list a_list, void *payload, putchar_callback putc_cb);

#endif

#ifdef PRINT_IMPLEMENTATION

void write_u32_hex(u32 value, bool upper, void *payload, putchar_callback putc_cb) {
    char *hex_values;
    if (upper) hex_values = "0123456789ABCDEF";
    else hex_values       = "0123456789abcdef";
    
    for (int i = 0; i < 8; ++i) {
        u32 v = (value >> ((7 - i)*4)) & 0xF;
        putc_cb(payload, hex_values[v]);
    }
}

void write_u64_hex(u64 value, bool upper, void *payload, putchar_callback putc_cb) {
    write_u32_hex((value >> 32) & 0xFFFFFFFF, upper, payload, putc_cb);
    write_u32_hex(value & 0xFFFFFFFF, upper, payload, putc_cb);
}

void write_u32_decimal(u32 value, void *payload, putchar_callback putc_cb) {
    char *dec_values = "0123456789";
    char buffer[11];
    zero_memory(&buffer, sizeof(buffer));
    u8 index = 0;
    
    do {
        buffer[index++] = dec_values[value % 10];
        value /= 10;
    } while (value > 0);
    
    for (int i = 10; i >= 0; --i) {
        char c = buffer[i];
        if (c == 0) continue;
        putc_cb(payload, c);
    }
}

void write_s32_decimal(s32 value, void *payload, putchar_callback putc_cb) {
    if (value < 0) {
        putc_cb(payload, '-');
        value = -value;
    }
    write_u32_decimal(value, payload, putc_cb);
}

void write_string(String str, void *payload, putchar_callback putc_cb) {
    for (s64 i = 0; i < str.length; ++i) {
        u8 c = str.data[i];
        putc_cb(payload, c);
    }
}

void print_valist_callback(String fmt, va_list a_list, void *payload, putchar_callback putc_cb) {
    for (s64 i = 0; i < fmt.length; ++i) {
        u8 c = fmt.data[i];
        
        if (c == '%') {
            if (i < fmt.length-1) {
                ++i;
                c = fmt.data[i];
                
                int size = 4;
                // if (c == 'l') {
                //     size = 8;
                
                //     if (i < fmt.length-1) {
                //         ++i;
                //         c = fmt.data[i];
                //     }
                // }
                
                if (c == 'u') {
                    if (size == 4) write_u32_decimal(va_arg(a_list, u32), payload, putc_cb);
                    // else if (size == 8) write_u64_decimal(va_arg(a_list, u64));
                } else if (c == 'd') {
                    if (size == 4) write_s32_decimal(va_arg(a_list, s32), payload, putc_cb);
                    // else if (size == 8) write_s64_decimal(va_arg(a_list, s64));
                } else if (c == 'S') {
                    String as = va_arg(a_list, String);
                    write_string(as, payload, putc_cb);
                } else if (c == 's') {
                    char *as = va_arg(a_list, char *);
                    write_string(temp_string(as), payload, putc_cb);
                } else if (c == 'X') {
                    write_u32_hex(va_arg(a_list, u32), true, payload, putc_cb);
                } else if (c == 'x') {
                    write_u32_hex(va_arg(a_list, u32), false, payload, putc_cb);
                } else if (c == 'p') {
                    void *value = va_arg(a_list, void *);
                    
                    putc_cb(payload, '0');
                    putc_cb(payload, 'x');
                    if (sizeof(void *) == sizeof(u32)) {
                        write_u32_hex(reinterpret_cast<u32>(value), true, payload, putc_cb);
                    } else if (sizeof(void *) == sizeof(u64)) {
                        write_u64_hex(reinterpret_cast<u64>(value), true, payload, putc_cb);
                    }
                } else if (c == 'c') {
                    u32 value = va_arg(a_list, u32);
                    putc_cb(payload, static_cast<u8>(value));
                } else if (c == 'f') {
                    float64 value = va_arg(a_list, float64);
                    // write_float64(value, payload, putc_cb);
                } else {
                    // @TODO
                    putc_cb(payload, c);
                }
            } else {
                putc_cb(payload, c);
            }
        } else {
            putc_cb(payload, c);
        }
    }
}


#endif // PRINT_IMPLEMENTATION