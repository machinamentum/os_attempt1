#include <stdint.h>
#include <stdarg.h>

#define ALIGN(x) __attribute__((aligned(x)))


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
    u32 mods_count;
    void *mods_addr;
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

#define S(str) (temp_string((str)))

void kprint(String s, ...);

#define VGA_WIDTH 80
#define VGA_HEGIHT 25
#define VGA_COLOR(fg, bg) (((fg) | ((bg) << 4)) << 8)
#define VGA_BLINK_FLAG ((1 << 8) << 8)

static u16 *const VGA_ADDRESS = (u16 *)(0xB8000 + 0xC0000000);

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
};

void Vga::print_valist(String fmt, va_list a_list) {
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
}

void Vga::print(String fmt, ...) {
    va_list a_list;
    va_start(a_list, fmt);
    print_valist(fmt, a_list);
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

// this manages the initial 128MB or so worth of pages
// then we can find pages for use for other memory allocations
// including more pages for bitmaps for further pages
u32 initial_memory_use_bitmap[1024] ALIGN(4096);
u32 upper_memory_size; // set in kernel_main

u32 page_allocator_init() {
    for (int i = 0; i < 1024; ++i) {
        initial_memory_use_bitmap[i] = 0; 
    }
    
    
}

#define PAGE_PRESENT           (1 << 0)
#define PAGE_READ_WRITE        (1 << 1)
#define PAGE_IS_NOT_PRIVILEGED (1 << 2) // 1; page is accesable to user land, otherwise only the kernel has access
#define PAGE_USE_WRITE_THROUGH (1 << 3) // 1; write-through caching, otherwise write-back
#define PAGE_DO_NOT_CACHE      (1 << 4)
#define PAGE_HAS_BEEN_ACCESSD  (1 << 5)
#define PAGE_IS_DIRTY          (1 << 6)
#define PAGE_SIZE_4MiB         (1 << 7) // 1; page size 4MiB, otherwise page size 4KiB 
#define PAGE_GLOBAL_BIT        (1 << 8)


extern "C"
void load_page_directory(u32 *page_directory);
extern "C"
void enable_paging();
extern "C"
void flush_tlb();
extern "C"
void invalidate_page_i486(u32 page);

void invalidate_page(u32 page) {
    // @TODO maybe
    // if (supports_invlpg(cpu)) {
    invalidate_page_i486(page);
    // } else {
    //     flush_tlb();
    // }
}


u32 page_directory[1024] ALIGN(4096);
u32 first_page_table[1024] ALIGN(4096);

/*
u32 virtual_to_physical_address(u32 virtual_addr) {
    u32 dir_index = virtual_addr >> 22;
    u32 table_index = (virtual_addr >> 12) & 0x03FF;
    
    u32 *pd = (u32 *) 0xFFFFF000;
    u32 *pt = ((u32 *) 0xFFC00000) + (0x400 * dir_index);
    
    if (!(pd[dir_index] & PAGE_PRESENT)) return 0;
    if (!(pt[table_index] & PAGE_PRESENT)) return 0;
    
    return (pt[table_index] & ~0xFFF) + (virtual_addr & 0xFFF);
}
*/

void map_page(u32 physical, u32 virtual_addr, u32 flags) {
    u32 dir_index = virtual_addr >> 22;
    u32 table_index = (virtual_addr >> 12) & 0x03FF;
    
    u32 *pd = (u32 *) 0xFFFFF000;
    u32 *pt = ((u32 *) 0xFFC00000) + (0x400 * dir_index);
    
    if (!(pd[dir_index] & PAGE_PRESENT)) {
        
    }
    
    pt[table_index] = (physical | (flags & 0xFFF)) | PAGE_PRESENT;
}

extern "C"
void unmap_page_table(u32 dir_index) {
    page_directory[dir_index] = 0x00000002;
    u32 *pd = (u32 *)(((u32)&page_directory) - 0xC0000000);
    invalidate_page((u32) pd);
}


extern int __KERNEL_VIRTUAL_BASE;

// operates in physical address space!
// should only be called by boot.s!
extern "C"
u32 *init_page_table_directory() {
    u16 *vga = (u16 *)0xB8000;
    
    u32 *pd = (u32 *)(((u32)&page_directory) - 0xC0000000);
    u32 *pt = (u32 *)(((u32)&first_page_table) - 0xC0000000);
    
    for (int i = 0; i < 1024; ++i) {
        pd[i] = 0x00000002;
    }
    
    for (int i = 0; i < 1024; ++i) {
        pt[i] = (i * 0x1000) | PAGE_PRESENT | PAGE_READ_WRITE;
    }
    
    u32 dir_index = ((u32)&__KERNEL_VIRTUAL_BASE) >> 22;
    pd[0] = ((u32) pt) | PAGE_PRESENT | PAGE_READ_WRITE;
    pd[dir_index] = ((u32) pt) | PAGE_PRESENT | PAGE_READ_WRITE;
    // page_directory[0] = ((u32) first_page_table) | PAGE_PRESENT | PAGE_READ_WRITE;
    
    // map last page to the PDE
    pd[1023] = ((u32) pd) | PAGE_PRESENT | PAGE_READ_WRITE;
    *vga = VGA_COLOR(Vga::Color::WHITE, Vga::Color::BLACK) | 'H';
    return &pd[0];
}

Vga vga;

void kprint(String s, ...) {
    va_list a_list;
    va_start(a_list, s);
    vga.print_valist(s, a_list);
    va_end(a_list);
}

extern "C"
void kernel_main(Multiboot_Information *info) {
    upper_memory_size = info->mem_upper;
    vga = Vga();
    
    // init_page_table_directory();
    // load_page_directory(&page_directory[0]);
    // enable_paging();
    
    // map_page((u32) VGA_ADDRESS, 0x34000000);
    
    vga.enable_cursor(true);
    vga.clear_screen();
    kprint(S("Hello, Sailor!\n"));
    kprint(S("mem_lower: %u\n"), info->mem_lower);
    kprint(S("mem_upper: %u\n"), info->mem_upper);
}
