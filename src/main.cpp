
#include "kernel.h"
#include "vga.h"
#include "interrupts.h"
#include "heap.h"
#include "keyboard.h"
#include "pci.h"
#include "print.h"

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
    
    extern u8 __KERNEL_MEMORY_START[];
    extern u8 __KERNEL_MEMORY_END[];
    
    u32 kstart = (u32)__KERNEL_MEMORY_START;
    u32 kend = (u32)__KERNEL_MEMORY_END;
    
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

extern "C" {
    void load_page_directory(u32 page_directory);
    
    void enable_paging();
    
    void flush_tlb();
    
    void invalidate_page_i486(u32 page);
}

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
        u32 *table = nullptr; // @Incomplete we crash if hit here
        kassert(table);
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

struct Stream {
    void *payload;
    putchar_callback putc_cb;
};

Stream streams[1];

void kprint_valist(String s, va_list a_list) {
    Stream *st = &streams[0];
    print_valist_callback(s, a_list, st->payload, st->putc_cb);
}

void kprint(char *s, ...) {
    va_list a_list;
    va_start(a_list, s);
    kprint_valist(temp_string(s), a_list);
    va_end(a_list);
}

void kprint(String s, ...) {
    va_list a_list;
    va_start(a_list, s);
    kprint_valist(s, a_list);
    va_end(a_list);
}


// @FixMe kerror needs to be fatal and we should assume that the environment is compromised; we need to forcefully drop down into the basic VGA text mode here and then print to that.
// @FixMe kerror needs to be fatal and we should assume that the environment is compromised; we need to forcefully drop down into the basic VGA text mode here and then print to that.
// @FixMe kerror needs to be fatal and we should assume that the environment is compromised; we need to forcefully drop down into the basic VGA text mode here and then print to that.
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

struct {
    int num_channels;
} ps2_info;

void ps2_wait_for_response() {
    while (true) {
        u8 data = _port_io_read_u8(PS2_STATUS);
        if (data & PS2_STATUS_OUTPUT_BUFFER_BIT) break;
    }
}

void ps2_wait_for_output_clear() {
    while(_port_io_read_u8(PS2_STATUS) & PS2_STATUS_OUTPUT_BUFFER_BIT) ;
}

void ps2_wait_for_input_ready() {
    while (true) {
        u8 data = _port_io_read_u8(PS2_STATUS);
        if (!(data & PS2_STATUS_INPUT_BUFFER_BIT)) break;
    }
}

// WARNING: this should be called only after disabling the PS/2 devices, otherwise we can get stuck if the PS/2 devices keep filling the buffers
void ps2_flush_output_buffers() {
    while (_port_io_read_u8(PS2_STATUS) & PS2_STATUS_OUTPUT_BUFFER_BIT)
        _port_io_read_u8(PS2_DATA);
}

void ps2_disable_devices() {
    // do we need to _io_wait when interfacing the PS/2?
    _port_io_write_u8(PS2_COMMAND, PS2_CMD_PORT_1_DISABLE); _io_wait();
    _port_io_write_u8(PS2_COMMAND, PS2_CMD_PORT_2_DISABLE); _io_wait();
}

void ps2_enable_devices() {
    _port_io_write_u8(PS2_COMMAND, PS2_CMD_PORT_1_ENABLE); _io_wait();
    _port_io_write_u8(PS2_COMMAND, PS2_CMD_PORT_2_ENABLE); _io_wait();
}

