#ifndef VGA_H
#define VGA_H

#include "kernel.h"

#include <stdarg.h>

#define VGA_WIDTH 80
#define VGA_HEGIHT 24
#define VGA_COLOR(fg, bg) (((fg) | ((bg) << 4)) << 8)
#define VGA_BLINK_FLAG ((1 << 8) << 8)

#define VGA_ADDRESS ((u16 *)(0xB8000 + KERNEL_VIRTUAL_BASE_ADDRESS))

struct Vga {
    
    // @Volatile these must be in order as the values are tied to the hardware VGA interface!
    enum Color {
        BLACK = 0,
        BLUE,
        GREEN,
        CYAN,
        RED,
        MAGENTA,
        BROWN,
        LIGHT_GREY,
        DARK_GREY,
        LIGHT_BLUE,
        LIGHT_GREEN,
        LIGHT_CYAN,
        LIGHT_RED,
        LIGHT_MAGENTA,
        LIGHT_BROWN,
        WHITE,
    };
    
    u16 *buffer = VGA_ADDRESS;
    u16 color = VGA_COLOR(Color::WHITE, Color::BLACK);
    u32 buffer_cursor_pos_x = 0;
    u32 buffer_cursor_pos_y = 0;
    void write(String s);
    void write_u32(u32 value);
    void write(u8 c);
    void print(String fmt, ...);
    void print_valist(String fmt, va_list a_list);
    void clear_screen();
    
    void enable_cursor(bool enable);
    void set_cursor_coordinates(u16 x, u16 y);
    void scroll_one_line();
};

#endif // VGA_H
