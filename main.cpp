
#include "kernel.h"
#include "vga.h"
#include "interrupts.h"

#include <stdarg.h>

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

void kprint(String s, ...);

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
    
    u32 dir_index = KERNEL_VIRTUAL_BASE_ADDRESS >> 22;
    pd[0] = ((u32) pt) | PAGE_PRESENT | PAGE_READ_WRITE;
    pd[dir_index] = ((u32) pt) | PAGE_PRESENT | PAGE_READ_WRITE;
    
    // map last page to the PDE
    pd[1023] = ((u32) pd) | PAGE_PRESENT | PAGE_READ_WRITE;
    *vga = VGA_COLOR(Vga::Color::WHITE, Vga::Color::BLACK) | 'H';
    return &pd[0];
}

Vga vga;

void kprint(char *s, ...) {
    va_list a_list;
    va_start(a_list, s);
    vga.print_valist(S(s), a_list);
    va_end(a_list);
}

void kprint(String s, ...) {
    va_list a_list;
    va_start(a_list, s);
    vga.print_valist(s, a_list);
    va_end(a_list);
}

void kprint_valist(String s, va_list a_list) {
    vga.print_valist(s, a_list);
}

void kerror(char *s, ...) {
    va_list a_list;
    va_start(a_list, s);
    kprint_valist(S(s), a_list);
    va_end(a_list);
    asm("hlt");
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
    
    kerror("Assertion failed: %S,%u: %S", file, line, s);
}

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


#define PIC1    0x20
#define PIC2    0xA0
#define PIC1_DATA 0x21
#define PIC2_DATA 0xA1

void pic_set_eoi(u8 irq) {
    if (irq >= 0x28) _port_io_write_u8(PIC2, 0x20);
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
    
    _port_io_write_u8(PIC1_DATA, 0);
    _port_io_write_u8(PIC2_DATA, 0);
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
    pic_remap(0x20, 0x28);
    asm("sti");
    kprint(S("done\n"));
    
    kprint(S("Testing interrupt..."));
    kprint(S("done\n"));

    kerror(S("END!"));
    for(;;) {
        asm("hlt");
    }
}