void ps2_initialize() {
    set_irq_mask(1);
    ps2_disable_devices();
    ps2_flush_output_buffers();
    
    _port_io_write_u8(PS2_COMMAND, PS2_CMD_READ_BYTE0);
    ps2_wait_for_response();
    u8 config_byte = _port_io_read_u8(PS2_DATA);
    kprint("Config: 0x%X\n", config_byte);
    // disable interrupts and port scancode set translation
    config_byte &= ~(PS2_CONFIG_PORT_1_INTERRUPT_BIT | PS2_CONFIG_PORT_2_INTERRUPT_BIT | PS2_CONFIG_PORT_1_TRANSLATION_BIT);
    
    if (config_byte & PS2_CONFIG_PORT_2_CLOCK_BIT) {
        ps2_info.num_channels = 2;
    } else {
        // this is an assumption but the osdev docs seem to indicate that unless your system doesnt have a PS/2 controller, then
        // port 1 is always active (unless the device is disconnected, maybe)
        ps2_info.num_channels = 1;
    }
    
    _io_wait();
    _port_io_write_u8(PS2_COMMAND, PS2_CMD_WRITE_BYTE0);
    ps2_wait_for_input_ready();
    _port_io_write_u8(PS2_DATA, config_byte);
    ps2_wait_for_input_ready();
    
    _port_io_write_u8(PS2_COMMAND, PS2_CMD_CONTROLLER_TEST);
    ps2_wait_for_response();
    u8 response = _port_io_read_u8(PS2_DATA);
    if (response != 0x55) {
        kprint("response: %X\n", response);
        kassert(false);
    }
    
    // @TODO maybe do a more thorough test for dual channel support
    
    _port_io_write_u8(PS2_COMMAND, PS2_CMD_PORT_1_TEST);
    ps2_wait_for_response();
    response = _port_io_read_u8(PS2_DATA);
    kprint("response: %X\n", response);
    kassert(response == 0x00);
    
    config_byte |= (PS2_CONFIG_PORT_1_INTERRUPT_BIT | PS2_CONFIG_PORT_2_INTERRUPT_BIT);
    _port_io_write_u8(PS2_COMMAND, PS2_CMD_WRITE_BYTE0);
    ps2_wait_for_input_ready();
    _port_io_write_u8(PS2_DATA, config_byte);
    
    ps2_wait_for_input_ready();
    ps2_enable_devices();
    
    ps2_wait_for_input_ready();
    _port_io_write_u8(PS2_DATA, 0xFF);
    ps2_wait_for_response();
    response = _port_io_read_u8(PS2_DATA);
    
    // The osdev wiki authors seem unsure what the actual behavio is here (getting 0xFA then 0xAA or vice versa)
    kassert(response == 0xFA);
    ps2_wait_for_response();
    response = _port_io_read_u8(PS2_DATA);
    kassert(response == 0xAA);
    if (response != 0xAA) {
        kprint("response: %X\n", response);
        kassert(false);
    }
    
    if (ps2_info.num_channels == 2) {
        // @TODO test second port
    }
    
    ps2_wait_for_input_ready();
    _port_io_write_u8(PS2_DATA, 0xF3);
    ps2_wait_for_input_ready();
    _port_io_write_u8(PS2_DATA, 0b11111 | (0b11 << 5));
    ps2_wait_for_response();
    response = _port_io_read_u8(PS2_DATA);
    kassert(response == 0xFA);
    
    clear_irq_mask(1);
}

u16 pci_read_u16(u32 bus, u32 slot, u32 func, u32 offset) {
    u32 addr = PCI_CONFIG_GET_ADDRESS(bus, slot, func, offset) | PCI_CONFIG_ENABLE_BIT;
    _port_io_write_u32(PCI_CONFIG_ADDRESS, addr);
    u32 value = _port_io_read_u32(PCI_CONFIG_DATA) >> (((offset & 2) * 8) & 0xFFFF);
    return static_cast<u16>(value);
}

void pci_write_u32(u32 bus, u32 slot, u32 func, u32 offset, u32 value) {
    u32 addr = PCI_CONFIG_GET_ADDRESS(bus, slot, func, offset) | PCI_CONFIG_ENABLE_BIT;
    _port_io_write_u32(PCI_CONFIG_ADDRESS, addr);
    _port_io_write_u32(PCI_CONFIG_DATA, value);
}

u32 pci_read_u32(u32 bus, u32 slot, u32 func, u32 offset) {
    u32 addr = PCI_CONFIG_GET_ADDRESS(bus, slot, func, offset) | PCI_CONFIG_ENABLE_BIT;
    _port_io_write_u32(PCI_CONFIG_ADDRESS, addr);
    return _port_io_read_u32(PCI_CONFIG_DATA);
}

