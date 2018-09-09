
#include "kernel.h"
#include "vga.h"
#include "interrupts.h"
#include "heap.h"

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

void  *zero_memory(void *_dst, u32 size) {
    u8 *dst = reinterpret_cast<u8 *>(_dst);
    for (u32 i = 0; i < size; ++i) {
        dst[i] =  0;
    }

    return _dst;
}

enum ALLOCATOR_MODE {
    FREE,
    ALLOC,
};

typedef void *(*allocator_type)(ALLOCATOR_MODE, void *existing, s64 size);

template <typename T>
struct Array {
    T *data = nullptr;
    s64 count = 0;
    s64 allocated = 0;
    allocator_type allocator = nullptr;

    T &operator [](s64 index) {
        kassert(index >= 0 && index < count);
        return data[index];
    }

    void reserve(s64 size) {
        if (size > allocated) {
            T *ndata = reinterpret_cast<T *>(allocator(ALLOC, nullptr, size * sizeof(T)));
            memcpy(ndata, data, count * sizeof(T));
            allocator(FREE, data, 0);
            data = ndata;
            allocated = size;
        }
    }

    void resize(s64 size) {
        reserve(size);

        count = size;
    }
};

struct Bitmap_Entry {
    u32 range_start;
    u32 range_end;
    u32 *buffer;
};

#define BITMAP_BUFFER_COUNT 1024
#define BITMAP_NUM_PAGES    (BITMAP_BUFFER_COUNT * 32)

u32 initial_memory_use_bitmap[BITMAP_BUFFER_COUNT] ALIGN(4096);
Bitmap_Entry initial_bitmap_entry;

u32 upper_memory_size_pages; // initially set in kernel_main
Array<Bitmap_Entry> bitmap_entries;
u32 num_bitmap_entries;

void mark_page_as_used(u32 physical) {
    kassert((physical & (PAGE_SIZE-1)) == 0);
    kassert(physical >= 0x00100000);

    u32 page_number = ((physical - 0x00100000) / PAGE_SIZE);
    u32 bitmap_index = page_number / BITMAP_NUM_PAGES;
    u32 buffer_index = (page_number / 32) % BITMAP_BUFFER_COUNT;
    u32 page_bit = page_number % 32;

    bitmap_entries[bitmap_index].buffer[buffer_index] |= (1 << page_bit);
}

void mark_page_as_free(u32 physical) {
    kassert((physical & (PAGE_SIZE-1)) == 0);
    kassert(physical >= 0x00100000);

    u32 page_number = ((physical - 0x00100000) / PAGE_SIZE);
    u32 bitmap_index = page_number / BITMAP_NUM_PAGES;
    u32 buffer_index = (page_number / 32) % BITMAP_BUFFER_COUNT;
    u32 page_bit = page_number % 32;

    bitmap_entries[bitmap_index].buffer[buffer_index] &= ~(1 << page_bit);
}


void mark_page_range_as_used(u32 physical_start, u32 physical_end) {
    kassert((physical_start & (PAGE_SIZE-1)) == 0);
    kassert((physical_end & (PAGE_SIZE-1)) == 0);
    kassert(physical_start <= physical_end);

    // @Speed there's probably a faster way to mark large memory regions as used
    for (; physical_start <= physical_end; physical_start += PAGE_SIZE) {
        mark_page_as_used(physical_start);
    }
}
u32 maybe_take_ownership_of_num_pages(u32 num) {
    if (num > upper_memory_size_pages) {
        u32 out = upper_memory_size_pages;
        upper_memory_size_pages = 0;
        return out;
    }

    upper_memory_size_pages -= num;
    return num;
}

void make_bitmap_entry(Bitmap_Entry *entry, u32 range_start, u32 num_pages, u32 *bitmap) {
    kassert(num_pages && "cannot make a bitmap entry of zero size!");

    entry->range_start = range_start;
    entry->range_end = range_start + (num_pages * PAGE_SIZE);
    entry->buffer = bitmap;
}

u32 next_free_page() {
    for (s64 i = 0; i < bitmap_entries.count; ++i) {
        auto entry = bitmap_entries[i];

        // @TODO i'm pretty sure this drops a handful of pages if we have a number of pages that don't divide evenly into 32
        u32 buffer_count = ((entry.range_end - entry.range_start) / PAGE_SIZE) / 32;
        for (u32 j = 0; j < buffer_count; ++j) {
            u32 value = entry.buffer[j];
            if (value == 0xFFFFFFFF) continue;

            for (int k = 0; k < 32; ++k) {
                if (((value >> k) & 1) == 0) {
                    u32 page = (k * PAGE_SIZE) + (j * 32 * PAGE_SIZE) + entry.range_start;
                    mark_page_as_used(page);
                    return page;
                }
            }
        }
    }

    return 0;
}

