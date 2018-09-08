
#include "vga.h"

#include <stdarg.h>

void Vga::scroll_one_line() {
    u16 value = VGA_COLOR(Color::WHITE, Color::BLACK);
    for (s32 i = 1; i < VGA_HEGIHT; ++i) {
        for (u64 x = 0; x < VGA_WIDTH; ++x) {
            u64 index = x + VGA_WIDTH * (i-1);
            buffer[index] = value;
        }
        u16 *dst = &buffer[((i-1) * VGA_WIDTH)];
        u16 *src = &buffer[(i * VGA_WIDTH)];
        memcpy(dst, src, sizeof(u16) * VGA_WIDTH);
    }

    for (u64 x = 0; x < VGA_WIDTH; ++x) {
        u64 index = x + VGA_WIDTH * (VGA_HEGIHT-1);
        buffer[index] = value;
    }

    buffer_cursor_pos_x = 0;
    buffer_cursor_pos_y--;
}

void Vga::print_valist(String fmt, va_list a_list) {
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
                    if (size == 4) write_u32_decimal(va_arg(a_list, u32));
                    // else if (size == 8) write_u64_decimal(va_arg(a_list, u64));
                } else if (c == 'd') {
                    if (size == 4) write_s32_decimal(va_arg(a_list, s32));
                    // else if (size == 8) write_s64_decimal(va_arg(a_list, s64));
                } else if (c == 'S') {
                    String as = va_arg(a_list, String);
                    print(as);
                } else if (c == 's') {
                    char *as = va_arg(a_list, char *);
                    print(temp_string(as));
                } else if (c == 'X') {
                    write_u32_hex(va_arg(a_list, u32), true);
                } else if (c == 'x') {
                    write_u32_hex(va_arg(a_list, u32), false);
                } else if (c == 'p') {
                    void *value = va_arg(a_list, void *);

                    write('0');
                    write('x');
                    if (sizeof(void *) == sizeof(u32)) {
                        write_u32_hex(reinterpret_cast<u32>(value), true);
                    } else if (sizeof(void *) == sizeof(u64)) {
                        write_u64_hex(reinterpret_cast<u64>(value), true);
                    }
                } else {
                    // @TODO
                    write(c);
                }
            } else {
                write(c);
            }
        } else {
            write(c);
        }
    }
}

void Vga::print(String fmt, ...) {
    va_list a_list;
    va_start(a_list, fmt);
    print_valist(fmt, a_list);
    va_end(a_list);
}

void Vga::write_u64_hex(u64 value, bool upper) {
    write_u32_hex((value >> 32) & 0xFFFFFFFF, upper);
    write_u32_hex(value & 0xFFFFFFFF, upper);
}

void Vga::write_u32_hex(u32 value, bool upper) {
    char *hex_values;
    if (upper) hex_values = "0123456789ABCDEF";
    else hex_values       = "0123456789abcdef";

    for (int i = 0; i < 8; ++i) {
        u32 v = (value >> ((7 - i)*4)) & 0xF;
        write(hex_values[v]);
    }
}

void Vga::write_u32_decimal(u32 value) {
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
        write(c);
    }
}

void Vga::write_s32_decimal(s32 value) {
     if (value < 0) {
        write('-');
        value = -value;
    }
    write_u32_decimal(value);
}

void Vga::write(u8 c) {
    if (c == '\n') {
        buffer_cursor_pos_x = 0;
        buffer_cursor_pos_y++;
        
        if (buffer_cursor_pos_y >= VGA_HEGIHT) {
            scroll_one_line();
        }
        return;
    }
    u64 index = buffer_cursor_pos_x + VGA_WIDTH * buffer_cursor_pos_y;
    buffer[index] = color | c;
    
    buffer_cursor_pos_x++;
    
    if (buffer_cursor_pos_x >= VGA_WIDTH) {
        buffer_cursor_pos_x = 0;
        buffer_cursor_pos_y++;
        
        if (buffer_cursor_pos_y >= VGA_HEGIHT) {
            scroll_one_line();
        }
    }
    
    set_cursor_coordinates(buffer_cursor_pos_x, buffer_cursor_pos_y);
}

void Vga::write(String s) {
    for (s64 i = 0; i < s.length; ++i) {
        u8 c = s.data[i];
        write(c);
    }
}

void Vga::clear_screen() {
    u16 value = VGA_COLOR(Color::WHITE, Color::BLACK);
    for (u64 y = 0; y < VGA_HEGIHT; ++y) {
        for (u64 x = 0; x < VGA_WIDTH; ++x) {
            u64 index = x + VGA_WIDTH * y;
            buffer[index] = value;
        }
    }
    
    buffer_cursor_pos_x = 0;
    buffer_cursor_pos_y = 0;
}

void Vga::enable_cursor(bool enable) {
    if (enable) {
        _port_io_write_u8(0x3D4, 0x0A);
        u8 cursor_start = 11;
        u8 cursor_end = 13;
        _port_io_write_u8(0x3D5, (_port_io_read_u8(0x3D5) & 0xC0) | cursor_start);
        
        _port_io_write_u8(0x3D4, 0x0B);
        _port_io_write_u8(0x3D5, (_port_io_read_u8(0x3E0) & 0xE0) | cursor_end);
        
        set_cursor_coordinates(0, 0);
    } else {
        _port_io_write_u8(0x3D4, 0x0A);
        _port_io_write_u8(0x3D5, 0x20);
    }
}

void Vga::set_cursor_coordinates(u16 x, u16 y) {
    u16 pos = y * VGA_WIDTH + x;
    _port_io_write_u8(0x3D4, 0x0F);
    _port_io_write_u8(0x3D5, pos & 0xFF);
    _port_io_write_u8(0x3D4, 0x0E);
    _port_io_write_u8(0x3D5, (pos >> 8) & 0xFF);
}