u16 pic_check_vendor(u8 bus, u8 slot, u8 function) {
    return pci_read_u16(bus, slot, function, 0);
}

void pci_enable_memory(Pci_Device_Config *config) {
    u32 command = config->command | 0x0007;
    u32 packet = command;
    
    u8 bus = config->bus;
    u8 slot = config->slot;
    u8 func = config->function;
    pci_write_u32(bus, slot, func, 0x4, packet);
}

bool pci_read_device_config(Pci_Device_Config *header, u32 bus, u32 slot, u32 function) {
    kassert(bus < PCI_MAX_BUSES);
    kassert(slot < PCI_MAX_DEVICES_PER_BUS);
    kassert(function < PCI_MAX_FUNCTIONS_PER_DEVICE);
    
    if (pic_check_vendor(bus, slot, function) == 0xFFFF) return false;
    
    // kassert(sizeof(Pci_Device_Config) == 256);
    u32 *hdr = reinterpret_cast<u32 *>(header);
    for (u32 i = 0; i < 256/sizeof(u32); ++i) {
        hdr[i] = pci_read_u32(bus, slot, function, i*sizeof(u32));
    }
    
    return true;
}

Array<Pci_Device_Config> pci_devices;

void print_pci_header(Pci_Device_Config *header) {
    kprint("Vendor ID: %X\n", header->vendor_id);
    kprint("Device ID: %X\n", header->device_id);
    kprint("Class, subclass: %X, %X\n", header->class_code, header->subclass_code);
    // kprint("Header Type: %X\n", header->header_type);
}

void pci_enumerate_devices() {
    for (u16 bus = 0; bus < PCI_MAX_BUSES; ++bus) {
        for (u8 device = 0; device < PCI_MAX_DEVICES_PER_BUS; ++device) {
            Pci_Device_Config hdr;
            
            hdr.bus = bus;
            hdr.slot = device;
            hdr.function = 0;
            if (pci_read_device_config(&hdr, bus, device, 0)) {
                pci_devices.add(hdr);
                
                if (hdr.header_type & PCI_HEADER_MULTIFUNCTION_BIT) {
                    for (u8 func = 1; func < PCI_MAX_FUNCTIONS_PER_DEVICE; ++func) {
                        hdr.function = func;
                        if (pci_read_device_config(&hdr, bus, device, func)) {
                            pci_devices.add(hdr);
                        }
                    }
                }
            }
        }
    }
}

void create_ide_driver(Pci_Device_Config *header);
void create_svga_driver(Pci_Device_Config *header);

void kernel_shell();

extern "C"
void kernel_main(Multiboot_Information *info) {
    asm("cli");
    _port_io_write_u8(PIC1_DATA, 0xFF); _io_wait();
    _port_io_write_u8(PIC2_DATA, 0xFF); _io_wait();
    upper_memory_size_pages = info->mem_upper / 4; // convert KB to pages (4096 byte blocks)
    vga = Vga();
    
    Stream *output = &streams[0];
    output->payload = &vga;
    output->putc_cb = vga_putchar;
    
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
    kprint("done\n");
    
    ps2_initialize();
    asm("sti");
    
    page_allocator_init();
    
    init_heap();
    
    kprint("Kernel is at physical addr: %X\n", virtual_to_physical_address(KERNEL_VIRTUAL_BASE_ADDRESS + 0x00100000));
    
    // kprint("Image begins at phys: %X\n", virtual_to_physical_address((u32) image_data));
    // kprint("Image ends at phsy: %X\n", virtual_to_physical_address((u32)((&image_data[0]) + 24624)));
    
    // for (;;) asm("hlt");
    
    // kprint("Testing interrupt...");
    // kprint("done\n");
    
    pci_enumerate_devices();
    
    For (pci_devices) print_pci_header(&it);
    
    
    // @TODO we need to create a streamlined way to managed drivers that are built into the kernel's binary and driver's that are loaded
    // from the file system. Maybe. Perhaps the inital drivers are in the kernel and then proper drivers are loaded at runtime and they
    // fully replace the kernel drivers. This causes some dead code in the kernel, but such code will be in small amounts since we only
    // need minimal driver support to access the file system.
    For (pci_devices) {
        if (it.class_code == PCI_CLASS_MASS_STORAGE_CONTROLLER && it.subclass_code == PCI_SUBCLASS_IDE_CONTROLLER) {
            create_ide_driver(&it);
        }
        
        if (it.vendor_id == PCI_VENDOR_ID_VMWARE && it.device_id == PCI_DEVICE_ID_VMWARE_SVGA2) {
            create_svga_driver(&it);
        }
    }
    
    kernel_shell();
    
    for(;;) {
        asm("hlt");
    }
}