void page_allocator_init() {
    u32 total_num_pages = upper_memory_size_pages;

    zero_memory(&initial_memory_use_bitmap, sizeof(initial_memory_use_bitmap));
    make_bitmap_entry(&initial_bitmap_entry, 0x00100000, maybe_take_ownership_of_num_pages(BITMAP_NUM_PAGES), &initial_memory_use_bitmap[0]);
    bitmap_entries.data = &initial_bitmap_entry;
    bitmap_entries.allocated = 1;
    bitmap_entries.count = 1;
    
    extern int __KERNEL_MEMORY_START;
    extern int __KERNEL_MEMORY_END;

    u32 kstart = (u32)&__KERNEL_MEMORY_START;
    u32 kend = (u32)&__KERNEL_MEMORY_END;

    kprint("kernel physical address: %X\n", kstart - KERNEL_VIRTUAL_BASE_ADDRESS);
    kprint("kernel end:              %X\n", kend - KERNEL_VIRTUAL_BASE_ADDRESS);

    // mark kernel area as in use
    mark_page_range_as_used(kstart - KERNEL_VIRTUAL_BASE_ADDRESS, kend - KERNEL_VIRTUAL_BASE_ADDRESS);

    // // now that we have a base tracker we can dynamically allocate
    // // a region for bitmaps to track all of availabe physical memory
    // bitmap_entries.resize((total_num_pages / BITMAP_NUM_PAGES) + ((total_num_pages % BITMAP_NUM_PAGES) ? 1 : 0));

    // kprint("Total ram size: %u MB\n", (total_num_pages * PAGE_SIZE) / (1024*1024));
    // kprint("Bitmap entry count: %d\n", bitmap_entries.count);

    // for (s64 i = 1; i < bitmap_entries.count; ++i) {
    //     auto last_etry = *bitmap_entries[i-1];
    //     u32 num_pages = maybe_take_ownership_of_num_pages(BITMAP_NUM_PAGES);
    //     kassert(num_pages);
    //     // make_bitmap_entry();
    // }
}

extern "C"
void load_page_directory(u32 page_directory);
extern "C"
void enable_paging();
extern "C"
void flush_tlb();
extern "C"
void invalidate_page_i486(u32 page);

void invalidate_page(u32 page) {
    // @TODO maybe, invlpg instruction exists in i486 and newer, but we probably don't care to support anything thats several decades old!
    // if (supports_invlpg(cpu)) {
    invalidate_page_i486(page);
    // } else {
    //     flush_tlb();
    // }
}


u32 page_directory[1024] ALIGN(PAGE_SIZE);
u32 first_page_table[1024] ALIGN(PAGE_SIZE);
// this will allow the heap to map pages into the heap's address space
// until we can map + generate page tables from the heap
u32 heap_page_table[1024] ALIGN(PAGE_SIZE);


u32 virtual_to_physical_address(u32 virtual_addr) {
    u32 dir_index = virtual_addr >> 22;
    u32 table_index = (virtual_addr >> 12) & 0x03FF;
    
    u32 *pd = (u32 *) 0xFFFFF000;
    u32 *pt = ((u32 *) 0xFFC00000) + (0x400 * dir_index);
    
    if (!(pd[dir_index] & PAGE_PRESENT)) return 0;
    if (!(pt[table_index] & PAGE_PRESENT)) return 0;
    
    return (pt[table_index] & ~0xFFF) + (virtual_addr & 0xFFF);
}

void map_page(u32 physical, u32 virtual_addr, u32 flags) {
    u32 dir_index = virtual_addr >> 22;
    u32 table_index = (virtual_addr >> 12) & 0x03FF;
    
    u32 *pd =  (u32 *) 0xFFFFF000;
    u32 *pt = ((u32 *) 0xFFC00000) + (0x400 * dir_index);
    
    if (!(pd[dir_index] & PAGE_PRESENT)) {
        u32 *table = nullptr;
        for (int i = 0; i < 1024; ++i) {
            table[i] = PAGE_READ_WRITE;
        }
        pd[dir_index] = virtual_to_physical_address((u32) table) | PAGE_PRESENT | PAGE_READ_WRITE;
        flush_tlb(); // @Cleanup invalidate the page?

        // it should be accessible now!
        pt = ((u32 *) 0xFFC00000) + (0x400 * dir_index);
    }
    
    pt[table_index] = (physical | (flags & 0xFFF)) | PAGE_PRESENT;
}

void unmap_page(u32 virtual_addr) {
    u32 dir_index = virtual_addr >> 22;
    u32 table_index = (virtual_addr >> 12) & 0x03FF;
    
    u32 *pd = (u32 *) 0xFFFFF000;
    u32 *pt = ((u32 *) 0xFFC00000) + (0x400 * dir_index);
    
    if (!(pd[dir_index] & PAGE_PRESENT)) return;
    if (!(pt[table_index] & PAGE_PRESENT)) return;

    pt[table_index] = PAGE_READ_WRITE;
}

