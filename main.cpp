#include <stdint.h>
#include <stdarg.h>

#define ALIGN(x) __attribute__((aligned(x)))
#define PACKED   __attribute__((packed))

#define PAGE_SIZE 0x1000

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

struct Multiboot_Mmap {
    u32 size;
    u64 base_addr;
    u64 length;

    enum {
        MEMORY_AVAILABLE = 1,
        MEMORY_RESERVED = 2,
        MEMORY_ACPI_RECLAIMABLE = 3,
        MEMORY_NVS = 4,
        MEMORY_BADRAM = 5,
    };
    u32 type;
} PACKED;

struct Multiboot_Information {
    u32 flags;
    u32 mem_lower;
    u32 mem_upper;
    u32 boot_device;
    u32 cmdline;
    u32 mods_count;
    u32 mods_addr;
    u8 syms[12];
    u32 mmap_length;
    u32 mmap_addr;
    u32 drives_length;
    u32 drives_addr;
    u32 config_table;
    u32 boot_loader_name;
};

extern "C"
void _port_io_write_u32(u16 port, u32 value);
extern "C"
void _port_io_write_u8(u16 port, u8 value);

extern "C"
u8  _port_io_read_u8(u16 port);
extern "C"
u32 _port_io_read_u32(u16 port);
extern "C"
void _io_wait();

extern "C"
void __irq_0x00_handler();

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

void memcpy(void *dst, void *src, u32 num) {
    u8 *_dst = reinterpret_cast<u8 *>(dst);
    u8 *_src = reinterpret_cast<u8 *>(src);

    for (u32 i = 0; i < num; ++i) {
        _dst[i] = _src[i];
    }
}

#define S(str) (temp_string((str)))

void kprint(String s, ...);

#define VGA_WIDTH 80
#define VGA_HEGIHT 24
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
    void scroll_one_line();
};

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
                
                if (c == 'u') {
                    write_u32(va_arg(a_list, u32));
                } else if (c == 'S') {
                    String as = va_arg(a_list, String);
                    print(as);
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


u32 page_directory[1024] ALIGN(PAGE_SIZE);
u32 first_page_table[1024] ALIGN(PAGE_SIZE);

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
        pt[i] = (i * PAGE_SIZE) | PAGE_PRESENT | PAGE_READ_WRITE;
    }
    
    u32 dir_index = ((u32)&__KERNEL_VIRTUAL_BASE) >> 22;
    pd[0] = ((u32) pt) | PAGE_PRESENT | PAGE_READ_WRITE;
    pd[dir_index] = ((u32) pt) | PAGE_PRESENT | PAGE_READ_WRITE;
    
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

void kprint_valist(String s, va_list a_list) {
    vga.print_valist(s, a_list);
}

void kerror(String s, ...) {
    va_list a_list;
    va_start(a_list, s);
    kprint_valist(s, a_list);
    va_end(a_list);
    asm("hlt");
}

void _kassert(bool arg, String s, String file, u32 line) {
    if (arg) return;
    
    kerror(S("Assertion failed: %S,%u: %S"), file, line, s);
}
#define kassert(arg) _kassert((arg), S(#arg), S(__FILE__), __LINE__)

extern "C"
void set_gdt(void *gdt, u16 size);

u64 gdt_table[64];
struct {
    u16 size;
    u32 offset;
} gdt_descriptor;

void encode_gdt_entry(u64 *gdt_entry, u32 base, u32 limit, u8 type) {
    u8 *target = reinterpret_cast<u8 *>(gdt_entry);
    
    if ((limit > 65536) && ((limit & 0xFFF) != 0xFFF)) {
        kerror(S("Error: GDT limit is invalid"));
    }
    
    if (limit > 65536) {
        limit = limit >> 12;
        target[6] = 0xC0;
    } else {
        target[6] = 0x40;
    }
    
    target[0] = limit & 0xFF;
    target[1] = (limit >> 8) & 0xFF;
    target[6] |= (limit >> 16) & 0xF;
    
    target[2] = base & 0xFF;
    target[3] = (base >> 8) & 0xFF;
    target[4] = (base >> 16) & 0xFF;
    target[7] = (base >> 24) & 0xFF;
    
    target[5] = type;
}

#define INTERRUPT_TYPE_TASK_GATE_32 (0x5)
#define INTERRUPT_TYPE_GATE_16      (0x6)
#define INTERRUPT_TYPE_TRAP_GATE_16 (0x7)
#define INTERRUPT_TYPE_GATE_32      (0xE)
#define INTERRUPT_TYPE_TRAP_GATE_32 (0xF)
#define INTERRUPT_PRESENT           (1 << 7)
#define INTERRUPT_STORAGE_SEGMENT   (1 << 4)