#include "nuklear.h"
#include "paris.c"
#include "string.h"

s32 get_glyph_start_end(u8 glyph, s32 *out_start, s32 *out_end) {
    if (glyph == ' ') {
        if (out_start) *out_start = 0;
        if (out_end)   *out_end = 0;
        return 6;
    }
    
    if (glyph < ' ' || glyph > 0x7F) {
        if (out_start) *out_start = 0;
        if (out_end)   *out_end = 0;
        return 0;
    }
    
    u8 index = glyph - ' ';
    
    s32 start = 0;
    s32 end = 0;
    u32 cur_index = -1; // -1 here so that when we hit the first marker, cur_index becomes 0, then when we hit the second marker, cur_index becomes 1 ('!')
    
    u32 *data = reinterpret_cast<u32 *>(&image_data[0]);
    for (s32 i = 0; i < image_width; ++i) {
        if (data[i] == 0xFFFFFFFF) {
            start = end;
            end = i;
            cur_index++;
            
            if (cur_index == index) {
                if (out_start) *out_start = start;
                if (out_end)   *out_end = end;
                return end - start;
            }
        }
    }
    
    return 0;
}

float nuklear_font_width(nk_handle handle, float h, const char* text, int len) {
    UNUSED(handle);
    UNUSED(h);
    
    float total = 0;
    for (int i = 0; i < len; ++i) {
        char c = text[i];
        total += get_glyph_start_end((u8) c, nullptr, nullptr);
        
    }
    return total;
}

extern struct VMW_SVGA_Driver {
    u16 index_port;
    u16 value_port;
    u16 bios_port;
    u16 irqstatus_port;
} svga_driver;

void svga_draw_rect(VMW_SVGA_Driver *svga, s32 x, s32 y, s32 width, s32 height, u32 color);
void svga_clear_screen(VMW_SVGA_Driver *svga, u32 color);
void svga_draw_rect_outline(VMW_SVGA_Driver *svga, s32 x, s32 y, s32 width, s32 height, u32 color);
void svga_update_screen(VMW_SVGA_Driver *svga);
void svga_draw_circle(VMW_SVGA_Driver *svga, s32 x, s32 y, s32 radius, u32 color);
void svga_copy_line_to_fb(VMW_SVGA_Driver *svga, u8 *buffer, s32 width_in_pixels, s32 x, s32 y, u32 filter_color);
void svga_cmd_update_rect(VMW_SVGA_Driver *svga, u32 x, u32 y, u32 width, u32 height);

struct nk_context ctx;

struct Terminal_Em {
    int current_scroll_offset_lines;
    String_Builder text_buffer; // this is a backlog
    String_Builder user_input; // this is the string the user is currently building
    String user_name;
    String machine_name;
};

struct nk_rect consume_from_top(struct nk_rect space, float amount) {
    space.y += amount;
    space.h -= amount;
    return space;
}

struct nk_rect consume_from_left(struct nk_rect space, float amount) {
    space.x += amount;
    space.w -= amount;
    return space;
}

void command_pci_info() {
    For (pci_devices) {
        print_pci_header(&it);
    }
}

#define COMMAND(cmd_str, name) do { if(strings_match(cmd_str, #name)) command_ ## name(); } while(0)

