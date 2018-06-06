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

typedef char *c_string;

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
    void write(const c_string s);
};

void Vga::write(const c_string s) {
    buffer[0] = ((2 | (0)) << 8) | s[0];
    buffer[1] = ((2 | (0)) << 8) | s[1];
}


extern "C"
void kernel_main() {
    Vga vga;
    vga.write("Hi");
}
