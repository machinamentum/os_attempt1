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

struct String {
    u8 *data;
    s64 length;
};

s64 strlen(const char *c_string) {
    if (!c_string) return 0;
    
    const char *s = c_string;
    while (*s) ++s;
    
    return s - c_string;
}

String temp_string(const char *c_string) {
    String s;
    s.data = const_cast<char *>(c_string);
    s.length = strlen(c_string);
    return s;
}

#define VGA_COLOR(fg, bg) (((fg) | ((bg) << 4)) << 8)

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
    u64 cursor_pos = 0;
    void write(String s);
};

void Vga::write(String s) {
    for (s64 i = 0; i < s.length; ++i) {
        // @TODO handle end-of-buffer
        buffer[cursor_pos] = color | s[i];
        
        cursor_pos++;
    }
}


extern "C"
void kernel_main() {
    Vga vga;
    vga.write(temp_string("Hi")));
}