void draw_terminal(struct nk_context *ctx, Terminal_Em *term) {
    struct nk_input *input = &ctx->input;
    struct nk_command_buffer *canvas = nk_window_get_canvas(ctx);
    
    struct nk_rect space;
    enum nk_widget_layout_states state = nk_widget(&space, ctx);
    if (!state) return;
    
    if (true) {
        for (s64 i = 0; i < keyboard_event_queue.count; i++) {
            Input in = keyboard_event_queue[i];
            if (in.action != KEY_PRESS) continue;
            
            if (in.keycode >= KEYCODE_SPACE && in.keycode < KEYCODE_BACKSPACE) {
                string_builder_putchar(&term->user_input, in.utf8_code[0]);
            } else if (in.keycode == KEYCODE_BACKSPACE) {
                if (term->user_input.data.length > 0) term->user_input.data.length--;
            } else if (in.keycode == KEYCODE_ENTER) {
                kprint(term->user_input.data);
                kprint("\n");
                
                // if (strings_match(term->user_input.data, "pci_info")) {}
                COMMAND(term->user_input.data, pci_info);
                
                term->user_input.data.length = 0;
                
                kprint(term->user_name);
                kprint("@");
                kprint(term->machine_name);
                kprint("> ");
            }
        }
    }
    
    // actually render the terminal contents
    
    struct nk_color bg = nk_rgb(0, 0, 0);
    struct nk_color fg = nk_rgb(255, 255, 255);
    nk_fill_rect(canvas, space, 0, nk_rgb(0, 0, 0));
    
    String text_buffer = term->text_buffer.data;
    const nk_user_font *font = ctx->style.font;
    
    auto current_offset_lines = term->current_scroll_offset_lines;
    
    // @TODO we need to make sure that we reserve at least 1 line for the current input
    while (text_buffer.length) {
        s64 offset = find_char(&text_buffer, '\n');
        if (offset == -1) {
            nk_draw_text(canvas, space, (char *)text_buffer.data, (int)text_buffer.length, font, bg, fg);
            
            float width = font->width(font->userdata, font->height, (char *)text_buffer.data, (int)text_buffer.length);
            space = consume_from_left(space, width);
            
            advance(&text_buffer, text_buffer.length);
        } else {
            if (current_offset_lines == 0) {
                nk_draw_text(canvas, space, (char *)text_buffer.data, (int)offset, font, bg, fg);
                space = consume_from_top(space, font->height);
            }
            else current_offset_lines++;
            
            advance(&text_buffer, offset);
            
            // consume the newline
            advance(&text_buffer, 1);
        }
    }
    
    String user = term->user_name;
    String machine = term->machine_name;
    String user_input = term->user_input.data;
    nk_draw_text(canvas, space, (char *)user_input.data, (int)user_input.length, font, bg, fg);
    
    /*
    String line = sprint("%S@%S # %S", user, machine, user_input);
    nk_draw_text(canvas, space, (char *)line.data, (int)line.length, font, bg, fg);
    heap_free(line.data);
    */
}

int get_line_count(String s) {
    int lines = 1;
    for (s64 i = 0; i < s.length; ++i) {
        u8 c = s.data[i];
        if (c == '\n') lines++;
    }
    
    return lines;
}

int terminal_putchar(void *payload, u8 c) {
    Terminal_Em *term = reinterpret_cast<Terminal_Em *>(payload);
    
    int visible_lines = 34;
    if (c == '\n') {
        int num = get_line_count(term->text_buffer.data);
        term->current_scroll_offset_lines = visible_lines - num;
        if (term->current_scroll_offset_lines > 0) term->current_scroll_offset_lines = 0;
    }
    string_builder_putchar(&term->text_buffer, c);
    return 0; // @TODO error codes
}

Terminal_Em term;