extern "C"
void set_idt(void *idt, u16 size);

extern "C"
void irq_handler(u32 irq) {
    kerror(S("IRQ: %u"), irq);
}

struct Idt_Descriptor {
    u16 offset_1;
    u16 selector;
    u8 zero;
    u8 type_attr;
    u16 offset_2;
};

Idt_Descriptor idt_table[256];

void set_idt_entry(Idt_Descriptor *idt, u32 offset, u8 type_attr, u8 privelege) {
    idt->offset_1 = offset & 0xFFFF;
    idt->selector = 0x08;
    idt->zero = 0;
    idt->type_attr = ((privelege << 5) & 0x3) | type_attr;
    idt->offset_2 = (offset >> 16) & 0xFFFF;
}

void init_interrupt_descriptor_table() {
    for (int i = 0; i < 256; ++i) {
        Idt_Descriptor *idt = &idt_table[i];
        set_idt_entry(idt, (u32)&__irq_0x00_handler, INTERRUPT_PRESENT | INTERRUPT_TYPE_GATE_32, 0);
    }
}

#define PIC1    0x20
#define PIC2    0xA0
#define PIC1_DATA 0x21
#define PIC2_DATA 0xA1

void pic_set_eoi(u8 irq) {
    if (irq >= 8) _port_io_write_u8(PIC2, 0x20);
    _port_io_write_u8(PIC1, 0x20);
}

void pic_remap(u8 offset1, u8 offset2) {
    u8 a1 = _port_io_read_u8(PIC1_DATA);
    u8 a2 = _port_io_read_u8(PIC2_DATA);
    
    _port_io_write_u8(PIC1, 0x11); _io_wait();
    _port_io_write_u8(PIC2, 0x11); _io_wait();
    
    _port_io_write_u8(PIC1_DATA, offset1); _io_wait();
    _port_io_write_u8(PIC2_DATA, offset2); _io_wait();
    _port_io_write_u8(PIC1_DATA, 4); _io_wait(); // tell master that a slave is at IRQ2
    _port_io_write_u8(PIC2_DATA, 2); _io_wait(); // tell slave that it is cascading
    _port_io_write_u8(PIC1_DATA, 0x01); _io_wait();
    _port_io_write_u8(PIC2_DATA, 0x01); _io_wait();
    
    _port_io_write_u8(PIC1_DATA, a1);
    _port_io_write_u8(PIC2_DATA, a2);
}

void set_irq_mask(u8 irq_line) {
    u16 port = PIC1_DATA;
    if (irq_line >= 8) {
        port = PIC2_DATA;
        irq_line -= 8;
    }
    
    u8 value = _port_io_read_u8(port) | (1 << irq_line);
    _port_io_write_u8(port, value);
}

void clear_irq_mask(u8 irq_line) {
    u16 port = PIC1_DATA;
    if (irq_line >= 8) {
        port = PIC2_DATA;
        irq_line -= 8;
    }
    
    u8 value = _port_io_read_u8(port) & ~(1 << irq_line);
    _port_io_write_u8(port, value);
}

void test(void *a) {
    kprint(S(""), a);
}

extern "C"
void kernel_main(Multiboot_Information *info) {
    upper_memory_size = info->mem_upper;
    vga = Vga();
    
    vga.enable_cursor(true);
    vga.clear_screen();
    kprint(S("Hello, Sailor!\n"));
    kprint(S("mem_lower: %u\n"), info->mem_lower);
    kprint(S("mem_upper: %u\n"), info->mem_upper);
    kprint(S("Setting up GDT..."));
    
    encode_gdt_entry(&gdt_table[0], 0, 0, 0);
    encode_gdt_entry(&gdt_table[1], 0, 0xFFFFFFFF, 0x9A); // code segment
    encode_gdt_entry(&gdt_table[2], 0, 0xFFFFFFFF, 0x92); // data segment
    gdt_descriptor.size = sizeof(u64) * 3;
    gdt_descriptor.offset = reinterpret_cast<u32>(&gdt_table[0]);
    set_gdt(&gdt_table, sizeof(u64) * 3);
    kprint(S("done\n"));
    kprint(S("Setting up IDT..."));
    init_interrupt_descriptor_table();
    set_idt(&idt_table, sizeof(idt_table[0]) * 256);
    pic_remap(0x20, 0x28);
    // asm("sti");
    kprint(S("done\n"));
    
    kprint(S("Testing interrupt..."));
    int k = 1;
    int a = 0;
    // k = k / a;
    kprint(S("done\n"));
    test(&k);

    for (int i = 0; i <= 32; ++i) {
        kprint(S("%u\n"), i);
    }

    kprint(S("END!"));
}