extern "C"
void unmap_page_table(u32 dir_index) {
    page_directory[dir_index] = 0x00000002;
    u32 *pd = (u32 *)(((u32)&page_directory) - KERNEL_VIRTUAL_BASE_ADDRESS);
    invalidate_page((u32) pd);
}

void map_page_table(u32 *table, u32 virtual_addr) {
    u32 table_physical = virtual_to_physical_address(reinterpret_cast<u32>(table));
    u32 dir_index = virtual_addr >> 22;

    u32 *pd = (u32 *) 0xFFFFF000;
    pd[dir_index] = table_physical | PAGE_PRESENT | PAGE_READ_WRITE;
    flush_tlb(); // are we supposed to invalidate the directory or the table?
}

// operates in physical address space!
// should only be called by boot.s!
extern "C"
u32 *init_page_table_directory() {
    u32 *pd = (u32 *)(((u32)&page_directory) - KERNEL_VIRTUAL_BASE_ADDRESS);
    u32 *pt = (u32 *)(((u32)&first_page_table) - KERNEL_VIRTUAL_BASE_ADDRESS);
    u32 *hpt = (u32 *)(((u32)&heap_page_table) - KERNEL_VIRTUAL_BASE_ADDRESS);

    for (int i = 0; i < 1024; ++i) {
        pd[i] = PAGE_READ_WRITE;
    }
    
    for (int i = 0; i < 1024; ++i) {
        pt[i] = (i * PAGE_SIZE) | PAGE_PRESENT | PAGE_READ_WRITE;
    }

    for (int i = 0; i < 1024; ++i) {
        hpt[i] = PAGE_READ_WRITE;
    }
    
    u32 dir_index = KERNEL_VIRTUAL_BASE_ADDRESS >> 22;
    pd[0] = ((u32) pt) | PAGE_PRESENT | PAGE_READ_WRITE;
    pd[dir_index] = ((u32) pt) | PAGE_PRESENT | PAGE_READ_WRITE;

    dir_index = HEAP_VIRTUAL_BASE_ADDRESS >> 22;
    pd[dir_index] = ((u32) hpt) | PAGE_PRESENT | PAGE_READ_WRITE;
    
    // map last page to the PDE
    pd[1023] = ((u32) pd) | PAGE_PRESENT | PAGE_READ_WRITE;
    return &pd[0];
}

Vga vga;

void kprint(char *s, ...) {
    va_list a_list;
    va_start(a_list, s);
    vga.print_valist(temp_string(s), a_list);
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
    kprint_valist(temp_string(s), a_list);
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

void _kassert(bool arg, char *s, char *file, u32 line) {
    _kassert(arg, temp_string(s), temp_string(file), line);
}

void _kassert(bool arg, String s, String file, u32 line) {
    if (arg) return;
    
    asm("cli");
    kerror("Assertion failed: %S,%u: %S", file, line, s);
    for (;;) {
        asm("hlt");
    }
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
        kerror("Error: GDT limit is invalid");
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

#define PIC_READ_IRR 0x0A;
#define PIC_READ_ISR 0x0B

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


u16 pic_get_isr() {
    _port_io_write_u8(PIC1, PIC_READ_ISR);
    _port_io_write_u8(PIC2, PIC_READ_ISR);
    return (_port_io_read_u8(PIC2) << 8) | _port_io_read_u8(PIC1);
}

extern "C"
void kernel_main(Multiboot_Information *info) {
    upper_memory_size_pages = info->mem_upper / 4; // convert KB to pages (4096 byte blocks)
    vga = Vga();
    
    vga.enable_cursor(true);
    vga.clear_screen();
    kprint("Hello, Sailor!\n");
    kprint("mem_lower: %u\n", info->mem_lower);
    kprint("mem_upper: %u\n", info->mem_upper);
    kprint("Setting up GDT...");
    
    encode_gdt_entry(&gdt_table[0], 0, 0, 0);
    encode_gdt_entry(&gdt_table[1], 0, 0xFFFFFFFF, 0x9A); // code segment
    encode_gdt_entry(&gdt_table[2], 0, 0xFFFFFFFF, 0x92); // data segment
    gdt_descriptor.size = sizeof(u64) * 3;
    gdt_descriptor.offset = reinterpret_cast<u32>(&gdt_table[0]);
    set_gdt(&gdt_table, sizeof(u64) * 3);
    kprint("done\n");
    kprint("Setting up IDT...");
    init_interrupt_descriptor_table();
    pic_remap(0x20, 0x28);
    asm("sti");
    kprint("done\n");

    page_allocator_init();
    init_heap();

    kprint("Kernel is at physical addr: %X\n", virtual_to_physical_address(KERNEL_VIRTUAL_BASE_ADDRESS + 0x00100000));

    // kprint("Testing interrupt...");
    // kprint("done\n");

    kerror("END!");
    for(;;) {
        asm("hlt");
    }
}
