#include <stdint.h>

typedef int64_t s64;
typedef int32_t s32;
typedef int16_t s16;
typedef int8_t  s8;

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t  u8;

typedef float float32;
typedef double float64;

struct Multiboot_Information {
    u32 flags;
    u32 mem_lower;
    u32 mem_upper;
    u32 boot_device;
    u32 cmdline;

};

extern "C"
void _port_io_write_u32(u16 port, u32 value);
extern "C"
void _port_io_write_u8(u16 port, u8 value);

extern "C"
u8 _port_io_read_u8(u16 port);

struct String {
    u8 *data;
    s64 length;
};

s64 strlen(char *c_string) {
    if (!c_string) return 0;
    
    char *s = c_string;
    while (*s) ++s;
    
    return s - c_string;
}

String temp_string(char *data, u64 length) {
    String s;
    s.data = reinterpret_cast<u8 *>(data);
    s.length = length;
    return s;
}

String temp_string(char *c_string) {
    return temp_string(c_string, strlen(c_string));
}

#define VGA_WIDTH 80
#define VGA_HEGIHT 25
#define VGA_COLOR(fg, bg) (((fg) | ((bg) << 4)) << 8)
#define VGA_BLINK_FLAG ((1 << 8) << 8)

static u16 *const VGA_ADDRESS = (u16 *)0xB8000;

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
    void clear_screen();

    void enable_cursor(bool enable);
    void set_cursor_coordinates(u16 x, u16 y);
};

#include <stdarg.h>

void Vga::print(String fmt, ...) {
    va_list a_list;
    va_start(a_list, fmt);

    for (s64 i = 0; i < fmt.length; ++i) {
        u8 c = fmt.data[i];

        if (c == '%') {
            if (i < fmt.length-1) {
                ++i;
                c = fmt.data[i];

                if (c == 'u') {
                    write_u32(va_arg(a_list, u32));
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

    va_end(a_list);
}

void Vga::write_u32(u32 value) {
    char *hex_values = "0123456789ABCDEF";

    for (int i = 0; i < 8; ++i) {
        u32 v = (value >> ((7 - i)*4)) & 0xF;
        write(hex_values[v]);
    }
}

void Vga::write(u8 c) {
    if (c == '\n') {
        buffer_cursor_pos_x = 0;
        buffer_cursor_pos_y++;
        // @FixMe handle end of height!

        return;
    }
    u64 index = buffer_cursor_pos_x + VGA_WIDTH * buffer_cursor_pos_y;
    buffer[index] = color | c;
    
    buffer_cursor_pos_x++;

    if (buffer_cursor_pos_x >= VGA_WIDTH) {
        buffer_cursor_pos_x = 0;
        buffer_cursor_pos_y++;
        // @FixMe handle end of height!
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

extern "C"
void kernel_main(Multiboot_Information *info) {
    Vga vga;
    vga.enable_cursor(true);
    vga.clear_screen();
    vga.print(temp_string("Hello, Sailor!\n"));
    vga.print(temp_string("mem_lower: %u\n"), info->mem_lower);
    vga.print(temp_string("mem_upper: %u\n"), info->mem_upper);
}