void kernel_shell() {
    enum {EASY, HARD};
    static int op = EASY;
    static float value = 0.6f;
    static int i =  20;
    
    struct nk_user_font font;
    
    font.userdata.ptr = nullptr;
    font.height = 16;
    font.width = nuklear_font_width;
    
#define NK_MEM (PAGE_SIZE * 64)
    // // 0x1000000
    
    nk_init_fixed(&ctx, heap_alloc(NK_MEM), NK_MEM, &font);
    
    zero_memory(&term, sizeof(Terminal_Em));
    append(&term.text_buffer, "Hello, Sailor!\n");
    //append(&term.text_buffer, "Testtesttest\n");
    term.current_scroll_offset_lines = 0;
    term.user_name = "josh";
    term.machine_name = "mach-qemu";
    
    // hi-jack the output stream
    {
        Stream *out = &streams[0];
        out->payload = &term;
        out->putc_cb = terminal_putchar;
    }
    
    //kprint("Testing kprint ! %d\n", 123456789);
    
    // for (;;) asm("hlt");
    
    int counter = 20;
    
    while (true) {
        nk_input_begin(&ctx);
        asm("cli"); // @TODO IRQ synchronization primitive
        for (s64 i = 0; i < keyboard_event_queue.count; i++) {
            Input in = keyboard_event_queue[i];
            
            int state = (in.action == KEY_RELEASE) ? 0 : 1;
            switch (in.keycode) {
                case KEYCODE_LEFT_SHIFT:
                case KEYCODE_RIGHT_SHIFT:
                nk_input_key(&ctx, NK_KEY_SHIFT, state);
                break;
                
                case KEYCODE_LEFT_CONTROL:
                case KEYCODE_RIGHT_CONTROL:
                nk_input_key(&ctx, NK_KEY_CTRL,  state);
                break;
                
                // case nk_input_key(&ctx, NK_KEY_DEL,   state); break;
                
                case KEYCODE_ENTER:
                nk_input_key(&ctx, NK_KEY_ENTER, state);
                break;
                
                case KEYCODE_TAB:
                nk_input_key(&ctx, NK_KEY_TAB,   state);
                break;
                
                case KEYCODE_BACKSPACE:
                nk_input_key(&ctx, NK_KEY_BACKSPACE, state);
                break;
                
                default:
                if (in.action == KEY_PRESS) nk_input_char(&ctx, in.utf8_code[0]/*hack*/);
                break;
                
                /*
                case nk_input_key(&ctx, NK_KEY_UP,    state); break;
                case nk_input_key(&ctx, NK_KEY_DOWN,  state); break;
                case nk_input_key(&ctx, NK_KEY_LEFT,  state); break;
                case nk_input_key(&ctx, NK_KEY_RIGHT, state); break;
                */
            }
            // @TODO send KB to WM 
            // kprint("%c", in.utf8_code[0]);
        }
        nk_input_end(&ctx);
        
        if (nk_begin_titled(&ctx, "_Terminal", "Terminal", nk_rect(50, 50, 500, 600),
                            NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_CLOSABLE|NK_WINDOW_SCALABLE|NK_WINDOW_MINIMIZABLE|NK_WINDOW_SCROLL_AUTO_HIDE|NK_WINDOW_NO_SCROLLBAR)) {
            
            struct nk_rect reg = nk_window_get_content_region(&ctx);
            
            nk_layout_space_begin(&ctx, NK_STATIC, reg.h, 1);
            reg = nk_layout_space_rect_to_local(&ctx, reg);
            nk_layout_space_push(&ctx, reg);
            draw_terminal(&ctx, &term);
            nk_layout_space_end(&ctx);
        }
        nk_end(&ctx);
        
        if (nk_begin(&ctx, "Show", nk_rect(700, 50, 220, 220),
                     NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_CLOSABLE)) {
            // fixed widget pixel width
            nk_layout_row_static(&ctx, 30, 80, 1);
            if (nk_button_label(&ctx, "button")) {
                // event handling
            }
            // fixed widget window ratio width
            nk_layout_row_dynamic(&ctx, 30, 2);
            if (nk_option_label(&ctx, "easy", op == EASY)) op = EASY;
            if (nk_option_label(&ctx, "hard", op == HARD)) op = HARD;
            // custom widget pixel width
            nk_layout_row_begin(&ctx, NK_STATIC, 30, 2);
            {
                nk_layout_row_push(&ctx, 50);
                nk_label(&ctx, "Volume:", NK_TEXT_LEFT);
                nk_layout_row_push(&ctx, 110);
                nk_slider_float(&ctx, 0, &value, 1.0f, 0.1f);
            }
            nk_layout_row_end(&ctx);
        }
        nk_end(&ctx);
        
        // enable interrupts and clear the keyboard state at the end of the GUI update so we can
        // read from the input queue during GUI updates
        // @FixMe this needs a lock around it, or we need to disable keyboard IRQs
        asm("sti");
        keyboard_event_queue.clear();
        
        svga_clear_screen(&svga_driver, 0xFF272822);
        const struct nk_command *it = 0;
        nk_foreach(it, &ctx) {
            switch (it->type) {
                case NK_COMMAND_RECT_FILLED: {
                    nk_command_rect_filled *rect = (nk_command_rect_filled *) it;
                    u32 c = nk_color_u32(rect->color);
                    svga_draw_rect(&svga_driver, rect->x, rect->y, rect->w, rect->h, c);
                } break;
                case NK_COMMAND_RECT_MULTI_COLOR:
                
                break;
                case NK_COMMAND_RECT: {
                    nk_command_rect *rect = (nk_command_rect *) it;
                    u32 c = nk_color_u32(rect->color);
                    svga_draw_rect_outline(&svga_driver, rect->x, rect->y, rect->w, rect->h, c);
                } break;
                case NK_COMMAND_TEXT: {
                    nk_command_text *text = (nk_command_text *) it;
                    u32 color = nk_color_u32(text->foreground);
                    // font := cast(*Font) tex.font.userdata.ptr;
                    // text: string;
                    // text.count = tex.length;
                    // text.data = tex.string.data;
                    // draw_text(<<renderer, <<font, cast(float) tex.x, cast(float) tex.y + (font.char_height * 3) / 4, text, color=color);
                    
                    u32 *data = reinterpret_cast<u32 *>(&image_data[0]);
                    s32 offset = 0;
                    for (s32 i = 0; i < text->length; ++i) {
                        if (text->string[i] == ' ') {
                            offset += 6;
                            continue;
                        }
                        s32 start = 0, end = 0;
                        s32 width = get_glyph_start_end(text->string[i], &start, &end);
                        
                        
                        for (int line = 1; line < 16; ++line) {
                            svga_copy_line_to_fb(&svga_driver, (u8 *)&data[start + image_width * line], width, text->x + offset, text->y + line, 0xFF000000);
                        }
                        offset += width;
                    }
                    
                    svga_cmd_update_rect(&svga_driver, text->x, text->y, offset, 8);
                } break;
                case NK_COMMAND_CIRCLE_FILLED: {
                    nk_command_circle_filled *circ = (nk_command_circle_filled *) it;
                    u32 c = nk_color_u32(circ->color);
                    // assert(circ.w == circ.h);
                    // draw_circle(<<renderer, cast(float) circ.x, cast(float) circ.y, cast(float) circ.w / 2, make_Color(c));
                    svga_draw_circle(&svga_driver, circ->x, circ->y, circ->w / 2, c);
                    
                } break;
                case NK_COMMAND_LINE:
                break;
                
                case NK_COMMAND_SCISSOR:
                break;
                
                default:
                // kprint("cmd: %\n", it->type);
                {}
            }
        }
        nk_clear(&ctx);
        
        //svga_draw_circle(&svga_driver, 200, 100, 100, 0xFFFFFFFF);
        
        svga_update_screen(&svga_driver);
        
        asm("hlt");
    }
}



#define STRING_IMPLEMENTATION
#include "string.h"

#define PRINT_IMPLEMENTATION
#include "print.h"

#define NK_IMPLEMENTATION
#define NK_ASSERT kassert
#include "nuklear.h"
